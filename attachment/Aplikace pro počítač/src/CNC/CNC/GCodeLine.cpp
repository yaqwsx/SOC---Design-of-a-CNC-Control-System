#include "GCodeLine.h"


float GCodeLine::GetParam(char param)
{
	if(ParamExists(toupper(param)))
		return data[toupper(param)];
	return numeric_limits<float>::signaling_NaN();
}

bool GCodeLine::ParamExists(char param)
{
	if(data.find(param) != data.end())
		return true;
	return false;
}
