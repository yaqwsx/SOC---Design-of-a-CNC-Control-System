#ifndef GPIO_H_
#define GPIO_H_


#include "command.h"

//Definition of GPIO component


void ResetGPIOCommand(void);
CommandRet ProcessGPIOCommand(__IO CommandStruct* command, bool fromStack);

#endif /* GPIO_H_ */
