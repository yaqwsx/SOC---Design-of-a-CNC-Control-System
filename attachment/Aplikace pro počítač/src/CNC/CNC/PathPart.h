#pragma once
#include <list>
#include "Geometry.h"
#include "CodeModificators.h"
#include "PathOffsets.h"
#include "ProcessedData.h"

struct PathPartSimple;

using namespace std;


enum PathPartType{UNPROC, LINE_NORMAL, LINE_RAPID,
	LINE_OFFSET, CIRCLE_CCW, CIRCLE_CW, SINE,
	OFFSET_CIRCLE_CW, OFFSET_CIRCLE_CCW,
	PROGRAM_BEGIN, PROGRAM_END};

enum PathPartTypeEx{PPEx_TEXT};

struct PathPartSimple//For drawing purposes
{
	PathPartType type;
	PathPartTypeEx typeEx;
	size_t line;
	Point start, end, center;
	string text;
};

class GCodeInterpreter;


//Definition of the basic element of the path
class PathPart
{
public:
	PathPart(void);
	~PathPart(void);
	void SetLine(size_t l){line = l;};
	size_t GetLine(){return line;};
	void SetIterator(list<unique_ptr<PathPart>>::iterator& i){item = i;};
	PathPart& GetPrevious();
	PathPart& GetNext();
	PathPart& GetNextMovable();
	PathPart& GetPreviousMovable();

	virtual bool IsMovable()=0;
	virtual PathPartType GetType()=0;

	//Getters & setters
	virtual double GetStartingSpeed()=0;
	virtual double GetEndingSpeed()=0;
	virtual double GetDesignedSpeed()=0;
	virtual double GetSpeed()=0;
	virtual Point GetStartingPoint()=0;
	virtual Point GetOffsetStartingPoint()=0;
	virtual Point GetEndingPoint()=0;
	virtual Point GetOffsetEndingPoint()=0;
	virtual void SetOffsetStartingPoint(Point& p)=0;
	virtual void SetOffsetEndingPoint(Point& p)=0;
	virtual Vector3D GetStartingTangent()=0;
	virtual Vector3D GetEndingTangent()=0;
	virtual Vector3D GetOffsetStartingTangent()=0;
	virtual Vector3D GetOffsetEndingTangent()=0;
	virtual Point GetAdditionalPoin()=0;
	virtual void SetStartingSpeed(double speed)=0;
	static Vector2D GetProjection(Vector3D& vector);
	virtual PathOffsets& GetOffset()=0;

	//ProcessFunction
	virtual void ProcessToolCompensation(GCodeInterpreter& in);
	virtual void ComputeSpeeds(GCodeInterpreter& in)=0;
	virtual void ProcessCompensationStarting(PathPart* prev, PathPart* next, GCodeInterpreter& in);
	virtual void ProcessCompensationEnding(PathPart* next, PathPart* prev, GCodeInterpreter& in);
	virtual void ProcessUnchangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in)=0;
	virtual void ProcessChangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in)=0;
	virtual void ProcessNoOffsetStart(PathPart* prev)=0;
	virtual void ProcessUnchangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in)=0;
	virtual void ProcessChangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in)=0;
	virtual void ProcessNoOffsetEnd(PathPart* next)=0;
	
	//ConvertFunctions
	virtual bool IsCommand()=0;
	virtual ProcessedData TransformToCommand()=0;
	virtual PathPartSimple ToSimple()=0;
	virtual PathPartSimple ToOffsetSimple()=0;
	virtual bool IsOffset()=0;

protected:
	//iterator to current item in the path
	list<unique_ptr<PathPart>>::iterator item;

private:
	size_t line;//In which line was this part defined?

protected:
	//Constats
	static double speedFactor;
	static double speedPrecision;
	static double tangentTolerance;
	static double precision;//Precision for Newton's method
	static size_t numOfIterations;//Max num of iterations for Newton's method
};

