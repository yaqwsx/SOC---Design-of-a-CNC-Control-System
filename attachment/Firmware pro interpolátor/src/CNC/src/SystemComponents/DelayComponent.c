#include "DelayComponent.h"
#include "errState.h"
#include "global.h"
#include <string.h>


//Data
__IO DelayComponent delayComponent;

//Function declaration

//Definition of ResetDelayComponent
//Takes nothing, returns nothing
//Reset delay component to initial state
void ResetDelayComponent(void)
{
	if (SysTick_Config(SystemCoreClock / 1000))
  	{
		/* Capture error */
		SetError(ERR_SYSTICKFAIL);
		return;
  	}
	memset((void*)(&delayComponent),0 , sizeof(delayComponent));
}

//Definition of AddDelay
//Creates new delay from given parameters
//If returned 0, error occured
uint8_t AddDelay(uint32_t delay, void (*callback)(void))
{
	if(delayComponent.capacity == DELAY_BUFF)
	{
		//Error - buffer overflow
		SetError(ERR_DELAYOVERFLOW);
		return 0;
	}
	//Find new ID
	uint8_t ID = 0;
	for(ID = 1; ID != DELAY_BUFF; ID++)//ID is count from 1, not 0
	{
		uint8_t i, count = 0;
		for(i = 0; i != delayComponent.capacity; i++)
			if(delayComponent.buffer[i].ID == ID)
				count++;
		if(count == 0)
			break;//We found free ID
	}
	delayComponent.buffer[delayComponent.capacity].timer = delay;
	delayComponent.buffer[delayComponent.capacity].ID = ID;
	delayComponent.buffer[delayComponent.capacity].callback = callback;
	//Increase index
	delayComponent.capacity++;
	//return index
	return ID;
}

//Definition of RemoveDelay
//Removes delay from buffer
void RemoveDelay(uint8_t ID)
{
	//Check the position of delay
	uint8_t position;
	for(position = 0; position != DELAY_BUFF; position++)
	{
		if(delayComponent.buffer[position].ID == ID)
			break;//Position found
	}
	if(position == DELAY_BUFF)
		return;//The delay doesn't exist
	RemoveDelayOnPosition(position);
}

//Definition of RemoveDelayOnPostion
//Removes delay on given position - for internal use only
void RemoveDelayOnPosition(uint8_t position)
{
	//Copy all other delay in array
	delayComponent.capacity--;//Decrease capacity
	for(; position != delayComponent.capacity; position++)
		delayComponent.buffer[position] = delayComponent.buffer[position+1];
}

//Definition of GetDelay
//Returns state of given delay
uint32_t GetDelay(uint8_t ID)
{
	uint8_t position;
	for(position = 0; position != DELAY_BUFF; position++)
	{
		if(delayComponent.buffer[position].ID == ID)
			return delayComponent.buffer[position].timer;
	}
	return 0;
}

//Definition of DecreaseDelay
//Decreases delay on given position
//Return true if the delay reaches zero
bool DecreaseDelay(uint8_t position)
{
	if(delayComponent.buffer[position].timer ==  0)
		return true;
	delayComponent.buffer[position].timer--;//Decrease
	if(delayComponent.buffer[position].timer ==  0)
			return true;
	return false;
}
