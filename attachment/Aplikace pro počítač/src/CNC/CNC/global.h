#pragma once
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
	//#include <vld.h>
#endif
#include <wx/wx.h>
#include "wx/log.h"
#include <Windows.h> 
#include <string>
#include <sstream>
#include "CommandIDCounter.h"



enum EventIDs{ID_OPEN_GCODE=6500, ID_RESETVIEW, ID_RESETTRAJECTORY,
	ID_TOGGLETRAJECTORY};

using namespace std;

extern wxStaticText* t;
extern wxStaticText* tY;
extern wxStaticText* ppX;
extern wxStaticText* ppY;
extern int StateOfProgram;
extern string iniFile;
extern void* wnd;
extern bool init;


class CommnadIDCounter;
extern CommandIDCounter IDcounter;

//GUID of device interface
static const GUID DEVICE_INTERFACE = 
{ 0x79923308, 0x94e3, 0x42b0, { 0xa0, 0x60, 0x72, 0x09, 0xe9, 0x45, 0x0d, 0x03 } };

void PostErrorMessage(const wxChar* t);
void PostErrorMessageC(wxString s);


float StringToFloat(const std::string s);
float StringToInt(const std::string s);


template <class Whatever>
string operator+(const string & a, const Whatever & b) {
  stringstream ss; ss << a << b; return ss.str();
}
template <class Whatever>
string & operator+=(string & a, const Whatever & b) {
  stringstream ss; ss << a << b; a = ss.str(); return a;
} 

#include <stdint.h>
