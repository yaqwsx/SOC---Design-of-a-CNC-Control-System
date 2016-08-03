#include "MyApp.h"

#pragma comment (lib , "setupapi.lib" )
#pragma comment (lib , "winusb.lib" )


MyApp::MyApp(void)
{
}


MyApp::~MyApp(void)
{
}


bool MyApp::OnInit()
{
	_CrtSetDbgFlag (_CrtSetDbgFlag (_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	device.Init();
	window = new MainWindow;
	wnd = window;
	data.SetDependences(window, &device);
	window->Show(true);
	window->SetDependences(&data, &program, &device);
	program.SetDependences(&data, window, &device);
	init = true;
	return true;
}

int MyApp::OnExit()
{
	return 0;
}