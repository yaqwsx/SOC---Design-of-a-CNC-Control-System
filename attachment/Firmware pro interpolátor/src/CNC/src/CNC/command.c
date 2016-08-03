#include "command.h"
#include "commandStack.h"
#include "state.h"
#include "GPIOControl/GPIO.h"
#include "globalCommands.h"
#include "global.h"
#include "axis.h"

//Function definition
//Definition of RedistributeCommandForPerf
//Redistributes command to the right component and perfoms it.
//stopOther defines, if the commands from stack should be stopped.
CommandRet RedistributeCommandForPerf(CommandStruct* command, bool fromStack, bool stopOther)
{
	if(stopOther)//Should be other commands stopped?
		StopPerforming();
	SetActiveCommand(command->ID);//performing command
	switch(command->receiver)
	{
		case COM_RECEIVER_GLOBAL:
			ProcessGlobalCommand(command, fromStack);
			break;
		case COM_RECEIVER_GPIO:
			ProcessGPIOCommand(command, fromStack);
			break;
		case COM_RECEIVER_AXIS:
			ProcessAxisCommand(command, fromStack);
			break;
		//Todo: Add more receivers
	}
	CommandDone(command->ID);//command done
	return COMMAND_PROC_OK;//Command processed normally
};
