#include "CommandIDCounter.h"


CommandIDCounter::CommandIDCounter(void): counter(1)
{
}


CommandIDCounter::~CommandIDCounter(void)
{
}

CommandID CommandIDCounter::GetID()
{
	return counter++;
}

