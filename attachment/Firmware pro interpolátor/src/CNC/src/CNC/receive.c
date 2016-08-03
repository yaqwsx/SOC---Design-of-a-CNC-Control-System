#include "receive.h"
#include <string.h>
#include "global.h"
#include "USB/VDclass.h"
#include "errState.h"
#include "commandStack.h"
#include "config.h"


//data for receive
//Data struct for receiving data
__IO ReceiveStruct receive;

//Definition of ResetReceive
//No params, no return value
//Resets/Initializes receive module
void ResetReceive(void)
{
	memset((unsigned char*)(&receive), 0, sizeof(receive));
	//Init counters to default state
	PrepareReceive();
}

//Definition of ProcessPacket
//Params: pointer to array of data, length VD_OUT_PPACKET, no return value
//Processes the incoming data
void ProcessPacket(uint8_t* packet)
{
	//Check, if this is the first packet of data
	if(receive.target == 0)
	{
		//This is the first packet
		receive.target = packet[0];
		if(receive.target > RECEIVE_BUFF)
		{
			//Throw an error - this shouldn't occur
			SetError(ERR_TOO_MANY_DATA);
			return;
		}
	}
	//Copy data into buffer
	memcpy((void*)(receive.buffer + VD_OUT_PACKET*receive.received), packet, VD_OUT_PACKET);
	//Increase internal counter
	receive.received++;

	if(receive.received == receive.target)
	{
		//Data transfer complete, distribute data
		DistributeData();
		//Prepare for next transmission
		PrepareReceive();
	}

}

//Definition of PrepareReceive
//No params, no return value
//Prepares structure for new data - clears counters, etc.
void PrepareReceive(void)
{
	receive.target = receive.received = 0;
}

//Definition of DistributeData
//No params, no return value
//Distributes data to other function object
void DistributeData(void)
{
	switch(receive.buffer[1])
	{
		case RECEIVER_ERRSTATE://errState component
			ErrStateProcessMessage(receive.buffer+2);
			break;
		case RECEIVER_STATE://state component
			StateProcessMessage(receive.buffer+2);
			break;
		case RECEIVER_COMMANDSTACK:
			CommandStackProcessMessage(receive.buffer+2);
			break;
		case RECEIVER_CONFIG:
			ConfigProcessMessage(receive.buffer+2);
			break;
		//Todo: More error handling
		default:
			// TODO: There should be error handling - this shouldn't occur
			break;
	}
}

//Definiton of SendData
//First argument is pointer to data, send one is lenght of data, which
//doesn't need to be exactly VD_IN_PACKET
//Returns USBD_Status
//Basically it's just wrap around USBD_VD_SendData
USBD_Status SendData(uint8_t* data, uint16_t length)
{
	return USBD_VD_SendData(&USB_OTG_dev, data, length);
}
