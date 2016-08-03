#ifndef AXIS_H_
#define AXIS_H_

#include "bool.h"
#include "command.h"
#include "CNC_conf.h"

//Component for axes movement


typedef struct
{
	float maxLinearAcceleration;//Maximal acceleration of kinematics
	float maxLinearJerk;//Maximal jerk of kinematic
	uint32_t updatePeriod;//Period of position update in microseconds.
	bool IsRunning;//Defines if the movement is in progress
}	AxisData;//configuration struct for axis

//Masks for notify component of AxisData
#define NOTIFY_MASK_SERVO	0x80
#define NOTIFY_MASK_ACTIVE	0x40
#define NOTIFY_MASK_INDEX	0x3F

typedef enum{ACCELERATING, CONSTANT, BRAKING} AxisMovementState;

typedef struct
{
	float* position;
	float ATpi, ATpiB;
	float start, constant, brake, end;//coordinates for start of linear and breaking movement
	float v, v0;//
}	AxisMovementBufferLine;

typedef struct
{
	float* position;
	float final;//coordinates for start of linear and breaking movement
	float crossProduct, center, R, u;
}	AxisMovementBufferCircle;

typedef struct
{
	float* position;
	float ATpi;
	float start, v, end;
}	AxisMovementBufferSine;

typedef struct
{
	float T, TB, t;
}	AxisMovementLineCommon;

typedef struct
{
	float T;
}	AxisMovementSineCommon;

typedef struct
{
	float w, w0;
	float alpha;
	float constant, brake, end;
	float ATpi, ATpiB, T, TB;
}	AxisMovementCircleCommon;

typedef struct
{
	enum {AXIS_MOVE_LINEAR, AXIS_MOVE_CIRCLE, AXIS_MOVE_SINE} type;
	uint32_t time;//Time from beginning of the movement
	float timeF;//Time in float
	AxisMovementState state;
	bool done;
	union
	{
			AxisMovementLineCommon cLine;
			AxisMovementSineCommon cSine;
			AxisMovementCircleCommon cCircle;
	};
	union
	{
			AxisMovementBufferLine line;
			AxisMovementBufferCircle circle;
			AxisMovementBufferSine sine;

	} data[3];
}	AxisMovementBuffer;//Buffer for axes movement


extern __IO float axesBuffer[3];//output buffer for axis
extern __IO AxisData axisData;
extern __IO AxisMovementBuffer axisMovementBuffer;//Buffer for movement - sharing data among
	//preparing function and timer

#define TIMERVALUE	axisData.updatePeriod*AXIS_TIM_FREQ/AXIS_UPDATE_UNIT
#define AXIS_UPDATE_T	(axisData.updatePeriod)

//Functions
void ResetAxis(void);
CommandRet ProcessAxisCommand(__IO CommandStruct* command, bool fromStack);
void UpdatePosition(void);//Timer calls this function to update position.
void AxisConfig(uint8_t* data);//Handles incoming config data
void UpdateTimerPeriod(void);
CommandRet ProcessAxisLine(__IO AxisLine* line);//Processes one line of movement
CommandRet ProcessAxisSine(__IO AxisSine* sine);//Processes sine movement
CommandRet ProcessAxisCircle(__IO AxisCircle* circle);//Processes circle or arc movement
bool UpdateAxisPosition();//Updates position of single axis, return true
	//if control should be notified
bool UpdateLinearAxesPosition();//Updates linear motion for all axes
bool UpdateCircleAxesPosition();//Updates circle motion for all axes
bool UpdateSineAxesPosition();//Updates sine motion for all axes
void UpdateLinearAxisPosition(uint8_t buff);//Updates linear motion for single axis
void UpdateCircleAxisPosition(uint8_t buff);//Updates circle motion for single axis
void UpdateSineAxisPosition(uint8_t buff);//Updates sine motion for single axis
bool HasPassed(float current, float target, bool direction);//Tells whether targeted segment has already passed

#endif /* AXIS_H_ */
