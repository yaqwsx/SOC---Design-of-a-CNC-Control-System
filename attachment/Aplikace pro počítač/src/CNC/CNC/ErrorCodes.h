#ifndef ERR_CODES
#define ERR_CODES

//___________List of error codes_______________
//Too many data to recieve, fatal error, device needs to reset
#define ERR_TOO_MANY_DATA	1
//Too many errors has occured, error buffer overflow, device resets
#define ERR_TOO_MANY_ERR	2
//To many stimulantously performed commands
#define ERR_TOO_MANY_COM	3
//Command stack overflow
#define ERR_COMSTACKOWERFLOW	4
//Cannot init sysTicks
#define ERR_SYSTICKFAIL		5
//DelayComponent buffer overflow
#define ERR_DELAYOVERFLOW	6
//Invalid configuration type for Movement
#define ERR_MOVEMENTUNUSED	7
//invalid configuration of stepper - pulse pin doesn't exists
#define ERR_STEPPER_WRONG_PULSEPIN	8


#endif
