#pragma once
#include "MyData.h"
#include "MainWindow.h"
#include "global.h"
#include "ProgramRun.h"
#include "MyDevice.h"

class MySerial;
class MyData;
class MainWindow;

class ProgramRun;

//Provides interface for program control (its thread)
class ProgramControl
{
	public:
		ProgramControl(void);
		~ProgramControl(void);
		void SetDependences(MyData* d, MainWindow* w, MyDevice* dev){data = d; window = w; device = dev;};
		bool Start();//Starts the program
		bool Pause();//Pauses the program
		bool Continue();//Continues the program
		bool Stop();//Stops the program
		enum stateOfProgram {RUN = 1, STOP = 3, PAUSE = 4};
	private:
		MyData* data;
		MyDevice* device;
		MainWindow* window;

		
};

