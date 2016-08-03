#ifndef PULSEGENERATOR_H_
#define PULSEGENERATOR_H_

#include "global.h"

//Component for generating pulses
/*
 * Realised via two timers TIM3 and TIM12
 * There are 6 pins for this purpose 0-3 TIM3, 5-6 TIM4*/

typedef enum{LOW = 0, HIGH} Polarity;

typedef struct
{
	bool polarity;
	bool set;
} PulseGeneratorStruct;

extern PulseGeneratorStruct PulseGeneratorData[6];
extern TIM_OCInitTypeDef  TIM_OCInitStructurePulseGen;

void InitPulseGenerator();
bool IsPulseGeneratorConfigured();
void GenerateSinglePulse(uint8_t pin, bool polarity);

#endif /* PULSEGENERATOR_H_ */
