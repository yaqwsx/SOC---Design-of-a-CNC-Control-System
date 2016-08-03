#include "global.h"
#include "state.h"
#include "globalCommands.h"
#include "GPIOControl/GPIO.h"
#include "axis.h"
#include "GPIOControl/PulseGenerator.h"
#include "Movement/movement.h"


//Function definition

//Definition of ResetCNC
//Takes no params
//Resets the whole CNC part to initial state, like device restart.
//All data are lost
void ResetCNC(void)
{
	ResetState();
	ResetReceive();
	ResetErrState();
	ResetGlobalCommand();
	ResetGPIOCommand();
	ResetAxis();
	InitPulseGenerator();
	InitMovement();
};
