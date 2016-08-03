#ifndef CONFIG_H_
#define CONFIG_H_
#include "global.h"

//Component that keeps state about configuration, through this component is configuration
//distributed do end-components
//It also checks if the configuration is complete an may prevent from performing commands
//in unconfigured mode. This coponent prevents from constants in program,
//Settings may be applied without recompiling program; they can even may be make in runtime

//Function definition;
void ConfigProcessMessage(uint8_t* message);//Process message from PC
void ResetConfig();//Resets config
bool IsConfigured();//Defines whether machine is ready to continue


//Config info for axis--------------------------------------------------------
extern uint32_t axisConfig;
//Definitions
#define AXIS_CONF_UPDATE		(1 << 0)/*Is the axis update interval set?*/
#define AXIS_CONF_ACCELERATION	(1 << 1)/*Is acceleration set?*/
#define AXIS_CONF_JERK			(1 << 2)/*Is jerk set?*/
#define AXIS_CONF_FULL	(AXIS_CONF_UPDATE|AXIS_CONF_ACCELERATION | AXIS_CONF_JERK)

//-----------------------------------------------------------------------------
#endif /* CONFIG_H_ */
