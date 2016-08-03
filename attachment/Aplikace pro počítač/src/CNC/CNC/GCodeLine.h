#pragma once
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <map>
#include <wx/wx.h>

using namespace std;

typedef pair<char, float> GFunction;


//Structure for G-code processing
struct GCodeLine
{
		float GetParam(char param);
		bool ParamExists(char param);
		GFunction function;
		map<char, float> data;
		size_t line;
};
