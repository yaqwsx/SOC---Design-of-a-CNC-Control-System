#ifndef GLOBAL_H_
#define GLOBAL_H_

//Include enumerations common for PC and MCU
#include "C:\Users\Honza\Documents\Visual Studio 2010\Projects\CNC\CNC\CommunicationEnumerations.h"
//Other include
#include "stm32f4_discovery.h"
#include "usb_core.h"
#include "receive.h"
#include "errState.h"
#include "state.h"
#include "bool.h"

USB_OTG_CORE_HANDLE  USB_OTG_dev;//Structure for USB device
typedef enum{GPIO_A = 0, GPIO_B, GPIO_C, GPIO_D, GPIO_E, GPIO_F, GPIO_G, GPIO_H, GPIO_I} GPIOenum;

extern uint8_t outBuff[128];

GPIO_TypeDef* GetGPIOFromEnum(GPIOenum g);
void EnableGPIOClock(GPIOenum g);





#endif /* GLOBAL_H_ */
