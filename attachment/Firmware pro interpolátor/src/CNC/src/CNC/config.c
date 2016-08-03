#include "config.h"
#include "axis.h"
#include "movement/movement.h"
#include "GPIOControl/PulseGenerator.h"

//Data definition
uint32_t axisConfig;


//Function definition
void ConfigProcessMessage(uint8_t* message)//Process message from PC
{
	switch(message[0])
	{
		case CONFIG_REC_AXIS:
			AxisConfig(message + 1);
			break;
		case CONFIG_REC_MOVEMENT:
			MovementConfig(message+1);
			break;
	}
}

void ResetConfig()//Resets config
{
	axisConfig = 0;
}
bool IsConfigured()//Defines whether machine is ready to continue
{
	/*if(axisConfig != AXIS_CONF_FULL)
		return false;*/
	if(!IsMovementConfigured())
		return false;
	if(!IsPulseGeneratorConfigured())
		return false;
		//ToDo - add more controls
	return true;
}
