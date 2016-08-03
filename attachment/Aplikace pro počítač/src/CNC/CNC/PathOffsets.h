#pragma once
#include "Geometry.h"

//This structure represents tool correction for the G-code processing
class PathOffsets
{
public:
	PathOffsets(void);
	~PathOffsets(void);
	float GetToolComp(){return toolCompensation;};
	float GetToolLegthComp(){return length;};
	Point GetOffset(){return offset;};
	void SetToolComp(float l){toolCompensation = l;};
	void SetToolLengthComp(float l){length = l;};
	void SetOffset(Point o){offset =  o;};
	void IncreaseToolLengthComp(float f){length += f;};
	void IncreaseOffset(Point p){offset += p;};
	void ZeroToolCompensation(){toolCompensation = 0;};
	void ZeroToolLengthCom(){length = 0;};
	void ZeroOffset(){offset = Point(0);};
private:
	float toolCompensation;//+right, -left
	float length;//rescpects the Z orientation
	Point offset;
};


extern PathOffsets nullOffset;