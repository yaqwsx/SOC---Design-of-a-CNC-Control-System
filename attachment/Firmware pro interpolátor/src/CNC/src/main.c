
/**
  ******************************************************************************
  * @file    main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/*
 * Autor: yaqwsx, založeno na zdrojových kódech k seriálu
 * 		Zaèínáme s STM32F4Discovery 7. od mard (mcu.cz)
*/



/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "USB/VDclass.h"
#include "usbd_usr.h"
#include "USB/usbd_desc.h"
#include "CNC.h"
#include "DelayComponent.h"
#include "CNC/GPIOControl/PulseGenerator.h"
#include "CNC/Movement/movement.h"

/** @addtogroup STM32F4-Discovery_USB_HID
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC1_CDR_Address    ((uint32_t)0x40012308)


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


  

__IO uint8_t UserButtonPressed = 0x00;
uint8_t Buffer[64];

__IO size_t num = 0;


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{


	USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_HS
	USB_OTG_HS_CORE_ID,
#else
	USB_OTG_FS_CORE_ID,
#endif
	&USR_desc,
	&USBD_VD_cb,
	&USR_cb);

	//Init delayCmponent
	ResetDelayComponent();
	//Init CNC part
	ResetCNC();
	axisData.updatePeriod = 100;//100 us
	UpdateTimerPeriod();
	//TODO: For debug only, remove
	axisData.maxLinearAcceleration = 5;
	axisConfig = AXIS_CONF_FULL;
	movement[0].type = STEPPER;
	movement[0].stepper.pulsePin = 0;
	movement[0].stepper.GPIO = GPIOA;
	//Start performing

	while(true)
		StartPerforming();

  while (1)
  {
	  if (STM32F4_Discovery_PBGetState(BUTTON_USER) == Bit_SET)
	  {
		  if (UserButtonPressed != 0x01)
		  {
			  /* new action */
			  UserButtonPressed = 0x01;
			  //Delay(0xFF);
			  GenerateSinglePulse(0, true);
		  }
	  }
	  else
	  {
		  if (UserButtonPressed != 0x00)
		  	 {
	  			  UserButtonPressed = 0x00;
		  	 }
	  	  }
  }
}




/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  if (nTime != 0x00)
  { 
    nTime--;
  }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
