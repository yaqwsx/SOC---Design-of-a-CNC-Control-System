#ifndef GLOBALCOMMANDS_H_
#define GLOBALCOMMANDS_H_

#include "command.h"

//Component for global commands - f.e: waiting

void ResetGlobalCommand(void);
CommandRet ProcessGlobalCommand(__IO CommandStruct* command, bool fromStack);

#endif /* GLOBALCOMMANDS_H_ */
