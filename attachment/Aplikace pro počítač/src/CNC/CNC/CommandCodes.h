#ifndef COM_CODE
#define COM_CODE

//Defines command codes

//Global component codes
//Wait command
/*structure of data - 32bit number defines length
of waiting time in ms*/
#define COM_GLOBAL_WAIT		0


//GPIO component codes
//Turn on LED
#define COM_GPIO_LED_ON		0
//Turn off LED
#define COM_GPIO_LED_OFF	1
//Toggle LED
#define COM_GPIO_LED_TOGGLE	2

//Axis components
#define COM_AXIS_LINE	0/*Line with possible rotation movement*/
#define COM_AXIS_CIRCLE	1	/*Circle with possible rotation movement*/
#define COM_AXIS_SINE	2	/*Sine with possible rotation movement*/


#endif