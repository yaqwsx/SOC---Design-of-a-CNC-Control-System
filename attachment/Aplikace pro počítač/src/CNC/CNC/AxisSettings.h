#pragma once
#include <string>
#include "MyDevice.h"

using namespace std;

class MyDevice;

//This class handles sending axis setting to the interpolator
class AxisSettings
{
	public:
		AxisSettings(void);
		~AxisSettings(void);
		void SendConfig(MyDevice* d);
	private:
};

