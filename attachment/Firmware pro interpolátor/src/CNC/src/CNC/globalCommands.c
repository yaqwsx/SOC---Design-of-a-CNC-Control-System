#include "globalCommands.h"
#include "DelayComponent.h"
#include "global.h"
#include "commandStack.h"

//Function definition
//Definition of ResetGlobalCommand
//actually does nothing, reserved for future
void ResetGlobalCommand(void)
{

}

//Definition of ProcessGlobalCommand
//Processes given command addressed to global component
//Returns state of function - if it was interruped or not
CommandRet ProcessGlobalCommand(__IO CommandStruct* command, bool fromStack)
{
	switch(command->type)
	{
		case COM_GLOBAL_WAIT://Wait command
			/*Attribute is delay in command structure*/
		{
			uint8_t ID = AddDelay(command->delay, 0);
			while(GetDelay(ID) != 0)
			{
				if(fromStack && !ContinuePerforming())
					return COMMAND_PROC_INTERRUPT;
			}
			RemoveDelay(ID);
			return COMMAND_PROC_OK;
		}
		break;
	}
	return COMMAND_PROC_OK;
}
