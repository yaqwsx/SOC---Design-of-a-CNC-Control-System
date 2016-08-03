#pragma once
#include "pathpartmovable.h"

//Part created as a result of cutter compensation. It has no reference to input G-code
class PathPartOffsetCircle :
	public PathPartMovable
{
public:
	//Constructors
	PathPartOffsetCircle(Point beginning, Point ending, Point center, double feed, PathPartType t, size_t line);
	~PathPartOffsetCircle(void);

	//Getters & setters
	PathPartType GetType(){return type;};
	Point GetStartingPoint(){return center;};
	Point GetEndingPoint(){return center;};
	Point GetOffsetStartingPoint(){return startingPoint;};
	Point GetOffsetEndingPoint(){return endingPoint;};
	PathOffsets& GetOffset(){throw exception("BUG: Volání funkce pro odsazení na odsazeném prvku");return nullOffset;};
	void SetOffsetStartingPoint(Point& p){throw exception("BUG: Volání funkce pro odsazení na odsazeném prvku");};
	void SetOffsetEndingPoint(Point& p){throw exception("BUG: Volání funkce pro odsazení na odsazeném prvku");};
	Vector3D GetStartingTangent();
	Vector3D GetEndingTangent();
	Vector3D GetOffsetStartingTangent();
	Vector3D GetOffsetEndingTangent();
	Point GetAdditionalPoin(){return center;};

	//Convert functions
	PathPartSimple ToSimple();
	PathPartSimple ToOffsetSimple();
	bool IsOffset(){return true;};
	ProcessedData TransformToCommand();
	bool IsCommand() {return startingPoint != endingPoint;};

	//Process functions
	void ComputeSpeeds(GCodeInterpreter& in);
	void ProcessUnchangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};
	void ProcessChangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};
	void ProcessNoOffsetStart(PathPart* prev){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};
	void ProcessUnchangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};
	void ProcessChangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};
	void ProcessNoOffsetEnd(PathPart* next){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};

private:
	double GetCircleLength();
	Point GetCirclePoint();
	Vector2D Get3DVectorProjection(Vector3D a);
	Vector3D Project2DVectorTo3D(Vector2D a);
	Vector3D ComputeTangent(Vector3D a);
	double r;
	Point center;
	PathPartType type;
	float Astart, Aend;

	//Speed computation
	double GetSFromStartMovement(double v, double v0, double r, double A, double J, bool save);
	double GetSFromEndMovement(double v, double vb, double r, double A, double J, bool save);
	void GetATFromMovement(double v, double v2, double r, double Ak, double J, double& A, double& T);
	void ComputeATLimitedByJerk(double dv, double r, double J, bool sign, double& A, double& T);
};

