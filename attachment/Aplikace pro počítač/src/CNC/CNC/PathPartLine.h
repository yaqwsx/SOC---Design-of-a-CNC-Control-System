#pragma once
#include "pathpartmovable.h"
#include "PathOffsets.h"


class PathPartLine :
	public PathPartMovable
{
public:
	//Constructor
	PathPartLine(Point beginning, Point end, double feed, WorkingPlane p, PathOffsets o, size_t line);
	~PathPartLine(void);

	//Getters & setters
	PathPartType GetType(){return LINE_NORMAL;};
	Vector3D GetStartingTangent();
	Vector3D GetEndingTangent();
	Vector3D GetOffsetStartingTangent();
	Vector3D GetOffsetEndingTangent();
	Point GetOffsetStartingPoint(){return offsetStartingPoint;};
	Point GetOffsetEndingPoint(){return offsetEndingPoint;};
	void SetOffsetStartingPoint(Point& p){offsetStartingPoint = p;};
	void SetOffsetEndingPoint(Point& p){offsetEndingPoint = p;};
	PathOffsets& GetOffset(){return offset;};
	Point GetAdditionalPoin(){throw exception("BUG: Volání dodateèného bodu u pøímky."); return Point();};

	//ConvertFunctions
	PathPartSimple ToOffsetSimple();
	ProcessedData TransformToCommand();
	bool IsOffset();
	PathPartSimple ToSimple();

	//Process fucntions
	void ProcessUnchangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in);
	void ProcessChangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in);
	void ProcessNoOffsetStart(PathPart* prev);
	void ProcessUnchangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in);
	void ProcessChangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in);
	void ProcessNoOffsetEnd(PathPart* next);
	void ComputeSpeeds(GCodeInterpreter& in);

private:
	Vector2D ProcessCircleInnerCorner(PathPart* next);
	Vector2D ProcessInnerCorner(Vector2D u, Vector2D v, Vector2D A, Vector2D B, double c1, double c2);
	Vector2D ProcessOuterCornerEnd(Point offStart, Point offEnd, double toolComp, bool dirChange);
	Vector2D ProcessInnerCornerCircleCircle();
	bool ComputeBrakingSpeed(GCodeInterpreter& in, PathPart* next, PathPart* prev);
	bool ComputeSpeedUsingJerk(GCodeInterpreter& in, PathPart* next, PathPart* prev);
	bool ComputeSpeedUsingStartingJerk(GCodeInterpreter& in, PathPart* next, PathPart* prev);
	bool ComputeSpeedUsingEndingJerk(GCodeInterpreter& in, PathPart* next, PathPart* prev);
	bool ComputeSpeedWithoutJerk(GCodeInterpreter& in, PathPart* next, PathPart* prev);
	bool DirChangedEnd(PathPart* p);

	double GetSFromStartMovement(double v, double v0, double A, double J, bool save);
	double GetSFromEndMovement(double v, double vb, double A, double J, bool save);
	void GetATFromMovement(double v, double v2, double Ak, double J, double& A, double& T);

	PathOffsets offset;
	Point offsetStartingPoint, offsetEndingPoint;
	double Astart, Aend;//Acceleration values
};

