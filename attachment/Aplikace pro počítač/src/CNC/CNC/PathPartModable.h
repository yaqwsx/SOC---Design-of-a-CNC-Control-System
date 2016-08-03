#pragma once
#include "pathpart.h"
#include <exception>


//PathPart modable - this type douesn't contain movement
class PathPartModable :
	public PathPart
{
public:
	PathPartModable(void);
	~PathPartModable(void);
	bool IsMovable() sealed {return false;};

	//Disable getters & setters
	virtual double GetStartingSpeed() sealed {DisabledMethod(); return 0;};
	virtual double GetEndingSpeed() sealed {DisabledMethod(); return 0;};
	virtual double GetDesignedSpeed() sealed {DisabledMethod(); return 0;};
	virtual double GetSpeed() sealed {DisabledMethod(); return 0;};
	virtual Point GetStartingPoint() sealed {DisabledMethod(); return 0;};
	virtual Point GetEndingPoint() sealed {DisabledMethod(); return 0;};
	virtual Point GetOffsetStartingPoint() sealed {DisabledMethod(); return 0;};
	virtual Point GetOffsetEndingPoint() sealed {DisabledMethod(); return 0;};
	virtual Vector3D GetStartingTangent() sealed {DisabledMethod(); return 0;};
	virtual Vector3D GetEndingTangent() sealed {DisabledMethod(); return 0;};
	virtual void SetOffsetStartingPoint(Point& p) sealed {DisabledMethod();};
	virtual void SetOffsetEndingPoint(Point& p) sealed {DisabledMethod();};
	virtual Vector3D GetOffsetStartingTangent() sealed {DisabledMethod(); return 0;};
	virtual Vector3D GetOffsetEndingTangent() sealed {DisabledMethod(); return 0;};
	PathOffsets& GetOffset() sealed {DisabledMethod(); return nullOffset;};
	PathPartSimple ToOffsetSimple(){DisabledMethod(); return PathPartSimple();};
	bool IsOffset(){return false;};
	Point GetAdditionalPoin(){DisabledMethod(); return Point();};
	void ComputeSpeeds(GCodeInterpreter& in){};
	virtual void SetStartingSpeed(double speed){DisabledMethod();};
	
	void ProcessUnchangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in) sealed {DisabledMethod();};
	void ProcessChangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in) sealed {DisabledMethod();};
	void ProcessNoOffsetStart(PathPart* prev) sealed {DisabledMethod();};
	void ProcessUnchangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in) sealed {DisabledMethod();};
	void ProcessChangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in) sealed {DisabledMethod();};
	void ProcessNoOffsetEnd(PathPart* next) sealed {DisabledMethod();};

protected:
	void DisabledMethod() {throw exception("BUG: Volání movable metody na modable objektu");}
};

