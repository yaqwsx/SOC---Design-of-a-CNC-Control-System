#pragma once
#include "pathpart.h"



class PathPartMovable :
	public PathPart
{
public:
	//Constructor
	PathPartMovable(Point begining, Point end, double feed, WorkingPlane p, size_t line);
	~PathPartMovable(void);
	bool IsMovable() sealed;

	//Define functions for movable parts
	double GetStartingSpeed();
	double GetEndingSpeed();
	double GetDesignedSpeed();
	double GetSpeed();
	Point GetStartingPoint();
	Point GetEndingPoint();
	bool IsCommand() {return true;};
	virtual void SetStartingSpeed(double speed){startingSpeed = speed;};

protected:
	//Atribures common for all movable parts
	Point startingPoint, endingPoint;
	double startingSpeed, speed, endingSpeed;
	double designedSpeed;
	WorkingPlane plane;
};

