#include "movement.h"
#include <string.h>
#include "../errState.h"


__IO MovementStruct movement[5];

void NotifyMovement(uint8_t index)
{
	switch(movement[index].type)
	{
		case STEPPER:
			NotifyStepper(&(movement[index].stepper), index);
			break;
		case SERVO:
			NotifyServo(&(movement[index].servo), index);
			break;
		case UNUSED:
			break;
	}
}


void InitMovement()
{
	memset(&movement, 0, sizeof(movement));
	uint8_t i;
	for(i = 0; i != 5; i++)
	{
		//ToDo: Nonsence - what is this switch here for?
		switch(movement[i].type)
		{
			case STEPPER:
				InitStepper(&(movement[i].stepper));
				break;
			case SERVO:
				InitServo(&(movement[i].servo));
				break;
			case UNUSED:
				break;
		}

	}
}

bool IsMovementConfigured()
{
	bool configured = true;
	uint8_t i;
		for(i = 0; i != 5 && configured == true; i++)
			switch(movement[i].type)
			{
				case STEPPER:
					configured &= IsStepperConfigured(&(movement[i].stepper));
					break;
				case SERVO:
					configured &= IsServoConfigured(&(movement[i].servo));
					break;
				case UNUSED:
					break;
			}
	return configured;
}

/*
 * Format of configuration data: 1 - type, 1 - index; the rest are data for stepper/servo*/
void MovementConfig(uint8_t* data)
{
	switch(data[0])//Switch type of configuration data
	{
		case CONFIG_MOVEMENT_SETTYPE:
			if(data[2] == 1)
				movement[data[1]].type = STEPPER;
			else if(data[2] == 2)
				movement[data[1]].type = SERVO;
			else
				movement[data[1]].type = UNUSED;
			break;
		case CONFIG_MOVEMENT_INDIVIDUAL:
			switch(movement[data[1]].type)
			{
				case STEPPER:
					ConfigStepper(&(movement[data[1]].stepper), data + 2);
					break;
				case SERVO:
					ConfigServo(&(movement[data[1]].servo), data + 2);
					break;
				case UNUSED://Shouldn't occur
					SetError(ERR_MOVEMENTUNUSED);
					break;
			}
			break;

	}
}
