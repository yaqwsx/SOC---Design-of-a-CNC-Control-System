#ifndef STEPPER_H_
#define STEPPER_H_
#include "global.h"



typedef struct
{
	uint32_t mmPerStep;
	int32_t postion;
	uint8_t pulsePin;
	GPIO_TypeDef* GPIO;
	uint16_t directionPin;
	bool inverted;
	bool lastDirection;
}	Stepper;


void NotifyStepper(Stepper* stepper, uint8_t index);
bool IsStepperConfigured(Stepper* stepper);
void InitStepper(Stepper* stepper);
void ConfigStepper(Stepper* stepper, uint8_t* data);

#endif /* STEPPER_H_ */
