#pragma once
#include "pathpartmovable.h"

//Part created as a result of cutter compensation. It has no reference to input G-code
class PathPartOffsetLine :
	public PathPartMovable
{
public:
	//Constructors
	PathPartOffsetLine(Point beginning, Point end, double feed, WorkingPlane p, size_t line);
	~PathPartOffsetLine(void);

	//Getters & setters
	PathPartType GetType(){return LINE_OFFSET;};
	Vector3D GetStartingTangent();
	Vector3D GetEndingTangent();
	Vector3D GetOffsetStartingTangent();
	Vector3D GetOffsetEndingTangent();
	PathPartSimple ToSimple();
	Point GetOffsetStartingPoint(){return startingPoint;};
	Point GetOffsetEndingPoint(){return endingPoint;};
	void SetOffsetStartingPoint(Point& p){throw exception("BUG: Nastavení offsetu u odsazené pøímky.");};
	void SetOffsetEndingPoint(Point& p){throw exception("BUG: Nastavení offsetu u odsazené pøímky.");};
	PathOffsets& GetOffset(){throw exception("BUG: Snaha o získání offsetu u odsazené pøímky."); return nullOffset;};
	Point GetAdditionalPoin(){throw exception("BUG: Volání dodateèného bodu u pøímky."); return Point();};

	//Convert functions
	PathPartSimple ToOffsetSimple();
	bool IsOffset(){return true;};
	ProcessedData TransformToCommand();

	//Process functions
	void ProcessUnchangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};;
	void ProcessChangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};
	void ProcessNoOffsetStart(PathPart* prev){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};
	void ProcessUnchangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};
	void ProcessChangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};
	void ProcessNoOffsetEnd(PathPart* next){throw exception("BUG: Volání funkce pro provedení odsazení na odsazeném prvku");};
	void ComputeSpeeds(GCodeInterpreter& in);

private:
	double Astart, Aend;
	double GetSFromStartMovement(double v, double v0, double A, double J, bool save);
	double GetSFromEndMovement(double v, double vb, double A, double J, bool save);
	void GetATFromMovement(double v, double v2, double Ak, double J, double& A, double& T);
};

