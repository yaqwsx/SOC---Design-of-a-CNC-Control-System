#include "PathPartRapid.h"


PathPartRapid::PathPartRapid(Point beginning, Point end, double f, WorkingPlane p, PathOffsets o, size_t line):
	PathPartLine(beginning, end, f, p, o, line)
{
}


PathPartRapid::~PathPartRapid(void)
{
}


PathPartSimple PathPartRapid::ToSimple()
{
	PathPartSimple ret = PathPartLine::ToSimple();
	ret.type = LINE_RAPID;
	return ret;
}

PathPartSimple PathPartRapid::ToOffsetSimple()
{
	PathPartSimple ret = PathPartLine::ToOffsetSimple();
	ret.type = LINE_RAPID;
	return ret;
}