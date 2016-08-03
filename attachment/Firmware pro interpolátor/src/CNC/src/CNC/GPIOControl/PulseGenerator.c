#include "PulseGenerator.h"
#include <string.h>
#include <assert.h>

PulseGeneratorStruct PulseGeneratorData[6];
TIM_OCInitTypeDef  TIM_OCInitStructurePulseGen;

void InitPulseGenerator()
{
	memset(&PulseGeneratorData, 0, sizeof(PulseGeneratorData));
	GPIO_InitTypeDef GPIO_InitStructure;
	/* TIM3 & TIM12 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);

	/* GPIOx clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* Configure pins */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//Prepare timers in CC mode
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TIM12, &TIM_TimeBaseStructure);

	TIM_PrescalerConfig(TIM3, SystemCoreClock/2/PULSE_TIM_FREQ -1, TIM_PSCReloadMode_Immediate);

	//Prepare channels
	TIM_OCInitStructurePulseGen.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructurePulseGen.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructurePulseGen.TIM_Pulse = PULSE_TIM_FREQ/MAX_TRIG_FREQ;
	TIM_OCInitStructurePulseGen.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM3, &TIM_OCInitStructurePulseGen);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_OC1Init(TIM12, &TIM_OCInitStructurePulseGen);
	TIM_OC1PreloadConfig(TIM12, TIM_OCPreload_Disable);
	TIM_OC2Init(TIM3, &TIM_OCInitStructurePulseGen);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Disable);
	TIM_OC2Init(TIM12, &TIM_OCInitStructurePulseGen);
	TIM_OC2PreloadConfig(TIM12, TIM_OCPreload_Disable);
	TIM_OC3Init(TIM3, &TIM_OCInitStructurePulseGen);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
	TIM_OC4Init(TIM3, &TIM_OCInitStructurePulseGen);
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Disable);

	TIM_SetCompare1(TIM3, PULSE_TIM_FREQ/MAX_TRIG_FREQ);
	TIM_SetCompare2(TIM3, PULSE_TIM_FREQ/MAX_TRIG_FREQ);
	TIM_SetCompare3(TIM3, PULSE_TIM_FREQ/MAX_TRIG_FREQ);
	TIM_SetCompare4(TIM3, PULSE_TIM_FREQ/MAX_TRIG_FREQ);

	//Set interrupts
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = TIM8_BRK_TIM12_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	//TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	//TIM_ITConfig(TIM12, TIM_IT_Update, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE);

	//Enable timers
	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM12, ENABLE);

}

bool IsPulseGeneratorConfigured()
{
	return true;
}

void GenerateSinglePulse(uint8_t pin, bool polarity)//Duration in us
{
	uint16_t capture;
	switch(pin)
	{
		case 0:
			//Bug in this code
			/*if(polarity)
				GPIOA->BSRRH |= 1 << 6;
			else
				GPIOA->BSRRL |= 1 << 6;
			STM32F4_Discovery_LEDOn(LED3);
			PulseGeneratorData[pin].polarity = polarity;
			capture = TIM_GetCapture1(TIM3);
			TIM_SetCompare1(TIM3, capture + PULSE_TIM_FREQ/MAX_TRIG_FREQ);
			PulseGeneratorData[pin].set = true;*/


			GPIOA->BSRRH |= 1 << 6;

			//HACK! Needs to be fixed
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			//End of HACK

			GPIOA->BSRRL |= 1 << 6;
			break;
		case 1:
			//Bug in this code
			/*if(polarity)
				GPIOA->BSRRH |= 1 << 7;
			else
				GPIOA->BSRRL |= 1 << 7;
			PulseGeneratorData[pin].polarity = polarity;
			capture = TIM_GetCapture2(TIM3);
			TIM_SetCompare2(TIM3, capture + PULSE_TIM_FREQ/MAX_TRIG_FREQ);
			PulseGeneratorData[pin].set = true;
			break;*/

			GPIOA->BSRRH |= 1 << 7;

			//HACK! Needs to be fixed
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
			//End of HACK

			GPIOA->BSRRL |= 1 << 7;

			break;
		case 3:
			//Bug in this code
			/*if(polarity)
				GPIOB->BSRRH |= 1 << 0;
			else
				GPIOB->BSRRL |= 1 << 0;
			PulseGeneratorData[pin].polarity = polarity;
			capture = TIM_GetCapture3(TIM3);
			TIM_SetCompare3(TIM3, capture + PULSE_TIM_FREQ/MAX_TRIG_FREQ);
			PulseGeneratorData[pin].set = true;*/
			break;
	}
}
