#pragma once
#include "pathpartline.h"

//Rapid line, children of classic line
class PathPartRapid :
	public PathPartLine
{
public:
	//Constructors
	PathPartRapid(Point begining, Point end, double feed, WorkingPlane p, PathOffsets o, size_t line);
	~PathPartRapid(void);

	PathPartType GetType(){return LINE_RAPID;};
	PathPartSimple ToSimple();
	PathPartSimple ToOffsetSimple();
};

