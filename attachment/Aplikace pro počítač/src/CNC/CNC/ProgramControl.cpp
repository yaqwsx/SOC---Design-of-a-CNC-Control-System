#include "ProgramControl.h"
#include "CommunicationEnumerations.h"
#include "global.h"
#include "D:\Users\Honza\Atollic\TrueSTUDIO\STM32_workspace\CNC\src\CNC\CommandStruct\commandStruct.h"

ProgramControl::ProgramControl(void)
{
}


ProgramControl::~ProgramControl(void)
{
}


bool ProgramControl::Start()
{
	if(StateOfProgram == PAUSE)
		return Continue();
	if(StateOfProgram == RUN)
		return false;
	ProgramRun* program = new ProgramRun();
	if(program->Create() != wxTHREAD_NO_ERROR)
	{
		PostErrorMessageC(wxT("Nepodaøilo se vytvoøit vlákno programu. Program nelze spustit."));
		return true;
	}
	program->SetDependencies(device, this, data);
	StateOfProgram = RUN;
	if(program->Run() != wxTHREAD_NO_ERROR)
	{
		PostErrorMessageC(wxT("Nepodaøilo se sputit vlákno programu. Program nebude spuštìn"));
		return true;
	}
	data->window->toolbar->EnableTool(2, false);
	data->window->toolbar->EnableTool(3, true);
	data->window->toolbar->EnableTool(4, true);
	return false;
}

bool ProgramControl::Pause()
{
	if(StateOfProgram == STOP || StateOfProgram == PAUSE)
		return false;
	StateOfProgram = PAUSE;
	data->window->toolbar->EnableTool(2, true);
	data->window->toolbar->EnableTool(3, false);
	data->window->toolbar->EnableTool(4, true);
	return false;
}

bool ProgramControl::Continue()
{
	StateOfProgram = ProgramControl::RUN;
	data->window->toolbar->EnableTool(2, false);
	data->window->toolbar->EnableTool(3, true);
	data->window->toolbar->EnableTool(4, true);
	return false;
}

bool ProgramControl::Stop()
{
	StateOfProgram = ProgramControl::STOP;
	if(!init)
		return true;
	data->window->toolbar->EnableTool(2, true);
	data->window->toolbar->EnableTool(3, false);
	data->window->toolbar->EnableTool(4, false);
	return false;
}