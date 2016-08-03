#ifndef COMMANDSTACK_CODE
#define COMMANDSTACK_CODE


//codes for command stack


//Code for returning free space in stack
#define COMSTACK_RET_SPACE	1
//Code for adding data into queue
#define	COMSTACK_ADD_COM	2
//Code for returning last command's in stack ID
#define COMSTACK_LAST_COM_ID	3
//Code for clearing the stack
#define COMSTACK_CLEAR		4
//Code for starting performing
#define COMSTACK_START		5
//Code por pausing performing
#define COMSTACK_PAUSE		6
//PC asks for last processed item
#define COMSTACK_LAST_ITEM_PROC		7
//PC resets last processed item
#define COMSTACK_RESET_LAST_ITEM_PROC	8
//PC asks for ID of actually processing command
#define COMSTACK_CURRENT_ITEM_PROC		9


#endif