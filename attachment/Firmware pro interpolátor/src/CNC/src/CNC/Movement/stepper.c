/*
 * stepper.c
 *
 *  Created on: Jul 11, 2012
 *      Author: Honza
 */

#include "stepper.h"
#include "axis.h"
#include "global.h"
#include "GPIOControl/PulseGenerator.h"
#include <math.h>
#include <stm32f4xx.h>

void NotifyStepper(Stepper* stepper, uint8_t index)
{
	float pos = stepper->postion/10000.0f;
	float mmPerStep = stepper->mmPerStep/10000.0f;
	if(fabs(axesBuffer[index] - pos) >= mmPerStep)
	{
		//Do a step
		bool direction = axesBuffer[index] > pos;
		if(direction)
			stepper->postion += stepper->mmPerStep;
		else
			stepper->postion -= stepper->mmPerStep;
		if(stepper->lastDirection != direction)
		{
			//Delay for the driver
			//HACK! Needs to be fixed
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			//End of HACK
		}
		stepper->lastDirection = direction;
		if(stepper->inverted)
			direction = !direction;
		//SetGPIO
		if(direction)
			stepper->GPIO->BSRRL |= stepper->directionPin;
		else
			stepper->GPIO->BSRRH |= stepper->directionPin;

		GenerateSinglePulse(stepper->pulsePin, true);
	}
}

bool IsStepperConfigured(Stepper* stepper)
{
	return true;
}

void InitStepper(Stepper* stepper)
{
	stepper->postion = 0;
	stepper->lastDirection = false;
}

void ConfigStepper(Stepper* stepper, uint8_t* data)
{
	switch(data[0])
	{
		case CONFIG_MOVEMENT_ST_PACKAGE:
		{
			if(data[1] >= 8)
			{
				SetError(ERR_STEPPER_WRONG_PULSEPIN);
			}
			stepper->pulsePin = data[1];

			stepper->GPIO = GetGPIOFromEnum(data[2]);
			stepper->directionPin = (uint16_t)(1<<data[3]);
			EnableGPIOClock(data[2]);
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Pin = stepper->directionPin;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
			GPIO_Init(stepper->GPIO, &GPIO_InitStructure);

			stepper->inverted = data[4] != 0;

			stepper->mmPerStep = *((uint32_t*)(data+5));
		}
		break;

		case CONFIG_MOVEMENT_ST_STEPS:
		{
			stepper->mmPerStep = *((float*)(data+1));
		}
		break;

		case CONFIG_MOVEMENT_ST_PULSE:
		{
			if(data[1] >= 8)
			{
				SetError(ERR_STEPPER_WRONG_PULSEPIN);
			}
			stepper->pulsePin = data[1];
		}
		break;

		case CONFIG_MOVEMENT_ST_DIRECTION:
		{
			stepper->GPIO = GetGPIOFromEnum(data[1]);
			stepper->directionPin = (uint16_t)(1<<data[2]);
			EnableGPIOClock(data[1]);
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Pin = stepper->directionPin;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
			GPIO_Init(stepper->GPIO, &GPIO_InitStructure);
		}
		break;

		case CONFIG_MOVEMENT_ST_INVERTED:
		{
			stepper->inverted = data[1] != 0;
		}
		break;

	}
}
