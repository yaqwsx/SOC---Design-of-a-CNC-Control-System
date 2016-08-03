#include "global.h"

//Struct for USB device
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment = 4
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;


uint8_t outBuff[128];

GPIO_TypeDef* GetGPIOFromEnum(GPIOenum g)
{
	switch(g)
	{
		case GPIO_A:
			return GPIOA;
		case GPIO_B:
			return GPIOB;
		case GPIO_C:
			return GPIOC;
		case GPIO_D:
			return GPIOD;
		case GPIO_E:
			return GPIOE;
		case GPIO_F:
			return GPIOF;
		case GPIO_G:
			return GPIOG;
		case GPIO_H:
			return GPIOH;
		case GPIO_I:
			return GPIOI;
	}
	return 0;
}

void EnableGPIOClock(GPIOenum g)
{
	switch(g)
	{
		case GPIO_A:
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		case GPIO_B:
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		case GPIO_C:
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
		case GPIO_D:
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
		case GPIO_E:
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		case GPIO_F:
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
		case GPIO_G:
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
		case GPIO_H:
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
		case GPIO_I:
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
	}
}

