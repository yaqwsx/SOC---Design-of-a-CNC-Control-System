#ifndef MOVEMENT_H_
#define MOVEMENT_H_

/*
 * File containing structures for physical movement of axes*/

#include "stepper.h"
#include "servo.h"

typedef enum {STEPPER = 1, SERVO = 2, UNUSED = 0} MovementType;

typedef struct
{
	MovementType type;
	union
	{
		Stepper stepper;
		Servo servo;
	};
} MovementStruct;

extern __IO MovementStruct movement[5];


//Function declaration

void NotifyMovement(uint8_t index);
void InitMovement();
bool IsMovementConfigured();
void MovementConfig(uint8_t* data);



#endif /* MOVEMENT_H_ */
