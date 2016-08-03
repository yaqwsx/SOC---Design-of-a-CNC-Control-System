#ifndef RECEIVE_H_
#define RECEIVE_H_

#include "USB/usbd_conf.h"
#include "CNC_conf.h"
#include "usbd_core.h"

//Receive structure
typedef struct
{
	uint8_t buffer[VD_OUT_PACKET*RECEIVE_BUFF];//Buffer for incoming data
	uint8_t target;//number of packets to be received
	uint8_t received;//number of already received packets
}	ReceiveStruct;

__IO extern ReceiveStruct receive;//Data for receiving data

//Functions for receiving
void ResetReceive(void);//Resets/Initializes receive
void ProcessPacket(uint8_t* packet);//Takes incoming packet and process it
void DistributeData(void);//After receiving complete data, distributes data to specified component component
void PrepareReceive(void);//Prepares receive for next incoming data

//Functions for transmitting
USBD_Status SendData(uint8_t* data, uint16_t length);//Sends data back to PC




#endif /* RECEIVE_H_ */
