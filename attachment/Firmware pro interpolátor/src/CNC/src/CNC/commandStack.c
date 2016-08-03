#include "commandStack.h"
#include "global.h"
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "config.h"
#include "errState.h"

//Data
__IO CommandStackStruct commandStack;
__IO bool continuePerf;//Bool value, indicates whether stop performing or stop

//Private function declarations
uint16_t CommandStackGetFreeSpace(void);//Returns free space in CommandStack
uint16_t CommandStackIncreaseValue(uint16_t);//Increases value in given range
uint16_t CommandStackIncreaseValueBy(uint16_t value, uint16_t i);
bool CommandStackAdd(CommandStruct *com);//Adds command to stack. Returns true if buff overflow
void CommandStackClear(void);//Clears the commandStack

//Function definitions
//Definition of ResetCommandStack
//Resets the stack to initial value
void ResetCommandStack(void)
{
	commandStack.first = commandStack.last = 0;
	continuePerf = false;
}

//ContinuePerforming
//Return value, if the function should continue perform command.
//Every command needs to call this function
bool ContinuePerforming(void)
{
	return continuePerf;
}

//Stop performing
//Stops performing of command in order to f.e: perform a direct command
void StopPerforming(void)
{
	continuePerf = false;
}

//Definition of StartPerforming
//This function starts processing commands in stack
//This function contains an almost never ending loop.
//It shouldn't be called in interrupt, etc..
void StartPerforming(void)
{
	bool configured = false;
	while(continuePerf && (configured = IsConfigured()))//until it is allowed and machine is configured
	{
		if(commandStack.first != commandStack.last && IsGood())//There are unprocessed items and no errors
		{
			if(RedistributeCommandForPerf(&(commandStack.buffer[commandStack.first]), true, false) == COMMAND_PROC_OK)//Distribute item
			{
				commandStack.lastItem = commandStack.buffer[commandStack.first].ID;
				//Item processed successfully - increase counter
				commandStack.first = CommandStackIncreaseValue(commandStack.first);
			}
		}
	}
}

//Definition of CommandStackProcessMessage
//Processes incoming message from PC
void CommandStackProcessMessage(uint8_t* message)
{
	//Incoming message. First byte is identifier
	switch(*((uint8_t*)(message)))
	{
		case COMSTACK_RET_SPACE://Return free space in queue
			/*This message has no params, just return free space
			 * Returns COMSTACK_FREE_SPACE_MESS + 16 bit number*/
			{
				outBuff[0] = 8/VD_IN_PACKET+1;//length of message
				outBuff[1] = COMSTACK_FREE_SPACE_MESS;
				uint16_t size = CommandStackGetFreeSpace();
				*((uint16_t*)(outBuff+2)) = size;
				SendData(outBuff, 4);
			}
			break;
		case COMSTACK_ADD_COM://Add command(s)
			/*Incoming message in format 8 bit defines number of elements
			 * Then follows n CommandStructures. No return
			 */
			{
				uint8_t debug = sizeof(CommandStruct);
				uint8_t number = *((uint8_t*)(message+1));//Stores the number of commands
				uint8_t i;
				for(i = 0; i != number; i++)
				{
					if(CommandStackAdd((CommandStruct*)((message+2)+i*sizeof(CommandStruct))))
					{
						//Buffer overflow
						SetError(ERR_COMSTACKOWERFLOW);
						break;
					}
				}
				//Increase number of elements
				//commandStack.last = CommandStackIncreaseValueBy(commandStack.last, number);
			}
			break;
		case COMSTACK_LAST_COM_ID://Returns last item's ID in stack
			/*This function can help in case of fixing errors
			 * PC can find out which item is last in the stack
			 * If the ID is 0, there's no command in stack
			 * Answer is COMSTACK_LAST_ID*/
			{
				//static uint8_t buff[2+sizeof(CommandID)];
				outBuff[0] = (2+sizeof(CommandID))/VD_IN_PACKET+1;
				outBuff[1] = COMSTACK_LAST_ID;
				if(commandStack.last == commandStack.first)
					*((CommandID*)(outBuff+2)) = 0;
				else
					*((CommandID*)(outBuff+2)) = commandStack.buffer[CommandStackIncreaseValueBy(commandStack.last, -1)].ID;
				SendData(outBuff, 2+sizeof(CommandID));
			}
			break;
		case COMSTACK_CLEAR://Clears the stack
			/**/
			{
				CommandStackClear();
			}
			break;
		case COMSTACK_START://Starts performing
		{
			continuePerf = true;
		}
		break;

		case COMSTACK_PAUSE://Stops performing, but stack isn't cleared
		{
			StopPerforming();
		}
		break;

		case COMSTACK_LAST_ITEM_PROC:
		{
			//uint8_t* buffer;
			uint8_t length;
			length = 2 + sizeof(CommandID);
			//buffer = malloc(length);
			outBuff[0] = length/VD_IN_PACKET + 1;
			outBuff[1] = LAST_PROC_ITEM_STACK;
			*((uint32_t*)(outBuff + 2)) = commandStack.lastItem;
			//Buffer ready, send
			SendData(outBuff, length);
			//free(buffer);
		}
		break;

		case COMSTACK_CURRENT_ITEM_PROC:
		{
			//uint8_t* buffer;
			uint8_t length;
			length = 2 + sizeof(CommandID);
			//buffer = malloc(length);
			outBuff[0] = length/VD_IN_PACKET + 1;
			outBuff[1] = CUR_PROC_ITEM_STACK;
			*((uint32_t*)(outBuff + 2)) = state.currentItem;
			//Buffer ready, send
			SendData(outBuff, length);
			//free(buffer);
		}
		break;

		case COMSTACK_RESET_LAST_ITEM_PROC:
		{
			commandStack.lastItem = 0;
		}
	}
}

//Definition of CommandStackGetFreeSpace
//Returns free space in stack
uint16_t CommandStackGetFreeSpace(void)
{
	if(commandStack.first == commandStack.last)
		return COMMANDSTACK_BUFF;
	if(commandStack.first < commandStack.last)
		return COMMANDSTACK_BUFF - commandStack.last + commandStack.first;
	else
		return commandStack.first - commandStack.last;
}

//Definition of CommandStackIncreaseValue
//Increases given index in range of stack
uint16_t CommandStackIncreaseValue(uint16_t value)
{
	value++;
	if(value >= COMMANDSTACK_BUFF)
	{
		value = 0;
	}
	return value;
}

//Definition of CommandStackIncreaseValueBy
//Increases with value i given index in range of stack
uint16_t CommandStackIncreaseValueBy(uint16_t value, uint16_t i)
{
	value += i;
	if(value >= COMMANDSTACK_BUFF)
	{
		value -= COMMANDSTACK_BUFF;
	}
	return value;
}

//Definition of CommandStackAdd
//Takes pointer to structure
//return true, if buffer overflow occurs. Otherwise false
//Adds command onto stack
bool CommandStackAdd(CommandStruct *com)
{
	if(CommandStackIncreaseValue(commandStack.last) == commandStack.first)
	{
		//Buffer overflow
		return true;
	}
	//Buffer if free
	memcpy((void*)(&commandStack.buffer[commandStack.last]), com, sizeof(CommandStruct));
	//Increase last index
	commandStack.last = CommandStackIncreaseValue(commandStack.last);
	return false;
}

//Definition of CommandStackClear
//Takes nothing, return nothing
//Clears the stack
void CommandStackClear(void)
{
	StopPerforming();
	commandStack.last = commandStack.first = 0;
}
