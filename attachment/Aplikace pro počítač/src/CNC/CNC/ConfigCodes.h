#ifndef CONFIG_CODE
#define CONFIG_CODE


//Configuration codes
//Receivers---------------------------------
#define CONFIG_REC_AXIS	1
#define CONFIG_REC_MOVEMENT	2



//Config codes for AXIS
#define CONFIG_AXIS_ACCELERATION	1/*Sets acceleration*/
#define CONFIG_AXIS_UPDATE_PERIOD	2/*Sets update period*/
#define CONFIG_AXIS_JERK			3/*Sets maximal jerk*/

//Config codes for MOVEMENT
#define CONFIG_MOVEMENT_SETTYPE		1/*Sets stepper/servo*/
#define CONFIG_MOVEMENT_INDIVIDUAL	2/*Additional setting, just send for further processing*/
	//Config codes for STEPPER
	#define CONFIG_MOVEMENT_ST_PACKAGE	1/*Sets the whole package for stepper*/
	#define CONFIG_MOVEMENT_ST_STEPS	2/*Sets mm per step*/
	#define CONFIG_MOVEMENT_ST_PULSE	3/*Sets pulse pin*/
	#define CONFIG_MOVEMENT_ST_DIRECTION	4/*Sets direction pin*/
	#define CONFIG_MOVEMENT_ST_INVERTED	5/*Defines whether the axis is inverted*/


#endif