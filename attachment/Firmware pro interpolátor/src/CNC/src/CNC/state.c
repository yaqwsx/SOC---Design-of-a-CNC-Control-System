#include "state.h"
#include <string.h>
#include <stdlib.h>
#include "errState.h"
#include "global.h"
#include "CNC.h"
#include "axis.h"
#include "Movement/movement.h"


//Data
__IO StateStruct state;

//Function definitions
//ResetState
//takes nathing, return nothing
//Resets the state to inital value
void ResetState(void)
{
	memset((unsigned char*)(&state), 0, sizeof(state));
}

//Definition of SetActiveCommand
//Param is ID of performed command
//Set state to performing given command
void SetActiveCommand(CommandID c)
{
	//Check if the command is set
	/*	uint8_t i;
	for(i = 0; i != state.capacity; i++)
	{
		if(c == state.buffer[i])
			return;//We found the command, return
	}
	//Error isn't set
	state.capacity++;
	if(state.capacity == ERRSTATE_BUFF)
	{
		//Fatal error, send message
		SendErrorMessage(ERR_TOO_MANY_COM);
		//Device reset
		ResetCNC();
		return;
	}
	state.buffer[state.capacity] = c;*/
	state.currentItem = c;
}

//Definition of CommandDone
//Takes command ID, return nothing
//Clears state of performed command
void CommandDone(CommandID ID)
{
	//Find the position of error code in buffer
/*	uint8_t i;
	for(i = 0; i != state.capacity; i++)
	{
		if(ID == state.buffer[i])
			break;//We found the ID
	}
	if(i == state.capacity)
		return;//There isn't any given ID in buffer
	//Clear the ID; decrement the capacity
	if(state.capacity == 0)
		__asm("nop");
	state.capacity--;
	//Move elements to position before
	for(; i != state.capacity; i++)
	{
		state.buffer[i] = state.buffer[i+1];
	}*/
	state.lastItem = ID;
}

//Definition of StateProcessMessage
//Takes pointer to buffer
//Handles the message from PC
void StateProcessMessage(uint8_t* data)
{
	//Incoming message. First byte is identifier
	switch(*((uint8_t*)(data)))
	{
		case STATEMESSAGE_RETSTATE: //PC is asking for state; answer is PER_COM_MESS
		{
			//Message is in format: length in packets(1), identifier (1), number of commands, array of IDs
			uint8_t/** buffer, */length;
			length = 3 + sizeof(CommandID)*state.capacity;
			//buffer = malloc(length);
			outBuff[0] = length/VD_IN_PACKET + 1;
			outBuff[1] = PER_COM_MESS;
			outBuff[2] = state.capacity;
			memcpy((void*)(outBuff + 3), (void*)(state.buffer), sizeof(CommandID)*state.capacity);
			//Buffer ready, send
			SendData(outBuff, length);
			//free(buffer);
		}
		break;

		case STATEMESSAGE_POSITION:
		{
			//uint8_t* buffer;
			uint8_t length;
			length = 2 + 2*sizeof(axesBuffer);
			//buffer = malloc(length);
			outBuff[0] = length/VD_IN_PACKET + 1;
			outBuff[1] = AXES_POSITION;
			memcpy((void*)(outBuff + 2), (void*)(&axesBuffer), sizeof(axesBuffer));
			//Temp
			*((float*)(outBuff + 2+sizeof(axesBuffer))) = movement[0].stepper.postion/10000.0f;
			*((float*)(outBuff + 2+4+sizeof(axesBuffer))) = movement[1].stepper.postion/10000.0f;
			//endOfTemp
			//Buffer ready, send
			SendData(outBuff, length);
			//free(buffer);
		}
		break;

		case STATEMESSAGE_LAST_ITEM:
		{
			//uint8_t* buffer;
			uint8_t length;
			length = 2 + sizeof(CommandID);
			//buffer = malloc(length);
			outBuff[0] = length/VD_IN_PACKET + 1;
			outBuff[1] = LAST_PROC_ITEM_STATE;
			*((uint32_t*)(outBuff + 2)) = state.lastItem;
			//Buffer ready, send
			SendData(outBuff, length);
			//free(buffer);
		}

		case STATEMESSAGE_RESET_LAST_ITEM:
		{
			state.lastItem = 0;
		}
		break;

	}

}

