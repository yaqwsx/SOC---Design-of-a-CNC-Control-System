#include "GCodeInterpreter.h"
#include "PathPartUnproc.h"
#include "PathPartRapid.h"
#include "PathPartCircle.h"
#include "PathPartMark.h"

//Fills the convert table with function pointers
void GCodeInterpreter::FillGTable()
{
	processingFunctions[GFunction('G', 0.0f)] = &GCodeInterpreter::G00;
	processingFunctions[GFunction('G', 1.0f)] = &GCodeInterpreter::G01;
	processingFunctions[GFunction('G', 2.0f)] = &GCodeInterpreter::G02;
	processingFunctions[GFunction('G', 3.0f)] = &GCodeInterpreter::G03;
	processingFunctions[GFunction('G', 20.0f)] = &GCodeInterpreter::G20;
	processingFunctions[GFunction('G', 21.0f)] = &GCodeInterpreter::G21;
	processingFunctions[GFunction('G', 40.0f)] = &GCodeInterpreter::G40;
	processingFunctions[GFunction('G', 41.0f)] = &GCodeInterpreter::G41;
	processingFunctions[GFunction('G', 42.0f)] = &GCodeInterpreter::G42;
	processingFunctions[GFunction('G', 43.0f)] = &GCodeInterpreter::G43;
	processingFunctions[GFunction('G', 44.0f)] = &GCodeInterpreter::G44;
	processingFunctions[GFunction('G', 49.0f)] = &GCodeInterpreter::G49;
	processingFunctions[GFunction('F', -1.0f)] = &GCodeInterpreter::F;
	processingFunctions[GFunction('M', 2.0f)] = &GCodeInterpreter::M02;
	processingFunctions[GFunction('M', 3.0f)] = &GCodeInterpreter::M03;
	processingFunctions[GFunction('M', 4.0f)] = &GCodeInterpreter::M04;
	processingFunctions[GFunction('M', 5.0f)] = &GCodeInterpreter::M05;
}

void GCodeInterpreter::G00(GCodeLine& line)
{
	Point end;
	if(coordType == ABS)
	{
		end = Point(
			ConvertUnits(line.GetParam('X')), 
			ConvertUnits(line.GetParam('Y')),
			ConvertUnits(line.GetParam('Z')));
	}
	else
	{
		end = currentPosition + Point(
			ConvertUnits(line.GetParam('X')), 
			ConvertUnits(line.GetParam('Y')),
			ConvertUnits(line.GetParam('Z')));
	}
	unique_ptr<PathPart> part(new 
		PathPartRapid(currentPosition, end, rapidFeed/60.0, workingPlane, GetPathOffsets(), line.line));
	AddPathSegment(part);
	currentPosition = end;
}

void GCodeInterpreter::G01(GCodeLine& line)
{
	Point end;
	if(coordType == ABS)
	{
		//Absolute coordinates
		end = Point(
			ConvertUnits(line.GetParam('X')), 
			ConvertUnits(line.GetParam('Y')),
			ConvertUnits(line.GetParam('Z')));
	}
	else
	{
		end = currentPosition + Point(
			ConvertUnits(line.GetParam('X')), 
			ConvertUnits(line.GetParam('Y')),
			ConvertUnits(line.GetParam('Z')));
	}
	unique_ptr<PathPart> part(new 
		PathPartLine(currentPosition, end, standardFeed/60.0, workingPlane, GetPathOffsets(), line.line));
	AddPathSegment(part);
	currentPosition = end;
}

void GCodeInterpreter::CircleFunction(GCodeLine& line, PathPartType type)
{
	unique_ptr<PathPart> part;
	Point end;
	if(coordType == ABS)
	{
		//Absolute coordinates
		end = Point(
			ConvertUnits(line.GetParam('X')), 
			ConvertUnits(line.GetParam('Y')),
			ConvertUnits(line.GetParam('Z')));
	}
	else
	{
		end = currentPosition + Point(
			ConvertUnits(line.GetParam('X')), 
			ConvertUnits(line.GetParam('Y')),
			ConvertUnits(line.GetParam('Z')));
	}
	if(line.ParamExists('R'))
	{
		part = unique_ptr<PathPart>(new PathPartCircle(currentPosition, end,
			ConvertUnits(line.GetParam('R')), workingPlane,
			standardFeed/60, type, GetPathOffsets(), line.line));
	}
	else
	{
		Point center;
		if(arcCoordType == INC)
		{
			center = currentPosition + Point(
			ConvertUnits(line.GetParam('I')), 
			ConvertUnits(line.GetParam('J')),
			ConvertUnits(line.GetParam('K')));
		}
		else
		{
			center = Point(
			ConvertUnits(line.GetParam('I')), 
			ConvertUnits(line.GetParam('J')),
			ConvertUnits(line.GetParam('K')));
		}
		part = unique_ptr<PathPart>(new PathPartCircle(currentPosition, end, center,
			standardFeed/60, type, GetPathOffsets(), line.line));
	}
	currentPosition = end;
	AddPathSegment(part);
}

void GCodeInterpreter::G02(GCodeLine& line)
{
	CircleFunction(line, CIRCLE_CW);
}
void GCodeInterpreter::G03(GCodeLine& line)
{
	CircleFunction(line, CIRCLE_CCW);
}

//Inch units
void GCodeInterpreter::G20(GCodeLine& line)
{
	unitMultiply = 2.54f;
#ifdef _DEBUG
	unique_ptr<PathPart> part(new PathPartUnproc("Jednotky zmìneny na palce", line.line));
	AddPathSegment(part);
#endif
}

//Metric units
void GCodeInterpreter::G21(GCodeLine& line)
{
	unitMultiply = 1.0;
#ifdef _DEBUG
	unique_ptr<PathPart> part(new PathPartUnproc("Jednotky zmìnenìny na milimetry", line.line));
	AddPathSegment(part);
#endif
}

void GCodeInterpreter::F(GCodeLine& line)
{
	float speed = line.GetParam('F');
	if(speed > rapidFeed)
	{
		throw exception("Byla nastavena pøíliš vysoká rychlost posuvu");
	}
	standardFeed = speed;
#ifdef _DEBUG
	unique_ptr<PathPart> part(new PathPartUnproc(string("Posuv zmìnìn na ") + speed, line.line));
	AddPathSegment(part);
#endif
}

//End of program
void GCodeInterpreter::M02(GCodeLine& line)
{
	unique_ptr<PathPart> part(new PathPartMark(PROGRAM_END, "Konec programu", line.line));
	AddPathSegment(part);
}

//Spindle CW on
void GCodeInterpreter::M03(GCodeLine& line)
{
	unique_ptr<PathPart> part(new PathPartUnproc("Vøeteno zapnuto po smìru hodinových ruèièek", line.line));
	AddPathSegment(part);
}

//Spindle CCW on
void GCodeInterpreter::M04(GCodeLine& line)
{
	unique_ptr<PathPart> part(new PathPartUnproc("Vøeteno zapnuto proti smìru hodinových ruèièek", line.line));
	AddPathSegment(part);
}

//Spindle stop
void GCodeInterpreter::M05(GCodeLine& line)
{
	unique_ptr<PathPart> part(new PathPartUnproc("Vøeteno zastaveno", line.line));
	AddPathSegment(part);
}

//Tool radius compensation off
void GCodeInterpreter::G40(GCodeLine& line)
{
	offset.ZeroToolCompensation();
}
//Tool radius compesation left
void GCodeInterpreter::G41(GCodeLine& line)
{
	if(workingPlane != XY)
		throw exception("Volání funkce G41 pøi neaktivní pracovní rovinì XY");
	offset.SetToolComp(-line.GetParam('D'));
}
//Tool radius compensation right
void GCodeInterpreter::G42(GCodeLine& line)
{
	if(workingPlane != XY)
		throw exception("Volání funkce G42 pøi neaktivní pracovní rovinì XY");
	offset.SetToolComp(line.GetParam('D'));
}
//Tool compensation in positive direction
void GCodeInterpreter::G43(GCodeLine& line)
{
	offset.SetToolLengthComp(line.GetParam('H'));
}
//Tool compensation in negative direction
void GCodeInterpreter::G44(GCodeLine& line)
{
	offset.SetToolLengthComp(-line.GetParam('H'));
}
//Tool legth compensation off
void GCodeInterpreter::G49(GCodeLine& line)
{
	offset.ZeroToolLengthCom();
}