#ifndef COMMAND_H_
#define COMMAND_H_

#include "stm32f4xx.h"
#include "bool.h"

#include "CommandStruct\commandStruct.h"

//Functions for command
CommandRet RedistributeCommandForPerf(CommandStruct* command, bool fromStack, bool stopOther);

#endif /* COMMAND_H_ */
