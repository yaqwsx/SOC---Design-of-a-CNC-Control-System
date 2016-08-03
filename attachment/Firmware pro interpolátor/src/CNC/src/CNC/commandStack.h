#ifndef COMMANDSTACK_H_
#define COMMANDSTACK_H_

#include "command.h"
#include "CNC_conf.h"

//Defines component commandStack
//Once there's a command in stack, it starts to performing.

//Data structure
typedef struct
{
	CommandStruct buffer[COMMANDSTACK_BUFF];//Buffer for data
	uint16_t first;//index to first item in stack
	uint16_t last;//index to the last + 1 item in stack
	CommandID lastItem;//ID of the last processed item
	CommandID curItem;//ID of currently performed item
} CommandStackStruct;

__IO extern CommandStackStruct commandStack;//Command stack
__IO extern bool continuePerf;//Bool value, indicates whether stop performing or stop

void ResetCommandStack(void);//Resets command stack
bool ContinuePerforming(void);//Return if the performing should continue or not
void StopPerforming(void);//Stops the performing of command, after StartPerforming
//The command is finished, respectively re-performed
void StartPerforming(void);//Starts performing commands in stack
void CommandStackProcessMessage(uint8_t* message);//Process message from PC


#endif /* COMMANDSTACK_H_ */
