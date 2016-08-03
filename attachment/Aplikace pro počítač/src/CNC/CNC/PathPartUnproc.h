#pragma once
#include "pathpartmodable.h"
#include <string>

//Class for unprocessed command - for temp usage only
class PathPartUnproc :
	public PathPartModable
{
public:
	PathPartUnproc(string t, size_t line);
	~PathPartUnproc(void);
	PathPartType GetType(){return UNPROC;};
	PathPartSimple ToSimple();
	bool IsCommand(){return false;}
	ProcessedData TransformToCommand(){DisabledMethod(); return ProcessedData();};
private:
	string text;
};

