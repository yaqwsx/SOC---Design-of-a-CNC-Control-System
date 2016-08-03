#ifndef STATE_MESSAGE
#define STATE_MESSAGE

//PC asks for error state of device, return ERRSTATEINFO_MESS
#define ERRSTATEMESSAGE_RETSTATE	1
//PC asks for performed commands, return 
#define STATEMESSAGE_RETSTATE		2
//PC asks for actual position
#define STATEMESSAGE_POSITION		3
//PC asks for last processed item
#define STATEMESSAGE_LAST_ITEM		4
//PC resets last processed item ID
#define STATEMESSAGE_RESET_LAST_ITEM	5

#endif