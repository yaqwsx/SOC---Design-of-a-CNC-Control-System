#pragma once
#include <string>
#include "MyDevice.h"
#include <string>

using namespace std;

//This class handles sending stepper setting to the interpolator
class StepperSettings
{
	public:
		StepperSettings(void);
		~StepperSettings(void);
		void SendConfig(MyDevice* d);
	private:
		int index;
		unsigned char GetGPIOEnum(string e);
};

