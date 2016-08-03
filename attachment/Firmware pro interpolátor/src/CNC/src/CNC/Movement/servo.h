#ifndef SERVO_H_
#define SERVO_H_

#include "global.h"

typedef struct
{

} Servo;

void NotifyServo(Servo* servo, uint8_t index);
bool IsServoConfigured(Servo* servo);
void InitServo(Servo* servo);
void ConfigServo(Servo* servo, uint8_t* data);

#endif /* SERVO_H_ */
