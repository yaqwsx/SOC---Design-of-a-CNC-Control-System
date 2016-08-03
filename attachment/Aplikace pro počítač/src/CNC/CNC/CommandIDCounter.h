#pragma once
#include "D:\Users\Honza\Atollic\TrueSTUDIO\STM32_workspace\CNC\src\CNC\CommandStruct\commandStruct.h"


//This class represents commandID generator.
class CommandIDCounter
{
	public:
		CommandIDCounter(void);
		~CommandIDCounter(void);
		CommandID GetID();
	private:
		CommandID counter;
};

