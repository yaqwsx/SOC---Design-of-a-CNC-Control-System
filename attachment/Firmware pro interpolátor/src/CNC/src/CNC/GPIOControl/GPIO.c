
#include "GPIO.h"
#include "global.h"

//Data structs
Led_TypeDef LEDs[4] = {LED3, LED4, LED5, LED6};

//Definition of ResetGPIOCommand
//Inits GPIO
void ResetGPIOCommand(void)
{
	STM32F4_Discovery_LEDInit(LED3);
	STM32F4_Discovery_LEDInit(LED4);
	STM32F4_Discovery_LEDInit(LED5);
	STM32F4_Discovery_LEDInit(LED6);
	STM32F4_Discovery_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);
}

//Definition of ProcessGPIOCommand
//Processes a command from PC
CommandRet ProcessGPIOCommand(__IO CommandStruct* command, bool fromStack)
{
	switch(command->type)
	{
		case COM_GPIO_LED_ON:
			STM32F4_Discovery_LEDOn(LEDs[command->LED]);
			break;
		case COM_GPIO_LED_OFF:
			STM32F4_Discovery_LEDOff(LEDs[command->LED]);
			break;
		case COM_GPIO_LED_TOGGLE:
			STM32F4_Discovery_LEDToggle(LEDs[command->LED]);
			break;
	}
	return COMMAND_PROC_OK;
}
