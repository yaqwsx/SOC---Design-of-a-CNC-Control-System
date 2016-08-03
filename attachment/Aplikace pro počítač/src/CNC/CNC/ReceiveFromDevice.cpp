#include "ReceiveFromDevice.h"
#include <wx/wx.h>
#include <Windows.h>
#include "CommunicationEnumerations.h"
#include "MainWindow.h"
#include <fstream>
#include "global.h"
#include <stdint.h>

ofstream position("vystup.txt");
ofstream debugPosition("_debugPositionLog.txt");


ReceiveFromDevice::ReceiveFromDevice(void): received(0), receivedPackets(0)
{
}


ReceiveFromDevice::~ReceiveFromDevice(void)
{
}

wxThread::ExitCode ReceiveFromDevice::Entry()
{
	bool error = false;//Indicates last state
	while(!TestDestroy())
	{
		DWORD read = 0;
		UCHAR buff[256]={0};
		if(device->WinUSBHandle == INVALID_HANDLE_VALUE || !device->GetState())
		{
			Reset();
			if(error)
				Sleep(errSleep);
			error = true;
			continue;
		}
		OVERLAPPED overlapped;
		ZeroMemory(&overlapped, sizeof(overlapped));
		overlapped.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);//(NULL, TRUE, FALSE, NULL);
		if(!WinUsb_ReadPipe(device->WinUSBHandle, IN_EP, data + received, IN_EP_PACKET, &read, &overlapped))
		{
			if(GetLastError() != ERROR_IO_PENDING)
			{
				Reset();
				if(error)
					Sleep(errSleep);
				error = true;
				continue;
			}
		}
		while(WaitForSingleObject(overlapped.hEvent, 100) == WAIT_TIMEOUT)
		{
			if(TestDestroy())//Check whether the thread should be destroyed
			{	
				CloseHandle(overlapped.hEvent);
				return 0;
			}
		}
		//Complete transfer - get received bytes
		WinUsb_GetOverlappedResult(device->WinUSBHandle, &overlapped, &read, FALSE);
		CloseHandle(overlapped.hEvent);//Clear event
		//Process data
		if(read == 0)
		{
			//Error occur, reset state
			Reset();
			if(error)
				Sleep(errSleep);
			error = true;
			continue;
		}
		error = false;
		//Increase counters
		received += read;
		receivedPackets++;
		//Check if we read entire message
		if(data[0] == receivedPackets)
		{
			ProcessMessage();
			Reset();
		}
	}
	return 0;
}

void ReceiveFromDevice::Reset()
{
	received = receivedPackets = 0;
}

void ReceiveFromDevice::ProcessMessage()
{
	switch(data[1])
	{
		case COMSTACK_FREE_SPACE_MESS:
		{
			device->freeSpace = *((uint16_t*)(data+2));
			//Thread synchronisation - notify
			wxMutexLocker mL(device->freeSpaceM);
			if(device->freeSpace > 128)
			{
				int i = 2;
				i = 6;
			}
			device->freeSpaceC.Signal();
		}
		break;

		case LAST_PROC_ITEM_STACK:
		{
			device->lastItemStack = *((uint32_t*)(data+2));
			//Thread synchronisation - notify
			wxMutexLocker mL(device->lastItemStackM);
			device->lastItemStackC.Signal();
		}
		break;

		case CUR_PROC_ITEM_STACK:
		{
			device->curItemStack = *((uint32_t*)(data+2));
			//Thread synchronisation - notify
			wxMutexLocker mL(device->curItemStackM);
			device->curItemStackC.Signal();
		}
		break;
		
		case AXES_POSITION:
		{
			float p[7];
			for(int i = 0; i != 7;i++)
			{
				p[i] = *((float*)(data+i*sizeof(float)+2));
			}
			wxString t;
			t << wxT("X: ") << p[0] << wxT("\tY: ") << p[1] << wxT("\tZ: ") << p[2];
			position << p[0] << "\t" << p[1] << "\t" <<  p[5] << "\t" <<p[6]<<endl;
			MainWindow* w = (MainWindow*)(wnd);
			w->statusBar->SetStatusText(t, 2);
			Point add;
			add.x = p[0];
			add.y = p[1];
			w->middlePanel->AddToTrajectory(add);
		}
		break;

		case DEBUG_POSITION_INFO:
		{
			//Incomming message is position info for debugging
			float p[3];
			for(int u = 0; u != 21; u++)
			{
				uint32_t time = *((uint32_t*)(data+2+u*12));
				for(int i = 0; i != 3;i++)
				{
					p[i] = *((float*)(data+i*sizeof(float)+6+u*12));
				}
				debugPosition << time << "\t" << p[0] << "\t" << p[1] << "\t" << p[0] << "\n";
			}
		}
		break;

		case ERROR_MESS:
		{
			wxString text = wxT("Zaøízení odeslalo chybu: ");
			switch(data[2])
			{
				case ERR_COMSTACKOWERFLOW:
					text << wxT("Pøeteèení vstupního bufferu");
					break;
				
				default:
					text << wxT("Neznámá chyba");
					break;
				
			}
			wxMessageBox(text, wxT("Chyba!"), wxOK | wxICON_EXCLAMATION);
		}
		break;
		default:
			{
			}
	}
}