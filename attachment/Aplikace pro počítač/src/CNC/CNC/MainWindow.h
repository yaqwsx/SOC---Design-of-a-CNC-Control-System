#pragma once
#include "global.h"
#include <wx/splitter.h>
#include <wx/toolbar.h>
#include "MyData.h"
#include "ProgramControl.h"
#include "MyDevice.h"
#include "PreviewWindow.h"

class MyDevice;
class MyData;
class MySerial;
class ProgramControl;

//MainWindow represents the main window of the application
class MainWindow :
	public wxFrame
{
	public:
		MainWindow(void);
		~MainWindow(void);
		void SetDependences(MyData* dat, ProgramControl* p, MyDevice* d);
		void OnOpen(wxCommandEvent& event);
		void OnOpenGcode(wxCommandEvent& event);
		void OnConnect();
		void OnDisconnect();
		void OnRunProgram(wxCommandEvent& event);
		void OnPauseProgram(wxCommandEvent& event);
		void OnStopProgram(wxCommandEvent& event);
		void OnViewReset(wxCommandEvent& event);
		void OnTrajectoryReset(wxCommandEvent& event);
		void OnTrajectoryToggle(wxCommandEvent& event);
		void SetProgramName(wxString name);
		WXLRESULT MSWWindowProc(WXUINT, WXWPARAM, WXLPARAM);//Override default wnd proc
private:
		wxSplitterWindow* topSplit, *downSplit;
		wxPanel* leftPanel, *rightPanel;
		PreviewWindow* middlePanel;
		wxMenuBar* menu;
		wxMenu *file, *view;
		wxToolBar *toolbar;
		wxStaticText *text, *textY, *parX, *parY;
		wxStatusBar* statusBar;
		wxTextCtrl* codeView;
		MyData* data;
		ProgramControl* program;
		MyDevice* device;

		friend class MyData;
		friend class ProgramControl;
		friend class ProgramRun;
		friend class ReceiveFromDevice;
};

