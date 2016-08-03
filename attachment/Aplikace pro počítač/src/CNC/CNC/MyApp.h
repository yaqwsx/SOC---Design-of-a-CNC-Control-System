#pragma once
#include "global.h"
#include "MainWindow.h"
#include "MyData.h"
#include "ProgramControl.h"
#include "MyDevice.h"

//Class representing the application
class MyApp :
	public wxApp
{
	public:
		MyApp(void);
		~MyApp(void);
		bool OnInit();
		int OnExit();
	private:
		MainWindow* window;
		MyData data;
		ProgramControl program;
		MyDevice	device;
};

