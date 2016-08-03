#include "MainWindow.h"
#include "windows.h"
#include "global.h"

#include <Dbt.h>


MainWindow::MainWindow(void):
wxFrame(NULL, wxID_ANY, wxString(wxT("CNC – øídicí systém")), wxDefaultPosition, wxSize(1000, 600), wxDEFAULT_FRAME_STYLE | wxMAXIMIZE)
{
	//Naètení ikony hlavního okna
	wxIcon icon(wxT("IDI_ICON1"), wxBITMAP_TYPE_ICO_RESOURCE);
	if(!icon.IsOk())
	{
		PostErrorMessage(wxT("Nepodaøilo ase naèíst ikonu hlavního okna"));
	}
	SetIcon(icon);
	//StatusBar
	statusBar = CreateStatusBar(4);
	statusBar->SetStatusText(wxT("Nepøipojeno k intepolátoru"), 0);
	statusBar->SetStatusText(wxT("Žádný program"), 1);
	statusBar->SetStatusText(wxT("Neznámá pozice"), 2);
	statusBar->SetStatusText(wxT("Stav neurèen"), 3);
	//Menu
	menu = new wxMenuBar;
	file = new wxMenu;
	file->Append(wxID_OPEN, wxT("Otevøít soubor s intrukcemi"), wxT("Otevøít soubor s programem"), false);
	file->Append(ID_OPEN_GCODE, wxT("Otevøít G-kód"), wxT("Otevøít soubor s programem"), false);
	file->Append(wxID_EXIT, wxT("&Konec"));
	menu->Append(file, wxT("&Soubor"));
	view = new wxMenu;
	view->Append(ID_RESETVIEW, wxT("Resetovat zobrazení drah"), wxT("Obnoví zobrazení"), false);
	view->Append(ID_RESETTRAJECTORY, wxT("Smazat stopu"), wxT("Smaže stopu nástroje"), false);
	view->Append(ID_TOGGLETRAJECTORY, wxT("Vypnout/zapnout stopu"), wxT(""), false);
	menu->Append(view, wxT("&Náhled"));
	SetMenuBar(menu);
	//Connect menus and buttons to the functions
	Connect(wxID_OPEN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnOpen));
	Connect(ID_OPEN_GCODE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnOpenGcode));
	Connect(ID_RESETVIEW, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnViewReset));
	Connect(ID_RESETTRAJECTORY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnTrajectoryReset));
	Connect(ID_TOGGLETRAJECTORY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnTrajectoryToggle));
	//Toolbar
	toolbar = CreateToolBar();
	toolbar->SetToolBitmapSize(wxSize(48, 48));
	toolbar->AddSeparator();
	toolbar->AddTool(2, wxBitmap(wxT("IDB_STARTPROGRAM"), wxBITMAP_TYPE_BMP_RESOURCE), wxT("Spustit program"));
	toolbar->AddTool(3, wxBitmap(wxT("IDB_PAUSEPROGRAM"), wxBITMAP_TYPE_BMP_RESOURCE), wxT("Pozastavit program"));
	toolbar->AddTool(4, wxBitmap(wxT("IDB_STOPPROGRAM"), wxBITMAP_TYPE_BMP_RESOURCE), wxT("Zastavit program"));
	toolbar->Realize();
	toolbar->EnableTool(1, false);
	toolbar->EnableTool(3, false);
	toolbar->EnableTool(4, false);
	SetStatusBarPane(-1);
	//Connect icons to the function
	Connect(2, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindow::OnRunProgram));
	Connect(3, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindow::OnPauseProgram));
	Connect(4, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindow::OnStopProgram));

	//Splitter window
	topSplit = new wxSplitterWindow(this);
	//Content of the left
	leftPanel = new wxPanel(topSplit, 0, 0, 150, 150);
	//Content of the right
	downSplit = new wxSplitterWindow(topSplit);
	//Content of the downSplit
	middlePanel = new PreviewWindow(downSplit, 0, 0, 150, 150);
	rightPanel = new wxPanel(downSplit, 0, 0, 150, 150);
	//Split the windows
	topSplit->SplitVertically(leftPanel, downSplit, 250);
	downSplit->SplitVertically(middlePanel, rightPanel, -300);
	middlePanel->Reset();
	topSplit->SetSashGravity(0);
	downSplit->SetSashGravity(1);
	topSplit->SetMinimumPaneSize(150);
	downSplit->SetMinimumPaneSize(200);

	//Content of the right panel
	codeView = new wxTextCtrl(rightPanel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_AUTO_SCROLL | wxTE_READONLY | wxTE_RICH2 | wxHSCROLL | wxTE_NOHIDESEL);
	wxBoxSizer *sizeRight = new wxBoxSizer(wxVERTICAL);
	sizeRight->Add(codeView, 1, wxEXPAND | wxALL, 0);
	rightPanel->SetSizer(sizeRight);

	//Set the font
	wxFont codeFont(15, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL, false);
	codeView->SetDefaultStyle(wxTextAttr(wxColour((unsigned long)(0)), wxNullColour, codeFont));
	
	//Initialization of USB - receivng messages obaout device state
	HWND hwnd = HWND(GetHWND());
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = DEVICE_INTERFACE;
	if(!RegisterDeviceNotification(hwnd, (void*)(&NotificationFilter), DEVICE_NOTIFY_WINDOW_HANDLE))
	{
		wxString err;
		err << wxT("Chyba pøi incializaci notifikace pro zaøízení - chyba ")  << GetLastError();
		PostErrorMessage(err);
	}

	//Centre window
	Center();
	Maximize();
	middlePanel->Reset();
}


MainWindow::~MainWindow(void)
{
}


void MainWindow::OnOpen(wxCommandEvent& event)
{
	data->OpenFile();
}

void MainWindow::OnOpenGcode(wxCommandEvent& event)
{
	data->OpenGcode();
}

void MainWindow::OnConnect()
{
	statusBar->SetStatusText(wxT("Pøipojeno k intepolátoru"), 0);
	device->OnConnect();
	//Send Configuration
	data->settings.SendSettings();
}

void MainWindow::OnDisconnect()
{
	statusBar->SetStatusText(wxT("Nepøipojeno k intepolátoru"), 0);
	device->OnDisconnect();
	program->Stop();
}

void MainWindow::OnRunProgram(wxCommandEvent& event)
{
	program->Start();
	OnTrajectoryReset(event);
}

void MainWindow::OnPauseProgram(wxCommandEvent& event)
{
	program->Pause();
}

void MainWindow::OnStopProgram(wxCommandEvent& event)
{
	program->Stop();
}

WXLRESULT MainWindow::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
	switch(message)
	{
		case WM_DEVICECHANGE:
			switch(wParam)
			{
				case DBT_DEVICEARRIVAL:
					OnConnect();
					break;
				case DBT_DEVICEREMOVECOMPLETE:
					OnDisconnect();
					break;
				case DBT_DEVICEREMOVEPENDING:
					break;
				default:
					break;
			}
	}
	return wxFrame::MSWWindowProc (message, wParam, lParam);
}

void MainWindow::SetDependences(MyData* dat, ProgramControl* p, MyDevice* d){data = dat; program = p; device = d; if(device->TryToConnect()) OnConnect(); else OnDisconnect();};


void MainWindow::OnViewReset(wxCommandEvent& event)
{
	middlePanel->Reset();
}

void MainWindow::OnTrajectoryReset(wxCommandEvent& event)
{
	middlePanel->ClearTrajectory();
}

void MainWindow::OnTrajectoryToggle(wxCommandEvent& event)
{
	middlePanel->ToggleTrajectory();
}

void MainWindow::SetProgramName(wxString name)
{
	statusBar->SetStatusText(wxT("Otevøen program: ") + name, 1);
}