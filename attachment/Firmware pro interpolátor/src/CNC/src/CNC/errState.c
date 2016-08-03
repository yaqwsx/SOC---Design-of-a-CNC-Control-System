#include "errState.h"
#include "global.h"
#include "CNC.h"
#include <string.h>
#include <stdlib.h>
#include "global.h"


//Data definition
__IO ErrStateStruct errState;//Data for state component

//Function definition
//Definition of ResetState
//Resets setting and data of state component
//Takes and returns nothing
void ResetErrState(void)
{
	memset((unsigned char*)(&errState), 0, sizeof(errState));
};

//Definition of SetError
//Param is given error code, define in ErrorCodes
//Sets error state
void SetError(uint16_t code)
{
	//Check if the error is set
	uint8_t i;
	for(i = 0; i != errState.capacity; i++)
	{
		if(code == errState.buffer[i])
			return;//We found the error message, return
	}
	//Error isn't set
	errState.buffer[errState.capacity] = code;
	errState.capacity++;
	if(errState.capacity == ERRSTATE_BUFF)
	{
		//Fatal error, send message
		SendErrorMessage(ERR_TOO_MANY_ERR);
		//Device reset
		ResetCNC();
		return;
	}
	//Send error message to PC
	SendErrorMessage(code);
}

//Definition of IsOk
//Return Good or BAD depending of existing errors
ErrState IsGood(void)
{
	if(errState.capacity)
		return BAD;
	return GOOD;
}

//Definition of ErrorFixed
//Takes error code, return nothing
//Clears error state
void ErrorFixed(uint16_t code)
{
	//Find the position of error code in buffer
	uint8_t i;
	for(i = 0; i != errState.capacity; i++)
	{
		if(code == errState.buffer[i])
			break;//We found the error message
	}
	if(i == errState.capacity)
		return;//There isn't any given code in buffer
	//Clear the code; decrement the capacity
	errState.capacity--;
	//Move elements to position before
	for(; i != errState.capacity; i++)
	{
		errState.buffer[i] = errState.buffer[i+1];
	}
}

//Definition of SendErrorMeesage
//Takes error code, return nothing
//Sends message in format: uchar length in packets, uchar type (error), uint16 code
void SendErrorMessage(uint16_t code)
{
	//static uint8_t buffer[4];//4 byte buffer
	outBuff[0] = 4/VD_IN_PACKET+1;
	outBuff[1] = ERROR_MESS;//Error message type
	*((uint16_t*)(outBuff + 2)) = code;//Set the error code
	//Send data
	SendData(outBuff, 4);
};

//Definition of ErrStateProcessMessage
//Takes pointer to buffer
//Handles the message from PC
void ErrStateProcessMessage(uint8_t* data)
{
	//Incoming message. First byte is identifier
	switch(*((uint8_t*)(data)))
	{
		case ERRSTATEMESSAGE_RETSTATE: //PC is asking for state; answer is ERRSTATEINFO_MESS
		{
			//Message is in format: length in packets(1), identifier (1), number of errors, array of errors
			uint8_t/** buffer,*/ length;
			length = 3 + 2*errState.capacity;
			//buffer = malloc(length);
			outBuff[0] = length/VD_IN_PACKET + 1;
			outBuff[1] = ERRSTATEINFO_MESS;
			outBuff[2] = errState.capacity;
			memcpy((void*)(outBuff + 3), (void*)(errState.buffer), 2*errState.capacity);
			//Buffer ready, send
			SendData(outBuff, length);
			//free(buffer);
		}
		break;

	}

}
