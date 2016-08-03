#include "ProgramRun.h"
#include "ProgramControl.h"
#include "D:\Users\Honza\Atollic\TrueSTUDIO\STM32_workspace\CNC\src\CNC\CommandStruct\commandStruct.h"
#include "CommunicationEnumerations.h"
#include <algorithm>
#include "global.h"


ProgramRun::ProgramRun(void)
{
}


ProgramRun::~ProgramRun(void)
{
}


ProgramRun::ExitCode ProgramRun::Entry()
{
	if(data->processedData.size() == 0)
	{
		wxMessageBox(wxT("Není otevøen žádný program"), wxT("Chyba!"), wxOK | wxICON_EXCLAMATION);
		program->Stop();
		return 0;
	}

	if(device->GetState() == false)
	{
		wxMessageBox(wxT("Není pøipojen interpolátor"), wxT("Chyba!"), wxOK | wxICON_EXCLAMATION);
		program->Stop();
		return 0;
	}
	bool done = false;
	uint32_t lastItem = 0;
	lastState = ProgramControl::STOP;
	while(!done)
	{
		if(StateOfProgram == ProgramControl::STOP)
		{
			//Terminate
			if(lastState != ProgramControl::STOP)
			{
				unsigned char buff[64] = {0};
				buff[0] = 1;
				buff[1] = RECEIVER_COMMANDSTACK;
				buff[2] = COMSTACK_PAUSE;
				device->SendRawData(buff, sizeof(buff));
				buff[2] = COMSTACK_CLEAR;
				device->SendRawData(buff, sizeof(buff));
			}
			lastState = ProgramControl::STOP;
			return 0;
		}
		if(StateOfProgram == ProgramControl::PAUSE)
		{
			//Pause
			if(lastState != ProgramControl::PAUSE)
			{
				unsigned char buff[64] = {0};
				buff[0] = 1;
				buff[1] = RECEIVER_COMMANDSTACK;
				buff[2] = COMSTACK_PAUSE;
				device->SendRawData(buff, sizeof(buff));
			}
			lastState = ProgramControl::PAUSE;
			Sleep(100);
			continue;
		}
		if(StateOfProgram == ProgramControl::RUN)
		{
			//Processing the program
			//Ask for position
			unsigned char buff[64] = {0};
			buff[0] = 1;
			buff[1] = RECEIVER_STATE;
			buff[2] = STATEMESSAGE_POSITION;
			device->SendRawData(buff, sizeof(buff));
			//Send start command
			if(lastState != ProgramControl::RUN)
			{
				buff[1] = RECEIVER_COMMANDSTACK;
				buff[2] = COMSTACK_START;
				device->SendRawData(buff, sizeof(buff));
			}
			lastState = ProgramControl::RUN;
			//Check if we are done
			uint32_t ID = device->GetLastProcessedItemStack();
			HighlightActiveLine(device->GetCurrentlyProcessedItemStack());
			if(ID == data->processedData.back().data.ID)
			{
				program->Stop();
				buff[0] = 1;
				buff[1] = RECEIVER_COMMANDSTACK;
				buff[2] = COMSTACK_RESET_LAST_ITEM_PROC;
				device->SendRawData(buff, sizeof(buff));
			}
			//Get free space
			uint32_t freeSpace = device->GetFreeSpaceInQueue() - 1;
			uint8_t itemsToSend = min(min(freeSpace, (uint32_t)4), data->processedData.size() - lastItem);
			//static ofstream log("misto.txt");
			//log << freeSpace << "\t" << int(itemsToSend) << "\t" << data->processedData.size() - lastItem << endl;
			if(itemsToSend != 0)
			{
				unsigned char* buffer = new unsigned char[((4+itemsToSend*sizeof(CommandStruct))/64 + 1)*64];
				ZeroMemory(buffer, ((4+itemsToSend*sizeof(CommandStruct))/64 + 1)*64);
				buffer[0] = ((4+itemsToSend*sizeof(CommandStruct))/64 + 1);
				buffer[1] = RECEIVER_COMMANDSTACK;
				buffer[2] = COMSTACK_ADD_COM;
				buffer[3] = itemsToSend;
				for(uint32_t i = 0; i != itemsToSend; i++)
				{
					CommandStruct* com = (CommandStruct*)(buffer + 4 +i*sizeof(CommandStruct));
					if(data->processedData[lastItem+i].data.line.v != 0)
						*com = data->processedData[lastItem+i].data;
					else
					{
						int u = 2;
						u = 65;
					}
				}
				device->SendRawData(buffer, buffer[0]*64);
				delete[] buffer;
				lastItem += itemsToSend;
			}
			Sleep(10);
		}
	}
	return 0;
}

ProgramRun::ExitCode ProgramRun::Clean()
{
	return 0;
}

void ProgramRun::HighlightActiveLine(uint32_t ID)
{
	auto i = find_if(data->processedData.begin(), data->processedData.end(), [&](ProcessedData& d)->bool
	{
		return d.data.ID == ID;
	});
	if(i != data->processedData.end())
	{
		data->window->codeView->SetSelection(data->window->codeView->XYToPosition(0, i->line-1), data->window->codeView->XYToPosition(0, i->line));
	}
}
