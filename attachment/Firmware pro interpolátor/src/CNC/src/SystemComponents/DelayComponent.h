#ifndef DELAYCOMPONENT_H_
#define DELAYCOMPONENT_H_

#include "stm32f4xx.h"
#include "bool.h"

//Component for waiting provided by SysTick

//Number of max stimultaneously running timers
#define DELAY_BUFF	32

//Data structs
//delayUnit struct
typedef struct
{
	uint32_t timer;//Time in ms
	uint8_t ID;
	void (*callback)(void);//Callback after reaching 0
	/*If callback set, the delayUnit will be after reaching 0 removed
	 * and the callback called. Otherwise the timer stays set*/
}	delayUnit;

typedef struct
{
	delayUnit buffer[DELAY_BUFF];
	uint8_t capacity;
} DelayComponent;

__IO extern DelayComponent delayComponent;

//Functions
void ResetDelayComponent(void);//Initializes delayComponent
uint8_t AddDelay(uint32_t delay, void (*callback)(void));//Adds new delay
void RemoveDelay(uint8_t ID);//Removes delay
void RemoveDelayOnPosition(uint8_t position);//Remove delay on specified position
uint32_t GetDelay(uint8_t ID);//Returns state of given delay
bool DecreaseDelay(uint8_t position);//Decreases given timer

#endif /* DELAYCOMPONENT_H_ */
