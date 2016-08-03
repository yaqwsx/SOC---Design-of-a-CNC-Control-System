#ifndef CNC_CONF_H_
#define CNC_CONF_H_

//______________Receive configuration_______________
#define RECEIVE_BUFF	8

//_____________ErrState configuration_____________
#define ERRSTATE_BUFF	32

//_____________State configuration________________
#define STATE_BUFF 16

//_____________Command Stack configuration________¨
#define COMMANDSTACK_BUFF	64

//_____________Axis configuration
#define AXIS_TIM_FREQ	1000000/*1Mhz*/
#define AXIS_UPDATE_UNIT	1000000/*microseconds*/

//____________PulseGenerator Configuration
#define PULSE_TIM_FREQ		42000000/*6,4Mhz*/
#define MAX_TRIG_FREQ		840000/*64kHz*/

//____________Stepper configuration
#define STEPPER_PULSE_DUR	10000 /*10 us*/

#endif /* CNC_CONF_H_ */
