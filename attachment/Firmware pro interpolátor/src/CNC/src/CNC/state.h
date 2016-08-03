#ifndef STATE_H_
#define STATE_H_

#include "command.h"
#include "CNC_conf.h"

//State component for containing state - what is the machine doing

//Data formats
typedef struct
{
	CommandID buffer[STATE_BUFF];//Buffer for commands
	uint8_t capacity;//Number of items in array
	CommandID lastItem, currentItem;
}	StateStruct;
__IO extern StateStruct state;

//Functions
void ResetState(void);//Resets state to initial value
void SetActiveCommand(CommandID c);//Sets active command
void CommandDone(CommandID c);//Command done
void StateProcessMessage(uint8_t* message);//Process message from PC



#endif /* STATE_H_ */
