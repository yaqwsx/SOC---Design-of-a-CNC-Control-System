#ifndef ERRSTATE_H_
#define ERRSTATE_H_

#include "stm32f4xx.h"
#include "CNC_conf.h"

typedef struct
{
	uint8_t capacity;//Number of items in array
	uint16_t buffer[STATE_BUFF];//Buffer for error codes
}	ErrStateStruct;

__IO extern ErrStateStruct errState;//Data for state component

typedef enum {GOOD=1, BAD=0} ErrState;

//Functions for state component
void SetError(uint16_t code);//Sets error
void ResetErrState(void);//Resets/Initializes state of state component
ErrState IsGood(void);//defines state of device; ToDo: modify to return bool
void ErrorFixed(uint16_t code);//Error fixed, clear state
void SendErrorMessage(uint16_t code);//Send error message to PC
void ErrStateProcessMessage(uint8_t* message);//Process message from PC

#endif /* STATE_H_ */
