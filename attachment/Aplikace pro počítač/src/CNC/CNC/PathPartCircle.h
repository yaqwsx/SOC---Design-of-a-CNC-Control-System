#pragma once
#include "pathpartmovable.h"
#include "PathOffsets.h"


class PathPartCircle :
	public PathPartMovable
{
public:
	//Constructors
	PathPartCircle(Point beginning, Point ending, Point center, double speed, PathPartType t, PathOffsets o, size_t line);
	PathPartCircle(Point beginning, Point ending, double radius, WorkingPlane p, double speed, PathPartType t, PathOffsets o, size_t line);
	~PathPartCircle(void);

	//Getters & setters
	PathPartType GetType(){return type;};
	Vector3D GetStartingTangent();
	Vector3D GetEndingTangent();
	Vector3D GetOffsetStartingTangent();
	Vector3D GetOffsetEndingTangent();
	Point GetOffsetStartingPoint(){return offsetStartingPoint;};
	Point GetOffsetEndingPoint(){return offsetEndingPoint;};
	void SetOffsetStartingPoint(Point& p){offsetStartingPoint = p;};
	void SetOffsetEndingPoint(Point& p);
	PathOffsets& GetOffset(){return offset;};
	Point GetAdditionalPoin(){return center;};

	//ConvertFunctions
	PathPartSimple ToSimple();
	bool IsCommand() {return offsetStartingPoint != offsetEndingPoint;};
	PathPartSimple ToOffsetSimple();
	bool IsOffset();
	ProcessedData TransformToCommand();

	//ProcessFunctions
	void ProcessUnchangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in);
	void ProcessChangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in);
	void ProcessNoOffsetStart(PathPart* prev);
	void ProcessUnchangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in);
	void ProcessChangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in);
	void ProcessNoOffsetEnd(PathPart* next);
	void ComputeSpeeds(GCodeInterpreter& in);
	

private:
	double GetCircleLength();
	double GetOriginalCircleLength();
	Point GetCirclePoint();
	Vector2D GetCircleCenter(Vector2D A, Vector2D B);
	Vector2D Get3DVectorProjection(Vector3D a);
	Vector3D Project2DVectorTo3D(Vector2D a);
	Vector3D ComputeTangent(Vector3D a);
	void ProcessUnchangedOffsetEndMod(PathPart* next, PathPart* prev, GCodeInterpreter& in);
	void ProcessUnchangedInnerCornerLine(PathPart* next);
	void ProcessUnchangedInnerCornerCircle(PathPart* next);
	void ProcessUnchangedOuterCornerLine(PathPart* next);
	void ProcessUnchangedOuterCornerCircle(PathPart* next);
	void ProcessStartingToBigger(PathPart* prev, GCodeInterpreter& in);
	void ProcessStartingToSmaller(PathPart* prev, GCodeInterpreter& in);

	//Functions for speed computation
	double GetSFromStartMovement(double v, double v0, double r, double A, double J, bool save);
	double GetSFromEndMovement(double v, double vb, double r, double A, double J, bool save);
	void GetATFromMovement(double v, double v2, double r, double Ak, double J, double& A, double& T);
	void ComputeATLimitedByJerk(double dv, double r, double J, bool sign, double& A, double& T);

	//Circle atributes
	double r;
	double Astart, Aend, Akstart, Akend;
	Point center;
	Point offsetStartingPoint, offsetEndingPoint;
	PathPartType type;
	PathOffsets offset;
};

