#pragma once
#include "MainWindow.h"
#include <vector>
#include <wx/wx.h>
#include <fstream>
#include "Settings.h"
#include "MyDevice.h"
#include "D:\Users\Honza\Atollic\TrueSTUDIO\STM32_workspace\CNC\src\CNC\CommandStruct\commandStruct.h"
#include "ProcessedData.h"


class MainWindow;
class MyDevice;
class Settings;
class PreviewWindow;



const char fileID[] = "abcde";

//Class containing the data for application
class MyData
{
	public:
		MyData(void);
		~MyData(void);
		void SetDependences(MainWindow* w, MyDevice* d);
		void OpenFile();
		void OpenGcode();
		void ProcessFile();
		bool ProcessData();
	private:
		MainWindow* window;//Pointer to window to get the acces for GUI
		//RawPath data for interpreter = with acceleration
		vector<ProcessedData> processedData;
		//Unprocessed lines - w/o acceleration or modified speed
		vector<string> unprocessedData;
		wxString file;
		//Processing the program - states
		char state;
		//Settings for the interpreter
		Settings settings;

		
		friend class MainWindow;
		friend class ProgramRun;
		friend class ProgramControl;
		friend class PreviewWindow;
};


