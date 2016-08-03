#include "Settings.h"


Settings::Settings(void)
{
}


Settings::~Settings(void)
{
}

void Settings::SetDependencies(MyDevice* d) {device = d;};

void Settings::SendSettings()
{
	AxisSettings a;
	a.SendConfig(device);
	StepperSettings s;
	s.SendConfig(device);
}