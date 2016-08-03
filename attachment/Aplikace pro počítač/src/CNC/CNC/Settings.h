#pragma once

#include "AxisSettings.h"
#include "StepperSettings.h"
#include <vector>
#include "MyDevice.h"

using namespace std;


//This class provides interface for sending configuration to the interpolator
class Settings
{
	public:
		Settings(void);
		~Settings(void);
		void SendSettings();
		void SetDependencies(MyDevice* d);
	private:
		MyDevice *device;
};

