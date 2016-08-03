#ifndef MESS_CODE_PC
#define MESS_CODE_PC

//Definition of message codes for answer to HOST

//Error occured
#define ERROR_MESS	1
//Returns error state of machine - OK, errors, etc
#define ERRSTATEINFO_MESS	2
//Returns performed command's IDs
#define PER_COM_MESS		3
//Returns free space in Command stack
#define COMSTACK_FREE_SPACE_MESS	4
//Returns ID of last command in stack
#define COMSTACK_LAST_ID	5
//Returns position of axes
#define AXES_POSITION	6
//Return ID of last processed item from state
//component - command for direct processing
#define LAST_PROC_ITEM_STATE	7
//Return ID of last processed item from stack
#define LAST_PROC_ITEM_STACK	8
//Returns ID of currently processing item
#define CUR_PROC_ITEM_STACK		9
//Incomming data are for position log
#define DEBUG_POSITION_INFO		10


#endif