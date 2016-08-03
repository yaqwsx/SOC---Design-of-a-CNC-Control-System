#pragma once
#include "wx/wx.h"
#include "MyData.h"
#include "ProgramControl.h"
#include "MainWindow.h"
#include "MyDevice.h"
#include <list>

class MyData;
class MySerial;
class ProgramControl;

//Thread for sending commands to the interpolator
class ProgramRun :
	public wxThread
{
	public:
		ProgramRun(void);
		~ProgramRun(void);
		ExitCode Entry();//Entry point of program
		ExitCode Clean();//Cleans before return
		void SetDependencies(MyDevice* d, ProgramControl* p, MyData* da){device = d; program = p; data = da;};
		void HighlightActiveLine(uint32_t ID);
	private:
		MyDevice* device;
		ProgramControl* program;
		MyData* data;
		uint8_t lastState;
};

