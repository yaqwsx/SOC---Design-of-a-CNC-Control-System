#include "AxisSettings.h"
#include "ConfigCodes.h"
#include "iniFile.h"
#include "CommunicationEnumerations.h"
#include "global.h"


AxisSettings::AxisSettings(void)
{
}


AxisSettings::~AxisSettings(void)
{
}

void AxisSettings::SendConfig(MyDevice* d)
{
	//Send acceleration
	CIniFile ini;
	unsigned char buffer[64] = {0};
	buffer[0] = 1;
	buffer[1] = RECEIVER_CONFIG;
	buffer[2] = CONFIG_REC_AXIS;
	buffer[3] = CONFIG_AXIS_ACCELERATION;
	*((float*)(buffer + 4)) = StringToFloat(ini.GetValue("maxLinearAcceleration", "axis", iniFile));
	d->SendRawData(buffer, sizeof(buffer));
	//Send update period
	buffer[3] = CONFIG_AXIS_UPDATE_PERIOD;
	*((uint32_t*)(buffer + 4)) = StringToInt(ini.GetValue("updatePeriod", "axis", iniFile));
	d->SendRawData(buffer, sizeof(buffer));
	//Send jerk
	buffer[3] = CONFIG_AXIS_JERK;
	*((float*)(buffer + 4)) = StringToFloat(ini.GetValue("maxLinearJerk", "axis", iniFile));
	d->SendRawData(buffer, sizeof(buffer));
}