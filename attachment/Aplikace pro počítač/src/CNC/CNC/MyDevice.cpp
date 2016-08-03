#include "MyDevice.h"
#include <strsafe.h>
#include "ReceiveFromDevice.h"
#include "CommunicationEnumerations.h"
#include "global.h"


MyDevice::MyDevice(void): WinUSBHandle (INVALID_HANDLE_VALUE), attached (false), freeSpaceC(freeSpaceM),
	lastItemStackC(lastItemStackM), curItemStackC(curItemStackM)
{
}

void MyDevice::Init()
{
	//Intializes device
	InitDevice();
	//Init thread
	receive = new ReceiveFromDevice;
	receive->Create();
	receive->SetDependences(this);
	receive->Run();
}

MyDevice::~MyDevice(void)
{
	DeInitDevice();
	receive->Delete();
}

bool MyDevice::TryToConnect()
{
	if(TestIfConnected())
		return InitDevice();
	return false;
}

void MyDevice::OnConnect()
{
	attached = InitDevice();
}

void MyDevice::OnDisconnect()
{
	StateOfProgram = ProgramControl::STOP;
	DeInitDevice();
	attached = false;
}

bool MyDevice::InitDevice()
{
	if(GetState())//Check if device is connected
		return true;
	//Connect device
	HANDLE hDeviceHandle = INVALID_HANDLE_VALUE;
	//Copied from Microdoft example - modified
	HDEVINFO hDeviceInfo;
	SP_DEVINFO_DATA DeviceInfoData;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = NULL;
    ULONG requiredLength=0;
    LPTSTR lpDevicePath = NULL;
    DWORD index = 0;
	BOOL bResult = false;
    //Get information about all devices with given interface
	hDeviceInfo = SetupDiGetClassDevs( &DEVICE_INTERFACE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if(hDeviceInfo == INVALID_HANDLE_VALUE) 
	{ 
        //Error
		wxString err;
        err << wxT("Chyba SetupDiGetClassDevs: ") << GetLastError();
		PostErrorMessage(err);
        return false;
    }

    //Enumeration off all devices
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for(index = 0; SetupDiEnumDeviceInfo(hDeviceInfo, index, &DeviceInfoData); index++)
    {
		if(lpDevicePath)
			LocalFree(lpDevicePath);
        if(pInterfaceDetailData)
			LocalFree(pInterfaceDetailData);
        deviceInterfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
        //Get information about interfaces
        bResult = SetupDiEnumDeviceInterfaces(hDeviceInfo, &DeviceInfoData,
			&DEVICE_INTERFACE, index, &deviceInterfaceData);
        //Check last error
        if (GetLastError() == ERROR_NO_MORE_ITEMS)
            break;//No more items

        //If there are another errors:
        if (!bResult) 
        {
			wxString err;
            err << wxT("Chyba SetupDiEnumDeviceInterfaces: ") << GetLastError();
			PostErrorMessage(err);
            LocalFree(lpDevicePath);
			LocalFree(pInterfaceDetailData);
			return false;
        }

        //Interface data is returned in SP_DEVICE_INTERFACE_DETAIL_DATA
        //which we need to allocate, so we have to call this function twice.
        //First to get the size so that we know how much to allocate
        //Second, the actual call with the allocated buffer  
        bResult = SetupDiGetDeviceInterfaceDetail(hDeviceInfo, &deviceInterfaceData,
			NULL, 0, &requiredLength, NULL);

        //Check for some other error
        if (!bResult) 
        {
            if ((ERROR_INSUFFICIENT_BUFFER==GetLastError()) && (requiredLength>0))
            {
                //we got the size, allocate buffer
                pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, requiredLength);
                
                if (!pInterfaceDetailData) 
                { 
                    //Error
					wxString err;
                    err << wxT("Error allocating memory for the device detail buffer") << GetLastError();
					PostErrorMessage(err);
					LocalFree(lpDevicePath);
					LocalFree(pInterfaceDetailData);
					return false;
                }
            }
            else
            {
				wxString err;
				err << wxT("Error SetupDiEnumDeviceInterfaces: ") << GetLastError();
                PostErrorMessage(err);
				LocalFree(lpDevicePath);
				LocalFree(pInterfaceDetailData);
				return false;
            }
        }

        //get the interface detailed data
        pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        //Now call it with the correct size and allocated buffer
        bResult = SetupDiGetDeviceInterfaceDetail(hDeviceInfo, &deviceInterfaceData,
			pInterfaceDetailData, requiredLength, NULL, &DeviceInfoData);
        
        //Check for some other error
        if (!bResult) 
        {
			wxString err;
            err << wxT("Error SetupDiGetDeviceInterfaceDetail: ") << GetLastError();
            PostErrorMessage(err);
			LocalFree(lpDevicePath);
			LocalFree(pInterfaceDetailData);
			return false;
        }

        //copy device path        
        size_t nLength = wcslen (pInterfaceDetailData->DevicePath) + 1;  
        lpDevicePath = (TCHAR *) LocalAlloc (LPTR, nLength * sizeof(TCHAR));
        StringCchCopy(lpDevicePath, nLength, pInterfaceDetailData->DevicePath);
        lpDevicePath[nLength-1] = 0;
    }
	if(index > 1)
	{
		PostErrorMessage(wxT("Chyba, pøipojeno více zaøízení!"));
		return false;
	}

    if (!lpDevicePath)
    {
        //Error
		wxString err;
        err << wxT("Chyba ") << GetLastError();
        //PostErrorMessage(err);
		LocalFree(lpDevicePath);
		LocalFree(pInterfaceDetailData);
		return false;
    }

    //Open the device
    hDeviceHandle = CreateFile(lpDevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        //Error
		wxString err;
        err << wxT("Chyba pøi získávání handle k souboru zaøízení: ") << GetLastError();
        PostErrorMessage(err);
		LocalFree(lpDevicePath);
		LocalFree(pInterfaceDetailData);
		return false;
    }
    LocalFree(lpDevicePath);
    LocalFree(pInterfaceDetailData);    
    bResult = SetupDiDestroyDeviceInfoList(hDeviceInfo);
	//Init winUSB handle
	if(!WinUsb_Initialize(hDeviceHandle, &WinUSBHandle))
	{
		 //Error
		wxString err;
        err << wxT("Chyba pøi získávání handle k WinUSB: ") << GetLastError();
        PostErrorMessage(err);
		LocalFree(lpDevicePath);
		LocalFree(pInterfaceDetailData);
		return false;
	}
	attached = true;
	return true;
}

bool MyDevice::TestIfConnected()
{
	HDEVINFO hDeviceInfo;
    SP_DEVINFO_DATA DeviceInfoData;
	//Get information about devices with given interface
    hDeviceInfo = SetupDiGetClassDevs( &DEVICE_INTERFACE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDeviceInfo == INVALID_HANDLE_VALUE) 
    { 
        //Error
		wxString err;
        err << wxT("Error SetupDiGetClassDevs: ")<< GetLastError();
		PostErrorMessage(err);
		SetupDiDestroyDeviceInfoList(hDeviceInfo);
        return false;
    }

    //Enumerate all devices
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    if(!SetupDiEnumDeviceInfo(hDeviceInfo, 0, &DeviceInfoData))
	{
		if(GetLastError() == ERROR_NO_MORE_ITEMS)
		{
			SetupDiDestroyDeviceInfoList(hDeviceInfo);
			return false;
		}
		wxString err;
		err << wxT("Error SetupDiEnumDeviceInfo: ") << GetLastError();
		PostErrorMessage(err);
		SetupDiDestroyDeviceInfoList(hDeviceInfo);
		return false;
	}
	SetupDiDestroyDeviceInfoList(hDeviceInfo);
	return true;
}

bool MyDevice::DeInitDevice()
{
	if(WinUSBHandle != INVALID_HANDLE_VALUE)
		WinUsb_Free(WinUSBHandle);
	attached = false;
	return true;
}

bool MyDevice::SendRawData(std::vector<unsigned char>& data)
{
	if(!GetState())
		return false;
	ULONG sent = 0;
	if(!WinUsb_WritePipe(WinUSBHandle, OUT_EP, &data[0], data.size(), &sent, 0))
	{
		return false;
	}
	return true;
}

bool MyDevice::SendRawData(unsigned char* data, size_t length)
{
	if(!GetState())
		return false;
	if(data[0] > 3)
	{
		int i = 0;
		i = 2;
	}
	ULONG sent = 0;
	if(!WinUsb_WritePipe(WinUSBHandle, OUT_EP, data, length, &sent, 0))
	{
		wxString err(wxT("Chyba pøi odesílání dat: "));
		err << GetLastError();
		PostErrorMessage(err);
		return false;
	}
	return true;
}

size_t MyDevice::GetFreeSpaceInQueue()
{
	if(!TestIfConnected())
		return 0;
	//Send requirement
	unsigned char buffer[64];
	buffer[0] = 1;
	buffer[1] = RECEIVER_COMMANDSTACK;
	buffer[2] = COMSTACK_RET_SPACE;
	//Wait for answer
	freeSpaceM.Lock();//lock mutex
	SendRawData(buffer, sizeof(buffer));//Send request
	//Wait for answer
	if(freeSpaceC.WaitTimeout(4000) != wxCOND_NO_ERROR)
	{
		if(StateOfProgram != ProgramControl::STOP)
		{
			PostErrorMessage(wxString(_T("Chyba pøi èekání na odpovìï zaøízení na požadavek COMSTACK_RET_SPACE")));
			wxMessageBox(wxT("Chyba pøi èekání na odpovìï zaøízení na požadavek COMSTACK_RET_SPACE"), wxT("Chyba!"), wxOK | wxICON_EXCLAMATION);
		}
		freeSpaceM.Unlock();
		return 1;//Error occured
	}
	freeSpaceM.Unlock();
	return freeSpace;
}

uint32_t MyDevice::GetLastProcessedItemStack()
{
	if(!TestIfConnected())
		return 0;
	//Send requirement
	unsigned char buffer[64];
	buffer[0] = 1;
	buffer[1] = RECEIVER_COMMANDSTACK;
	buffer[2] = COMSTACK_LAST_ITEM_PROC;
	//Wait for answer
	lastItemStackM.Lock();//lock mutex
	SendRawData(buffer, sizeof(buffer));//Send request
	//Wait for answer
	if(lastItemStackC.WaitTimeout(4000) != wxCOND_NO_ERROR)
	{
		if(StateOfProgram != ProgramControl::STOP)
			PostErrorMessage(wxString(_T("Chyba pøi èekání na odpovìï zaøízení na požadavek STATEMESSAGE_LAST_ITEM")));
		lastItemStackM.Unlock();
		return 0;//Error occured
	}
	lastItemStackM.Unlock();
	return lastItemStack;
}

uint32_t MyDevice::GetCurrentlyProcessedItemStack()
{
	if(!TestIfConnected())
		return 0;
	//Send requirement
	unsigned char buffer[64];
	buffer[0] = 1;
	buffer[1] = RECEIVER_COMMANDSTACK;
	buffer[2] = COMSTACK_CURRENT_ITEM_PROC;
	//Wait for answer
	curItemStackM.Lock();//lock mutex
	SendRawData(buffer, sizeof(buffer));//Send request
	//Wait for answer
	if(curItemStackC.WaitTimeout(4000) != wxCOND_NO_ERROR)
	{
		if(StateOfProgram != ProgramControl::STOP)
			PostErrorMessage(wxString(_T("Chyba pøi èekání na odpovìï COMSTACK_CURRENT_ITEM_PROC")));
		curItemStackM.Unlock();
		return 0;//Error occured
	}
	curItemStackM.Unlock();
	return curItemStack;
}