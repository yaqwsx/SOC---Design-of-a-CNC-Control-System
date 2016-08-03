#include "axis.h"
#include "global.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include "CNC_conf.h"
#include "SupportFunctions/mathematic.h"
#include <math.h>
#include "commandStack.h"
#include "Movement/movement.h"

__IO float axesBuffer[3];//Data
__IO AxisData axisData;
__IO AxisMovementBuffer axisMovementBuffer;

//Private functions



//Function definition
//Definition of ResetAxis
//Initializes Axis component
void ResetAxis(void)
{
	memset(&axisData, 0, sizeof(axisData));
	memset(&axesBuffer, 0, sizeof(axesBuffer));
	UpdateTimerPeriod();
}

void AxisConfig(uint8_t* data)//Handles incoming config data
{
	switch(data[0])
	{
		case CONFIG_AXIS_ACCELERATION://Follow by two float for acceleration
			/*First linear*/
			axisData.maxLinearAcceleration = *((float*)(data+1));
			axisConfig |= AXIS_CONF_ACCELERATION;
			break;
		case CONFIG_AXIS_JERK://Follow by two float for acceleration
			/*First linear*/
			axisData.maxLinearJerk = *((float*)(data+1));
			axisConfig |= AXIS_CONF_JERK;
			break;
		case CONFIG_AXIS_UPDATE_PERIOD://Followed by 32bit unsigned number - period in microseconds
			axisData.updatePeriod = *((uint32_t*)(data+1));
			UpdateTimerPeriod();
			axisConfig |= AXIS_CONF_UPDATE;
			break;

	}
}

void UpdateTimerPeriod(void)
{
	if(axisData.updatePeriod == 0)
	{
		TIM_Cmd(TIM14, DISABLE);
		return;
	}
	//Init clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	//Init interrupt
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM8_TRG_COM_TIM14_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//Base Init
	TIM_TimeBaseInitTypeDef baseInit;
	baseInit.TIM_Period = 65535;
	baseInit.TIM_CounterMode = TIM_CounterMode_Up;
	baseInit.TIM_Prescaler = baseInit.TIM_ClockDivision = 0;
	TIM_TimeBaseInit(TIM14, &baseInit);
	TIM_PrescalerConfig(TIM14, (uint16_t)(SystemCoreClock/2/AXIS_TIM_FREQ-1), TIM_PSCReloadMode_Immediate);
	//Init channel 1
	TIM_OCInitTypeDef OCInit;
	OCInit.TIM_OCMode = TIM_OCMode_Timing;
	OCInit.TIM_OutputState = TIM_OutputState_Enable;
	OCInit.TIM_OCPolarity = TIM_OCPolarity_High;
	OCInit.TIM_Pulse = TIMERVALUE;
	TIM_OC1Init(TIM14, &OCInit);
	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Disable);
	//Enable interrupt
	TIM_ITConfig(TIM14, TIM_IT_CC1, ENABLE);
	//Enable timer
	TIM_Cmd(TIM14, ENABLE);
}

void UpdatePosition(void)
{
	if(!axisData.IsRunning)
		return;//There's no movement in progress, skip

	if(UpdateAxisPosition())
		axisData.IsRunning = false;//Processed, stop

}

//Definition of ProcessAxisCommand
//Processes a command from PC
CommandRet ProcessAxisCommand(__IO CommandStruct* command, bool fromStack)
{
	switch(command->type)
	{
		case COM_AXIS_LINE:
			return ProcessAxisLine(&command->line);
			break;
		case COM_AXIS_SINE:
			return ProcessAxisSine(&command->sine);
		case COM_AXIS_CIRCLE:
			return ProcessAxisCircle(&command->circle);
	}
	return COMMAND_PROC_OK;
}

CommandRet ProcessAxisLine(__IO AxisLine* line)//Processes one line of movement
{
	//Prepare line
	float direction[3];
	//Prepare direction
	int i;
	for(i = 0; i!= 3; i++)
	{
		direction[i]= line->axes[i] - axesBuffer[i];
	}
	float length = Abs3DVector(direction);
	Normalize3DVector(direction);

	//Prepare acceleration
	float A[3], Ab[3];
	Multiply3DVectorTo(direction, line->As, A);
	Multiply3DVectorTo(direction, line->Ab, Ab);

	//Check, if the line is long enough to accelerate
	float T, Tb;
	if(line->As == 0)
		T = 0;
	else
		T = M_PI*(line->v - line->v0)/(2*line->As);
	if(line->Ab == 0)
		Tb = 0;
	else
		Tb = M_PI*(line->v - line->vb)/(2*line->Ab);
	float S = line->As*T*T/M_PI+line->v0*T;
	float Sb = -line->Ab*Tb*Tb/M_PI+line->v*Tb;
	if(S + Sb > length)//Is it possible to accelerate?
	{
		line->v = sqrt((4*axisData.maxLinearAcceleration*length+M_PI*(line->v0*line->v0+line->vb*line->vb))/(2*M_PI));
		//Compute new values for new speed
		T = M_PI*(line->v - line->v0)/(2*axisData.maxLinearAcceleration);
		Tb = M_PI*(line->v - line->vb)/(2*axisData.maxLinearAcceleration);
		S = axisData.maxLinearAcceleration*T*T/M_PI+line->v0*T;
		Sb = -axisData.maxLinearAcceleration*Tb*Tb/M_PI+line->v*Tb;
	}

	//Prepare values
	float ATpi[3];
	Multiply3DVectorTo(A, T/(M_PI*M_PI), ATpi);//acceleration
	for(i = 0; i != 3; i++)
		axisMovementBuffer.data[i].line.ATpi = ATpi[i];

	Multiply3DVectorTo(Ab, -Tb/(M_PI*M_PI), ATpi);
	for(i = 0; i != 3; i++)
		axisMovementBuffer.data[i].line.ATpiB = ATpi[i];

	float lin[3];
	Multiply3DVectorTo(direction, S, lin);
	for(i = 0; i != 3; i++)
		axisMovementBuffer.data[i].line.constant = axesBuffer[i] + lin[i];

	float brake[3];
	Multiply3DVectorTo(direction, Sb, brake);
	for(i = 0; i != 3; i++)
		axisMovementBuffer.data[i].line.brake = line->axes[i] - brake[i];

	float v[3];
	Multiply3DVectorTo(direction, line->v, v);
	for(i = 0; i != 3; i++)
	{
		axisMovementBuffer.data[i].line.v = v[i];
	}

	Multiply3DVectorTo(direction, line->v0, v);
	for(i = 0; i != 3; i++)
		axisMovementBuffer.data[i].line.v0 = v[i];


	axisMovementBuffer.state = ACCELERATING;
	axisMovementBuffer.cLine.T = T;
	axisMovementBuffer.cLine.TB = Tb;
	axisMovementBuffer.time = 0;
	axisMovementBuffer.type = AXIS_MOVE_LINEAR;
	if(line->v != 0)
		axisMovementBuffer.cLine.t = (length - S - Sb)/line->v;
	else
		axisMovementBuffer.cLine.t = 0;
	for(i = 0; i != 3; i++)
	{
		axisMovementBuffer.done = false;
		axisMovementBuffer.data[i].line.start = axesBuffer[i];
		axisMovementBuffer.data[i].line.end = line->axes[i];
		axisMovementBuffer.data[i].line.position = &axesBuffer[i];

	}

	//Process movement
	axisData.IsRunning = true;//Start the movement, wait until finished
	while(axisData.IsRunning)
	{
		if(!ContinuePerforming() || !IsGood())//Should we continue?
		{
			axisData.IsRunning = false;
			return COMMAND_PROC_INTERRUPT;//Processing was interrupted
		}
	}

	return COMMAND_PROC_OK;
}


CommandRet ProcessAxisSine(__IO AxisSine* sine)
{
	//Get the direction of acceleration
	float S[3];//Starting point
	uint8_t i = 0;
	for(i = 0; i != 3; i++)
	{
		S[i] = axesBuffer[i];
	}

	float Vx[3];//Velocity in normal way
	float D[3];//Direction point
	Subtract3DVectorTo(sine->B, S, D);
	float length = Abs3DVector(D);
	Normalize3DVector(D);
	AssignVector(D, Vx);//For determining velocity
	Multiply3DVector(D, length/2.0);
	Add3DVectorTo(S, D, D);

	float A[3];
	Subtract3DVectorTo(D, sine->C, A);
	Normalize3DVector(A);

	//Get the velocity in the given direction
	float V[3];
	Subtract3DVectorTo(sine->C, S, V);
	Normalize3DVector(V);
	Multiply3DVector(V, sine->v);
	float vx, vy, temp;
	if((temp = -A[0]*Vx[1]+Vx[0]*A[1]) != 0)
	{
		vx = (-A[0]*V[1]+V[0]*A[1])/temp;
		vy = (V[0]*Vx[1]-Vx[0]*V[1])/temp;
	} else if((temp = -A[2]*Vx[1]+Vx[2]*A[1]) != 0)
	{
		vx = (-A[2]*V[1]+V[2]*A[1])/temp;
		vy = (V[2]*Vx[1]-Vx[2]*V[1])/temp;
	}
	else if((temp = -A[2]*Vx[0]+Vx[2]*A[0]) != 0)
	{
		vx = (-A[2]*V[0]+V[2]*A[0])/temp;
		vy = (V[2]*Vx[0]-Vx[2]*V[0])/temp;
	}
	else
	{
		//This shouldn't occur
		vx = vy = 0;
	}
	//Determine A
	float Aabs = M_PI*vy*vx/length;
	Multiply3DVector(A, Aabs);
	float T = M_PI*vy/Aabs;

	axisMovementBuffer.done = false;
	axisMovementBuffer.time = 0;
	axisMovementBuffer.type = AXIS_MOVE_SINE;
	axisMovementBuffer.cSine.T = T;
	for(i=0; i != 3; i++)
	{

		axisMovementBuffer.data[i].sine.position = &axesBuffer[i];
		axisMovementBuffer.data[i].sine.ATpi = A[i]*T/(M_PI*M_PI);
		axisMovementBuffer.data[i].sine.start = S[i];
		axisMovementBuffer.data[i].sine.v = V[i];
		axisMovementBuffer.data[i].sine.end = sine->B[i];
	}


	//Process movement
	axisData.IsRunning = true;//Start the movement, wait until finished
	while(axisData.IsRunning)
	{
		if(!ContinuePerforming() || !IsGood())//Should we continue?
		{
			axisData.IsRunning = false;
			return COMMAND_PROC_INTERRUPT;//Processing was interrupted
		}
	}

	return COMMAND_PROC_OK;
}

CommandRet ProcessAxisCircle(__IO AxisCircle* circle)//Processes one line of movement
{
	//Get the center S
	memset(&axisMovementBuffer, 0, sizeof(axisMovementBuffer));
	//For simplifying of the code
	//float* A = axesBuffer;
	float A[3];
	A[0] = axesBuffer[0];
	A[1] = axesBuffer[1];
	A[2] = axesBuffer[2];
	float* B = circle->B;
	float* C = circle->C;

	//Find the center
	float X[3], Y[3];
	Add3DVectorTo(B, A, X);
	Divide3DVector(X, 2);
	Add3DVectorTo(B, C, Y);
	Divide3DVector(Y, 2);
	float CB[3], AB[3];
	Subtract3DVectorTo(C, B, CB);
	Subtract3DVectorTo(A, B, AB);
	float n[3];
	CrossProduct3DVector(CB, AB, n);
	Normalize3DVector(n);
	float u[3], v[3];
	CrossProduct3DVector(AB, n, u);
	CrossProduct3DVector(CB, n, v);
	Normalize3DVector(u);
	Normalize3DVector(v);
	float temp, t;
	if((temp = u[0]*v[1]-v[0]*u[1]) != 0)
	{
		t = (u[1]*(X[0]-Y[0])-u[0]*(X[1]-Y[1]))/temp;
	}
	else if((temp = u[2]*v[1]-v[2]*u[1]) != 0)
	{
		t = (u[1]*(X[2]-Y[2])-u[2]*(X[1]-Y[1]))/temp;
	}
	else if((temp = u[2]*v[0]-v[2]*u[0]) != 0)
	{
		t = (u[0]*(X[2]-Y[2])-u[2]*(X[0]-Y[0]))/temp;
	}
	else
	{
		//This shouldn't occur
	}
	float S[3];
	Multiply3DVectorTo(u, t, S);
	Add3DVectorTo(S, X, S);
	S[0] = circle->S[0];
	S[1] = circle->S[1];
	S[2] = circle->S[2];

	//Get radius
	float AS[3];
	Subtract3DVectorTo(A, S, AS);
	float r = Abs3DVector(AS);
	Normalize3DVector(AS);

	//Get the length of arc
	Normalize3DVector(CB);
	Normalize3DVector(AB);
	float alpha = 2*M_PI - 2*acos(DotProduct3DVector(CB, AB));
	if(alpha != alpha)
		alpha = 0;
	alpha = circle->alpha;

	float w, w0, wb;
	w = circle->v/r;
	w0 = circle->v0/r;
	wb = circle->vb/r;
	float As, Ab;
	As = circle->As/r;
	Ab = circle->Ab/r;


	//Check, if the circle is long enough to accelerate
	float T = fabs(M_PI*(w - w0)/(2*As));
	float Tb = fabs(M_PI*(w - wb)/(2*Ab));
	float Sa = As*T*T/M_PI+w0*T;
	float Sb = -Ab*Tb*Tb/M_PI+w*Tb;
	if(Sa + Sb > alpha)//Is it possible to accelerate?
	{
		w = sqrt((4*axisData.maxLinearAcceleration/r*alpha+M_PI*(w0*w0+wb*wb))/(2*M_PI));
		//Compute new values for new speed
		T = fabs(M_PI*(w - w0)/(2*axisData.maxLinearAcceleration/r));
		Tb = fabs(M_PI*(w - wb)/(2*axisData.maxLinearAcceleration/r));
		Sa = axisData.maxLinearAcceleration/r*T*T/M_PI+w0*T;
		Sb = -axisData.maxLinearAcceleration/r*Tb*Tb/M_PI+w*Tb;
	}


	float crossProduct[3];
	CrossProduct3DVector(n, AS, crossProduct);

	axisMovementBuffer.type = AXIS_MOVE_CIRCLE;
	axisMovementBuffer.done = false;
	axisMovementBuffer.state = ACCELERATING;
	axisMovementBuffer.time = 0;
	axisMovementBuffer.cCircle.ATpi = As*T/(M_PI*M_PI);
	axisMovementBuffer.cCircle.ATpiB = -Ab*Tb/(M_PI*M_PI);
	axisMovementBuffer.cCircle.T = T;
	axisMovementBuffer.cCircle.TB = Tb;
	axisMovementBuffer.cCircle.alpha = 0;
	axisMovementBuffer.cCircle.brake = alpha - Sb;
	axisMovementBuffer.cCircle.constant = Sa;
	axisMovementBuffer.cCircle.end = alpha;
	axisMovementBuffer.cCircle.w = w;
	axisMovementBuffer.cCircle.w0 = w0;
	uint8_t i;
	for(i = 0; i < 3; i++)
	{
		axisMovementBuffer.data[i].circle.position = &axesBuffer[i];
		axisMovementBuffer.data[i].circle.R = r;
		axisMovementBuffer.data[i].circle.center = S[i];
		axisMovementBuffer.data[i].circle.crossProduct = crossProduct[i];
		axisMovementBuffer.data[i].circle.u = AS[i];
		axisMovementBuffer.data[i].circle.final = C[i];
	}

	//Process movement
	axisData.IsRunning = true;//Start the movement, wait until finished
	while(axisData.IsRunning)
	{
		if(!ContinuePerforming() || !IsGood())//Should we continue?
		{
			axisData.IsRunning = false;
			return COMMAND_PROC_INTERRUPT;//Processing was interrupted
		}
	}

	return COMMAND_PROC_OK;
}

bool UpdateAxisPosition()//Updates position of single axis, return true
	//if control should be notified
{
	switch(axisMovementBuffer.type)
	{
		case AXIS_MOVE_LINEAR:
			return UpdateLinearAxesPosition();
			break;
		case AXIS_MOVE_CIRCLE:
			return UpdateCircleAxesPosition();
			break;
		case AXIS_MOVE_SINE:
			return UpdateSineAxesPosition();
			break;
	}
	return true;
}

void UpdateLinearAxisPosition(uint8_t i)//Updates linear motion for single axis
{
	float time = axisMovementBuffer.timeF;
	AxisMovementBuffer* buff = &axisMovementBuffer;
	if(STM32F4_Discovery_PBGetState(BUTTON_USER))
	{
		__asm("nop");
	}
	else if(buff->state == BRAKING)
	{
		//Braking in progress
		if(buff->cLine.TB != 0)
			*(buff->data[i].line.position) = buff->data[i].line.brake + buff->data[i].line.ATpiB*(M_PI*time - buff->cLine.TB*myFastSin(M_PI*time/buff->cLine.TB))+buff->data[i].line.v*time;
	}
	else if(buff->state == CONSTANT)
	{
		//Constant velocity
		*(buff->data[i].line.position) = buff->data[i].line.constant + buff->data[i].line.v*time;
	}
	else
	{
		//Accelerating
		if(buff->cLine.T != 0)
			*(buff->data[i].line.position) = buff->data[i].line.start + buff->data[i].line.ATpi*(M_PI*time - buff->cLine.T*myFastSin(M_PI*time/buff->cLine.T))+buff->data[i].line.v0*time;

	}
}

void UpdateCircleAxisPosition(uint8_t i)//Updates circle motion for single axis
{
	AxisMovementBufferCircle* buff = &axisMovementBuffer.data[i].circle;
	float alpha = axisMovementBuffer.cCircle.alpha;
	*(buff->position) = buff->R*(myFastCos(alpha)*buff->u+myFastSin(alpha)*buff->crossProduct) + buff->center;
	return;
}

void UpdateSineAxisPosition(uint8_t i)//Updates sine motion for single axis
{
	AxisMovementBuffer* buff = &axisMovementBuffer;
	float time = buff->timeF;
	*(buff->data[i].sine.position) = buff->data[i].sine.start + buff->data[i].sine.ATpi*(M_PI*time - buff->cSine.T*myFastSin(M_PI*time/buff->cSine.T))+buff->data[i].sine.v*time;
	return;
}

bool HasPassed(float current, float target, bool direction)//Tells whether targeted segment has already passed
{
	if(direction)
		return (current >= target);
	else
		return (current <= target);
}

bool UpdateLinearAxesPosition()//Updates linear motion for all axes
{
	axisMovementBuffer.time += AXIS_UPDATE_T;
	axisMovementBuffer.timeF = axisMovementBuffer.time/(float)(AXIS_UPDATE_UNIT);
	uint8_t i;
	for(i = 0; i != 3; i++)
	{
		UpdateLinearAxisPosition(i);
		NotifyMovement(i);
	}
	if(axisMovementBuffer.timeF >= axisMovementBuffer.cLine.TB && axisMovementBuffer.state == BRAKING)
	{
		for(i = 0; i != 3; i++)
			*(axisMovementBuffer.data[i].line.position) = axisMovementBuffer.data[i].line.end;
		return true;
	}
	if(axisMovementBuffer.timeF >= axisMovementBuffer.cLine.t && axisMovementBuffer.state == CONSTANT)
	{
		axisMovementBuffer.time = 0;
		axisMovementBuffer.state = BRAKING;
	}
	if(axisMovementBuffer.timeF >= axisMovementBuffer.cLine.T && axisMovementBuffer.state == ACCELERATING)
	{
		axisMovementBuffer.time = 0;
		axisMovementBuffer.state = CONSTANT;
	}
	return false;
}

bool UpdateCircleAxesPosition()//Updates circle motion for all axes
{
	//Update alpha
	AxisMovementBuffer* buff = &axisMovementBuffer;
	float time = buff->time/(float)(AXIS_UPDATE_UNIT);
	buff->time += AXIS_UPDATE_T;//ToDo: Think about the time update placement

	if(buff->cCircle.alpha >= buff->cCircle.end)
	{
		//We've already finished the movement
		buff->cCircle.alpha = buff->cCircle.end;
		buff->done = true;
	}
	else if(buff->state == BRAKING)
	{
		//Braking in progress
		/*STM32F4_Discovery_LEDOff(LED4);
		STM32F4_Discovery_LEDOff(LED6);
		STM32F4_Discovery_LEDOn(LED5);*/
		if(buff->cCircle.TB != 0)
			buff->cCircle.alpha = buff->cCircle.brake + buff->cCircle.ATpiB*(M_PI*time - buff->cCircle.TB*myFastSin(M_PI*time/buff->cCircle.TB))+buff->cCircle.w*time;
	}
	else if(buff->state == CONSTANT)
	{
		//Constant velocity
		/*STM32F4_Discovery_LEDOff(LED4);
		STM32F4_Discovery_LEDOn(LED6);
		STM32F4_Discovery_LEDOff(LED5);*/
		buff->cCircle.alpha = buff->cCircle.constant + buff->cCircle.w*time;
		if(buff->cCircle.alpha >= buff->cCircle.brake && buff->cCircle.TB != 0)
		{
			buff->time = 0;
			buff->state = BRAKING;
		}
	}
	else
	{
		//Accelerating
		/*STM32F4_Discovery_LEDOn(LED4);
		STM32F4_Discovery_LEDOff(LED6);
		STM32F4_Discovery_LEDOff(LED5);*/
		if(buff->cCircle.T != 0)
			buff->cCircle.alpha = buff->cCircle.ATpi*(M_PI*time - buff->cCircle.T*myFastSin(M_PI*time/buff->cCircle.T))+buff->cCircle.w0*time;
		if(buff->cCircle.alpha >= buff->cCircle.constant)
		{
			buff->time = 0;
			buff->state = CONSTANT;
		}
	}



	uint8_t i;
	for(i = 0; i != 3; i++)
	{
		UpdateCircleAxisPosition(i);
		NotifyMovement(i);
	}
	return buff->done;
}

bool UpdateSineAxesPosition()//Updates sine motion for all axes
{
	axisMovementBuffer.time += AXIS_UPDATE_T;
	axisMovementBuffer.timeF = axisMovementBuffer.time/(float)(AXIS_UPDATE_UNIT);
	uint8_t i;
	for(i = 0; i != 3; i++)
	{
		UpdateSineAxisPosition(i);
		NotifyMovement(i);
	}
	if(axisMovementBuffer.timeF >= axisMovementBuffer.cSine.T)
	{
		//Finished
		uint8_t i;
		for(i = 0; i != 3; i++)
			*(axisMovementBuffer.data[i].sine.position) = axisMovementBuffer.data[i].sine.end;
		return true;
	}
	return false;
}
