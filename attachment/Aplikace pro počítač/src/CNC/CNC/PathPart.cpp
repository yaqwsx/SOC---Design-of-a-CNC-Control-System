#include "PathPart.h"


//Constants definition
double PathPart::speedFactor = 0.99;
double PathPart::speedPrecision = 0.001;
double PathPart::tangentTolerance = 0.1;
size_t PathPart::numOfIterations = 50;
double PathPart::precision = 0.001;


PathPart::PathPart(void)
{
}


PathPart::~PathPart(void)
{
}

PathPart& PathPart::GetPrevious()
{
	auto it = item;
	if((*it)->GetType() != PROGRAM_BEGIN)
		it--;
	return *(*it);
}

PathPart& PathPart::GetNext()
{
	auto it = item;
	if((*it)->GetType() != PROGRAM_END)
		it++;
	return *(*it);
}

PathPart& PathPart::GetNextMovable()
{
	PathPart* ret = this;
	if(GetType() != PROGRAM_END)
	{
		do
		{
			ret = &ret->GetNext();
		}
		while(!ret->IsMovable() && ret->GetType() != PROGRAM_END);
	}
	return* ret;
}

PathPart& PathPart::GetPreviousMovable()
{
	PathPart* ret = this;
	if(GetType() != PROGRAM_BEGIN)
	{
		do
		{
			ret = &ret->GetPrevious();
		}
		while(!ret->IsMovable() && ret->GetType() != PROGRAM_BEGIN);
	}
	return* ret;
}


void PathPart::ProcessToolCompensation(GCodeInterpreter& in)
{
	PathPart* prev = this;
	do{
		prev = &prev->GetPreviousMovable();
	} while((prev->GetType() == OFFSET_CIRCLE_CW || 
		prev->GetType() == OFFSET_CIRCLE_CCW || prev->GetType() == LINE_OFFSET) && prev->GetType() != PROGRAM_BEGIN);
	ProcessCompensationStarting(prev, &GetNextMovable(), in);
	ProcessCompensationEnding(&GetNextMovable(), prev, in);
}

void  PathPart::ProcessCompensationStarting(PathPart* prev, PathPart* next, GCodeInterpreter& in)
{
	if(prev->GetType() == PROGRAM_BEGIN)
		if(GetOffset().GetToolComp() != 0)
		{
			//Starting segment
			ProcessChangedOffsetStart(prev, next, in);
			return;
		}
		else
		{
			//Starting segment with no offset
			ProcessNoOffsetStart(prev);
			return;
		}


	if(prev->GetOffset().GetToolComp() == 0 &&
		GetOffset().GetToolComp() == 0)
	{
		//There's no offset
		ProcessNoOffsetStart(prev);
		return;
	}

	if(prev->GetOffset().GetToolComp() == GetOffset().GetToolComp())
	{
		//Nothing changes, keep compensation
		ProcessUnchangedOffsetStart(prev, next, in);
		return;
	}
	//offset changed, process
	ProcessChangedOffsetStart(prev, next, in);
}

void  PathPart::ProcessCompensationEnding(PathPart* next, PathPart* prev, GCodeInterpreter& in)
{
	if(next->GetType() == PROGRAM_END)
	{
		if(GetOffset().GetToolComp() == 0)
		{
			//Starting segment with no offset
			ProcessNoOffsetEnd(next);
			return;
		}
		if(GetOffset().GetToolComp() != prev->GetOffset().GetToolComp())
		{
			//Starting segment
			ProcessChangedOffsetEnd(next, prev, in);
			return;
		}
		else
		{
			ProcessUnchangedOffsetEnd(next, prev, in);
			return;
		}
	}

	if(prev->GetType() == PROGRAM_BEGIN)
		if(GetOffset().GetToolComp() != 0)
		{
			//Starting segment
			ProcessChangedOffsetEnd(next, prev, in);
			return;
		}
		else
		{
			//Starting segment with no offset
			ProcessNoOffsetEnd(prev);
			return;
		}


	if(/*prev->GetOffset().GetToolComp() == 0 &&*/
		GetOffset().GetToolComp() == 0)
	{
		//There's no offset
		ProcessNoOffsetEnd(next);
		return;
	}

	if(next->GetOffset().GetToolComp() == GetOffset().GetToolComp())
	{
		//Nothing changes, keep compensation
		ProcessUnchangedOffsetEnd(next, prev, in);
		return;
	}
	//offset changed, process
	ProcessChangedOffsetEnd(next, prev, in);
}


//Return projection of the given 3D vector to the XY plane
Vector2D PathPart::GetProjection(Vector3D& vector)
{
	Vector2D ret;
	ret.x = vector.x;
	ret.y = vector.y;
	ret.Normalize();
	return ret;
}