#include "global.h"

int StateOfProgram;
wxStaticText* t;
wxStaticText* tY;
wxStaticText* ppX;
wxStaticText* ppY;
void* wnd;
string iniFile="settings.ini";
bool init = false;
CommandIDCounter IDcounter;

void PostErrorMessage(const wxChar* t)
{
	wxString text;
	text << t << wxT("\nVýpis: ") << wxSysErrorMsg(wxSysErrorCode());
	wxMessageBox(text, wxT("Chyba!"), wxOK | wxICON_EXCLAMATION);
	//MessageBox(NULL, wxSysErrorMsg(wxSysErrorCode()), t, NULL);
}

void PostErrorMessageC(wxString t)
{
	wxString text;
	text << t << wxT("\nVýpis: ") << wxSysErrorMsg(wxSysErrorCode());
	wxMessageBox(text, wxT("Chyba!"), wxOK | wxICON_EXCLAMATION);
}


float StringToFloat(const std::string s)
{
	std::istringstream ss(s);
	float t;
	ss >> t;
	return t;
}

float StringToInt(const std::string s)
{
	std::istringstream ss(s);
	int t;
	ss >> t;
	return t;
}


