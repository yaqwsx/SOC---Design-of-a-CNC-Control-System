#ifndef COMMANDSTRUCT_H_
#define COMMANDSTRUCT_H_

#include "AxisCommand.h"
#include <stdint.h>

//Structure for command

typedef uint32_t CommandID;

typedef struct
{
	uint8_t receiver;//Which component can handle this command
	uint8_t type;//Type of command
	CommandID ID;//Unique ID for command
	//Note for ID - there's no need for recycling.
	//Even if the machine would do 1000 commands per second,
	//the ID would run out after 49 days
	union
	{
		uint32_t delay;//Delay in ms for waiting
		uint8_t LED;//Led argument for GPIO
		AxisLine line;//line argument for AxisComponent
		AxisSine sine;//sine arguments for axis component
		AxisCircle circle;//circle argument for axis component
	};
}	CommandStruct;

typedef enum {COMMAND_PROC_OK, COMMAND_PROC_INTERRUPT} CommandRet;//Defines return state of command processing
#endif /* COMMANDSTRUCT_H_ */
