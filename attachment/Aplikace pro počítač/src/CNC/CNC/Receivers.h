#ifndef RECEIVERS
#define RECEIVERS

//Receivers on first level
//Data are adressed to error state component
#define RECEIVER_ERRSTATE	1
//Data are adressed to state component
#define RECEIVER_STATE		2
//Data are adressed to CommandStack
#define RECEIVER_COMMANDSTACK	3
//Data are adressed to configComponent
#define RECEIVER_CONFIG		4



//Receivers on second level - commands
//"Global commands" - f.e: wait
#define COM_RECEIVER_GLOBAL	1
//GPIO component
#define COM_RECEIVER_GPIO	2
//Axis - commands for movement
#define COM_RECEIVER_AXIS	3

#endif