#include "StepperSettings.h"
#include "iniFile.h"
#include "CommunicationEnumerations.h"

StepperSettings::StepperSettings(void)
{
}


StepperSettings::~StepperSettings(void)
{
}

void StepperSettings::SendConfig(MyDevice* d)
{
	CIniFile ini;
	int num = StringToInt(ini.GetValue("numOfSteppers", "steppers", iniFile));
	for(size_t i = 0; i != num; i++)
	{
		//Set type
		unsigned char buffer[64] = {0};
		buffer[0] = 1;//One packet
		buffer[1] = RECEIVER_CONFIG;
		buffer[2] = CONFIG_REC_MOVEMENT;
		buffer[3] = CONFIG_MOVEMENT_SETTYPE;
		buffer[4] = i;
		buffer[5] = 1;//Stepper
		d->SendRawData(buffer, sizeof(buffer));
		//Send configuration package
		string section = "stepper";
		section = section + i;
		buffer[3] = CONFIG_MOVEMENT_INDIVIDUAL;
		buffer[4] = i;
		buffer[5] = CONFIG_MOVEMENT_ST_PACKAGE;
		buffer[6] = StringToInt(ini.GetValue("pulsePin", section, iniFile));
		buffer[7] = GetGPIOEnum(ini.GetValue("dirPinGPIO", section, iniFile));
		buffer[8] = StringToInt(ini.GetValue("dirPin", section, iniFile));
		if(ini.GetValue("inverted", section, iniFile) == "true")
			buffer[9] = 1;
		else
			buffer[9] = 0;
		uint32_t t = StringToInt(ini.GetValue("mmPerStep", section, iniFile));
		*((uint32_t*)(buffer + 10)) = StringToInt(ini.GetValue("mmPerStep", section, iniFile));

		d->SendRawData(buffer, sizeof(buffer));
	}
}

unsigned char StepperSettings::GetGPIOEnum(string e)
{
	if(e == "GPIOA")
		return 0;
	else if(e == "GPIOB")
		return 1;
	else if(e == "GPIOC")
		return 2;
	else if(e == "GPIOD")
		return 3;
	else if(e == "GPIOE")
		return 4;
	else if(e == "GPIOF")
		return 5;
	else if(e == "GPIOG")
		return 6;
	else if(e == "GPIOH")
		return 7;
	else if(e == "GPIOI")
		return 8;
	return 255;
}