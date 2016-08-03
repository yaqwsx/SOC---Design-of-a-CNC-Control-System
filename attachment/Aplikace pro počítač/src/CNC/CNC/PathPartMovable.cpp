#include "PathPartMovable.h"


PathPartMovable::PathPartMovable(Point begining, Point end, double feed, WorkingPlane p, size_t line):
startingPoint(begining), endingPoint(end), designedSpeed(feed), plane(p), startingSpeed(0),
	speed(0), endingSpeed(0)
{
	SetLine(line);
}

bool PathPartMovable::IsMovable()
{
	return true;
}


PathPartMovable::~PathPartMovable(void)
{
}

double PathPartMovable::GetStartingSpeed()
{
	return startingSpeed;
}

double PathPartMovable::GetEndingSpeed()
{
	return endingSpeed;
}

double PathPartMovable::GetDesignedSpeed()
{
	return designedSpeed;
}

double PathPartMovable::GetSpeed()
{
	return speed;
}

Point PathPartMovable::GetStartingPoint()
{
	return startingPoint;
}

Point PathPartMovable::GetEndingPoint()
{
	return endingPoint;
}