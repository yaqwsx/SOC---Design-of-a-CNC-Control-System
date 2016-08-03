#ifndef SWITCHCOMPONENT_H_
#define SWITCHCOMPONENT_H_

#include "global.h"

typedef enum{SWITCH_INACTIVE = 0, SWITCH_LIMIT, SWITCH_PRELIMIT, SWITCH_HOME} SwitchState;

#define SWITCH_XP	(1<<0)
#define SWITCH_XN	(1<<1)
#define SWITCH_YP	(1<<2)
#define SWITCH_YN	(1<<3)
#define SWITCH_ZP	(1<<4)
#define SWITCH_ZN	(1<<5)


typedef struct
{
		SwitchState state;
		uint8_t axes;//Defines for which axes is the switch valid
		GPIO_TypeDef* GPIO;
		uint16_t directionPin;
		bool active;
}	Switch;

typedef struct
{
		float homingSpeed;
		Switch limitSwitches[10];
		Switch preLimitSwitches[10];
		Switch homingSwitches[10];
}	SwitchData;




#endif /* SWITCHCOMPONENT_H_ */
