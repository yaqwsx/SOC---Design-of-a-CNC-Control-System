#define _USE_MATH_DEFINES
#include "PathPartLine.h"
#include "GCodeInterpreter.h"
#include "PathPartOffsetCircle.h"
#include "PathPartCircle.h"
#include "CommunicationEnumerations.h"
#include <cmath>


//Default constructor
PathPartLine::PathPartLine(Point beginning, Point end, double f, WorkingPlane p, PathOffsets o, size_t line):
PathPartMovable(beginning, end, f, p, line), offset(o), offsetStartingPoint(beginning),
	offsetEndingPoint(end)
{
}


PathPartLine::~PathPartLine(void)
{
}


Vector3D PathPartLine::GetStartingTangent()
{
	Vector3D ret(startingPoint, endingPoint);
	ret.Normalize();
	return ret;
}


Vector3D PathPartLine::GetEndingTangent()
{
	return GetStartingTangent();
}

Vector3D PathPartLine::GetOffsetStartingTangent()
{
	Vector3D ret(offsetStartingPoint, offsetEndingPoint);
	ret.Normalize();
	return ret;
}

Vector3D PathPartLine::GetOffsetEndingTangent()
{
	return GetOffsetStartingTangent();
}

PathPartSimple PathPartLine::ToSimple()
{
	PathPartSimple ret;
	ret.type = LINE_NORMAL;
	ret.line = GetLine();
	ret.start = startingPoint;
	ret.end = endingPoint;
	return ret;
}

void PathPartLine::ProcessUnchangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in)
{
	if(prev->GetType() == PROGRAM_BEGIN)
	{
		//Starting segment
		//Nothing to do here
		return;
	}
	Point start = GetOffsetStartingPoint();
	if(start != GetStartingPoint())
		return;
	Vector2D norm = GetProjection(GetStartingTangent());
	norm = norm.GetNormalVector();
	if(norm == Vector2D(0, 0))
	{
		//The line is pralel to Z-axis
		//We can't apply any correction based on this line
		Point end = prev->GetOffsetEndingPoint();
		start.x = end.x;
		start.y = end.y;
		SetOffsetStartingPoint(start);
		return;
	}
	//The line has a projenction into XY plane
	//Check for the corner
	Vector2D tangent = GetProjection(GetStartingTangent());
	Vector2D prevTangent = GetProjection(prev->GetEndingTangent());
	if(tangent == prevTangent)
	{
		//There's no need for advanced transition
		//Parts are tangential
		norm = norm * GetOffset().GetToolComp();
		start.x += norm.x;
		start.y += norm.y;
		SetOffsetStartingPoint(start);
		return;
	}
	//Check for the corner
	if(prevTangent == Vector2D(0, 0))
	{
		//The previous line is parallel with the next one
		//Nothing to do there
		Point end = prev->GetOffsetEndingPoint();
		start.x = end.x;
		start.y = end.y;
		SetOffsetStartingPoint(start);
		return;
	}
	bool corner;
	if(prev->GetOffset().GetToolComp() > 0)
	{
		corner = HalfPlane(tangent, norm) != HalfPlane(tangent, prevTangent);
	}
	else
	{
		corner = HalfPlane(tangent, norm) == HalfPlane(tangent, prevTangent);
	}
	if(corner)
	{
		//Inner corner
		//This movement has to start from last ending
		SetOffsetStartingPoint(prev->GetOffsetEndingPoint());
		return;
	}
	else
	{
		//Outer corner
		norm = norm * prev->GetOffset().GetToolComp();
		start.x += norm.x;
		start.y += norm.y;
		SetOffsetStartingPoint(start);
		//Add transition circle
		PathPartType type;
		if(prev->GetOffset().GetToolComp() > 0)
			type = OFFSET_CIRCLE_CW;
		else
			type = OFFSET_CIRCLE_CCW;
		in.AddPathPart(unique_ptr<PathPart>(new 
			PathPartOffsetCircle(prev->GetOffsetEndingPoint(), start, GetStartingPoint(), GetDesignedSpeed(), type, GetLine())),
			item);
		return;
	}
}

void PathPartLine::ProcessChangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in)
{
	ProcessUnchangedOffsetStart(prev, next, in);//Procedure is the same
}

void PathPartLine::ProcessNoOffsetStart(PathPart* prev)
{

}

void PathPartLine::ProcessUnchangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in)
{
	Point end = GetEndingPoint();
	if(end != GetOffsetEndingPoint())
		return;
	Vector2D norm = GetProjection(GetEndingTangent());
	norm = norm.GetNormalVector();
	if(norm == Vector2D(0, 0))
	{
		//The line is pralel to Z-axis
		//We can't apply any correction based on this line
		Point start = GetOffsetStartingPoint();
		end.x = start.x;
		end.y = start.y;
		SetOffsetEndingPoint(end);
		return;
	}
	//The line has a projenction into XY plane
	//Check for the corner
	Vector2D tangent = GetProjection(GetEndingTangent());
	Vector2D nextTangent;
	if(next->GetType() != PROGRAM_END)
		nextTangent = GetProjection(next->GetStartingTangent());
	else
		nextTangent = tangent;
	if(tangent == nextTangent)
	{
		//There's no need for advanced transition
		//Parts are tangential
		norm = norm * GetOffset().GetToolComp();
		end.x += norm.x;
		end.y += norm.y;
		SetOffsetEndingPoint(end);
		return;
	}
	if(nextTangent == Vector2D(0, 0))
	{
		//The next line is parallel to Z axis
		//We have to skip it and calculate offset based on the next one.
		PathPart* another;
		while(nextTangent == Vector2D(0, 0))
		{
			another = &next->GetNextMovable();
			nextTangent = GetProjection(another->GetStartingTangent());
		}
		bool corner;
		if(GetOffset().GetToolComp() > 0)
		{
			corner = HalfPlane(tangent, norm) == HalfPlane(tangent, nextTangent);
		}
		else
		{
			corner = HalfPlane(tangent, norm) != HalfPlane(tangent, nextTangent);
		}
		if(corner)
		{
			if(prev->GetType() == PROGRAM_BEGIN ||
				prev->GetOffset().GetToolComp() != GetOffset().GetToolComp())
			{
				//This is a lead in movement
				Vector2D off = GetProjection(another->GetStartingTangent()).GetNormalVector()*another->GetOffset().GetToolComp();
				end = another->GetStartingPoint();
				end.x += off.x;
				end.y += off.y;
				SetOffsetEndingPoint(end);
				return;
			}
			//Inner corner
			Vector2D point;
			if(another->GetType() == CIRCLE_CCW || another->GetType() == CIRCLE_CW)
			{
				point = ProcessCircleInnerCorner(another);
			}
			else
			{
				point = ProcessInnerCorner(tangent, nextTangent,
					Vector2D(GetEndingPoint().x, GetEndingPoint().y),
					Vector2D(another->GetStartingPoint().x, another->GetStartingPoint().y),
					GetOffset().GetToolComp(), another->GetOffset().GetToolComp());
			}
			end.x = point.x;
			end.y = point.y;
			SetOffsetEndingPoint(end);
			return;
		}
		else
		{
			//Outer corner
			if(prev->GetType() == PROGRAM_BEGIN
				|| GetOffset().GetToolComp() != prev->GetOffset().GetToolComp())
			{
				Vector2D of = ProcessOuterCornerEnd(GetOffsetStartingPoint(),
					GetEndingPoint(), GetOffset().GetToolComp(), DirChangedEnd(prev));
				end.x += of.x;
				end.y += of.y;
				SetOffsetEndingPoint(end);
				return;
			}
			else
			{
				//No change, simple offset
				norm = norm * GetOffset().GetToolComp();
				end.x += norm.x;
				end.y += norm.y;
				SetOffsetEndingPoint(end);
				return;
			}
		}
	}
	//Check for the corner
	bool corner;
	if(GetOffset().GetToolComp() > 0)
	{
		corner = HalfPlane(tangent, norm) == HalfPlane(tangent, nextTangent);
	}
	else
	{
		corner = HalfPlane(tangent, norm) != HalfPlane(tangent, nextTangent);
	}
	if(corner)
	{
		//Inner corner
		if(prev->GetType() == PROGRAM_BEGIN ||
			prev->GetOffset().GetToolComp() != GetOffset().GetToolComp())
		{
			//This is a lead in movement, it has to be normal to the next line
			Vector2D off = GetProjection(next->GetStartingTangent()).GetNormalVector()*next->GetOffset().GetToolComp();
			end = next->GetStartingPoint();
			end.x += off.x;
			end.y += off.y;
			SetOffsetEndingPoint(end);
			return;
		}
		Vector2D point;
		if(next->GetType() == CIRCLE_CCW || next->GetType() == CIRCLE_CW)
		{
			point = ProcessCircleInnerCorner(next);
		}
		else
		{
			point = ProcessInnerCorner(tangent, nextTangent,
				Vector2D(GetEndingPoint().x, GetEndingPoint().y),
				Vector2D(next->GetStartingPoint().x, next->GetStartingPoint().y),
				GetOffset().GetToolComp(), next->GetOffset().GetToolComp());
		}
		end.x = point.x;
		end.y = point.y;
		SetOffsetEndingPoint(end);
		return;
	}
	else
	{
		//Outer corner
		if(prev->GetType() == PROGRAM_BEGIN
			|| GetOffset().GetToolComp() != prev->GetOffset().GetToolComp())
		{
			Vector2D of = ProcessOuterCornerEnd(GetOffsetStartingPoint(),
				GetEndingPoint(), GetOffset().GetToolComp(), DirChangedEnd(prev));
			end.x += of.x;
			end.y += of.y;
			SetOffsetEndingPoint(end);
			return;
		}
		else
		{
			//No change, simple offset
			norm = norm * GetOffset().GetToolComp();
			end.x += norm.x;
			end.y += norm.y;
			SetOffsetEndingPoint(end);
			return;
		}
	}

}

void PathPartLine::ProcessChangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in)
{
	ProcessUnchangedOffsetEnd(next, prev, in);//Processing is the same
}

void PathPartLine::ProcessNoOffsetEnd(PathPart* next)
{

}

PathPartSimple PathPartLine::ToOffsetSimple()
{
	PathPartSimple ret;
	ret.type = LINE_NORMAL;
	ret.line = GetLine();
	ret.start = offsetStartingPoint;
	ret.end = offsetEndingPoint;
	return ret;
}

bool PathPartLine::IsOffset()
{
	bool ret = startingPoint != offsetStartingPoint ||
		endingPoint != offsetEndingPoint;
	return ret;
}

Vector2D PathPartLine::ProcessInnerCorner(Vector2D u, Vector2D v, Vector2D A, Vector2D B, double c1, double c2)
{
	Vector2D vNorm = v.GetNormalVector() * c2;
	Vector2D norm = u.GetNormalVector() * c1;
	double t = (v.y*(norm.x+A.x-vNorm.x-B.x)-v.x*(norm.y+A.y-vNorm.y-B.y))/(-u.x*v.y+u.y*v.x);
	//Calculate the point
	Vector2D point = A + u*t+norm;
	return point;
}


Vector2D PathPartLine::ProcessOuterCornerEnd(Point offsetStart, Point ending, double l, bool dirChange)
{
	Vector2D v(offsetStart.x - ending.x, offsetStart.y - ending.y);
	double D = sqrt(l*l*v.y*v.y*(v.x*v.x+v.y*v.y-l*l));
	double D2 = v.x*sqrt(-v.y*v.y*l*l*(l*l-v.x*v.x-v.y*v.y))/(v.x*v.x+v.y*v.y);
	//Check if we are changing direction of compensation
	Vector2D of;
	if(dirChange)
	{
		//Direction of compensation wasn't changed
		of.x = (v.x*l*l-D)/(v.x*v.x+v.y*v.y);
		of.y = (l*l*(1-v.x*v.x/(v.x*v.x+v.y*v.y))+D2)/v.y;
	}
	else
	{
		//Direction was changed, use other sign
		of.x = (v.x*l*l+D)/(v.x*v.x+v.y*v.y);
		of.y = (l*l*(1-v.x*v.x/(v.x*v.x+v.y*v.y))-D2)/v.y;
	}
	return of;
}

bool PathPartLine::DirChangedEnd(PathPart* p)
{
	if(p->GetType() == PROGRAM_BEGIN || p->GetOffset().GetToolComp() == 0)
	{
		return GetOffset().GetToolComp() > 0;
	}
	return (GetOffset().GetToolComp() >= 0 && p->GetOffset().GetToolComp() <= 0) ||
				(GetOffset().GetToolComp() <= 0 && p->GetOffset().GetToolComp() >= 0);
}

Vector2D PathPartLine::ProcessCircleInnerCorner(PathPart* next)
{
	//We need to find intersection circle-line
	Vector2D tangent = GetProjection(GetStartingTangent());
	Vector2D normal = tangent.GetNormalVector()*GetOffset().GetToolComp();
	Point start = GetStartingPoint();
	double k1 = normal.x + start.x - next->GetAdditionalPoin().x;
	double k2 = normal.y + start.y - next->GetAdditionalPoin().y;
	double b = 2*(tangent.x*k1+tangent.y*k2);
	Vector3D rad(next->GetStartingPoint(), next->GetAdditionalPoin());
	double r;
	if(next->GetType() == CIRCLE_CCW)
		r = rad.Length() - next->GetOffset().GetToolComp();
	else
		r = rad.Length() + next->GetOffset().GetToolComp();
	double a = (tangent.x*tangent.x+tangent.y*tangent.y);
	double D = b*b-4*a*(k1*k1+k2*k2-r*r);
	if(D < 0)
		throw exception("Nelze nalézt spoleèný prùseèík pøi vytvoøení odsazení. Korekce je pøíliš velká.");
	D = sqrt(D);
	double t1 = (-b-D)/(2*a);
	double t2 = (-b+D)/(2*a);
	double t = max(t1, t2);
	Vector2D res = tangent*t;
	res.x += start.x + normal.x;
	res.y += start.y + normal.y;

	return res;
}

void PathPartLine::ComputeSpeeds(GCodeInterpreter& in)
{
	PathPart* prev = &GetPreviousMovable();
	PathPart* next = &GetNextMovable();

	//Test ending speed of element
	if(next->GetType() == PROGRAM_END)
		endingSpeed = 0;
	else
		endingSpeed = next->GetStartingSpeed();
	//Compute length of the line
	double S = Vector3D(offsetStartingPoint, offsetEndingPoint).Length();

	if(S == 0)
	{
		//This is null part
		startingSpeed = speed = endingSpeed = next->GetStartingSpeed();
		Astart = Aend = in.GetAcceleration();
		return;
	}

	//Test if we should brake - firt element or non-tangential parts
	if(prev->GetType() == PROGRAM_BEGIN ||
		!Compare3DVectors(GetOffsetStartingTangent(), prev->GetOffsetEndingTangent(), tangentTolerance))
			startingSpeed = 0;
	else
	{
		startingSpeed = prev->GetDesignedSpeed();
		if(startingSpeed > designedSpeed)
			startingSpeed = designedSpeed;
		//Is it possible to break?
		if(startingSpeed > endingSpeed)
		{
			double S2 = GetSFromEndMovement(startingSpeed, endingSpeed,
				in.GetAcceleration(), in.GetJerk(), true);
			if(S2 > S)
			{
				//It's impossible to brake;
				//Find new speed using bisection
				double v = startingSpeed;
				double dv = startingSpeed - endingSpeed;
				bool last = true;
				for(int i = 0; i != 10 && (fabs(S - S2) > 0.01 || S2 > S); i++)
				{
					dv /= 2;
					if(last)
						v -= dv;
					else
						v += dv;
					S2 = S2 = GetSFromEndMovement(v, endingSpeed,
						in.GetAcceleration(), in.GetJerk(), true);
					if(S2 > S)
						last = true;
					else
						last = false;
				}
				startingSpeed = designedSpeed = speed = v;
			}
		}
	}

	//Test, if it is possible to reach designed speed
	double S1 = GetSFromStartMovement(designedSpeed, startingSpeed,
				in.GetAcceleration(), in.GetJerk(), true);
	double S2 = GetSFromEndMovement(designedSpeed, endingSpeed,
				in.GetAcceleration(), in.GetJerk(), true);

	if(S2 < 0.001 && S1 > S)
	{
		//We are not able to accelerate
		//Find new speed using bisection
		double v = designedSpeed;
		double dv = designedSpeed - startingSpeed;
		bool last = true;
		for(int i = 0; i != 10 && (fabs(S - S1) > 0.01 || S1 > S); i++)
		{
			dv /= 2;
			if(last)
				v -= dv;
			else
				v += dv;
			S1 = GetSFromStartMovement(v, startingSpeed,
				in.GetAcceleration(), in.GetJerk(), true);
			if(S1 > S)
				last = true;
			else
				last = false;
		}
		speed = endingSpeed = designedSpeed = v;
		if(next->GetStartingSpeed() != 0)
			next->SetStartingSpeed(v);
		next->ComputeSpeeds(in);
	}
	else if(S1 + S2 > S)
	{
		//It's impossile to reached designed speed, find new one
		//Using bisection
		double v = designedSpeed;
		double dv = min(designedSpeed - startingSpeed, designedSpeed - endingSpeed);
		bool last = true;
		for(int i = 0; i != 10 && (fabs(S - S1 - S2) > 0.01 || S1 + S2 > S); i++)
		{
			dv /= 2;
			if(last)
				v -= dv;
			else
				v += dv;
			S1 = GetSFromStartMovement(v, startingSpeed,
				in.GetAcceleration(), in.GetJerk(), true);
			S2 = GetSFromEndMovement(v, endingSpeed,
				in.GetAcceleration(), in.GetJerk(), true);
			if(S1 + S2 > S)
				last = true;
			else
				last = false;
		}
		designedSpeed = speed = v;
		if(S1+S2 > S)
		{
			ComputeSpeeds(in);
		}
	}
	else
		speed = designedSpeed;

	if(Astart == 0)
		Astart = in.GetAcceleration();
	if(Aend == 0)
		Aend = in.GetAcceleration();

}

ProcessedData PathPartLine::TransformToCommand()
{
	ProcessedData temp;
	temp.line = GetLine();
	temp.data.ID = IDcounter.GetID();
	temp.data.type = COM_AXIS_LINE;
	temp.data.receiver = COM_RECEIVER_AXIS;
	temp.data.line.axes[0] = offsetEndingPoint.x;
	temp.data.line.axes[1] = offsetEndingPoint.y;
	temp.data.line.axes[2] = offsetEndingPoint.z;
	temp.data.line.v = speed*.98;
	temp.data.line.v0 = startingSpeed*.98;
	temp.data.line.vb = endingSpeed*.98;
	temp.data.line.As = Astart;
	temp.data.line.Ab = Aend;
	return temp;
}


bool PathPartLine::ComputeBrakingSpeed(GCodeInterpreter& in, PathPart* next, PathPart* prev)
{
	double b = endingSpeed;
	//Initial guess for Newton's method
	double v = max(endingSpeed*1.00001, 0.00001);
	double J = in.GetJerk();
	//Length of the line
	double S = Vector3D(offsetStartingPoint, offsetEndingPoint).Length();
	if(S == 0)
	{
		//This is null part
		startingSpeed = speed = endingSpeed = next->GetStartingSpeed();
		Astart = Aend = in.GetAcceleration();
		return true;
	}

	//Initial set of difference
	double d = 1;
	for(size_t i = 0; i != numOfIterations && fabs(d) > precision; i++)
	{
		d = 2*(b*b*M_PI-M_PI*v*v+2*J*S*sqrt(2*(v-b)/J))/(M_PI*(b-3*v));
		//Just test if difference is a valid value 
		assert(d==d);
		v = v-d;
	}
	//Test, if v is a valid value
	assert(v==v);

	//Test, if the maximal acceleration wasn't exceeded
	double T = M_PI*sqrt(2*(v-b)/J)/2;
	double A = J*T/M_PI;
	if(A > in.GetAcceleration())
	{
		//Acceleration was exceeded
		v = sqrt(b*b + (4*A*S)/M_PI);
	}

	v *= speedFactor;
	v = min(v, designedSpeed);

	if(v > prev->GetDesignedSpeed())
	{
		startingSpeed = v;
		return false;
	}
	else
	{
		startingSpeed = speed = v;
		//Calculate braking acceleration
		T = M_PI*sqrt(2*(v-b)/J)/2;
		A = J*T/M_PI;
		if(A < in.GetAcceleration())
			Aend = A;
		else
			Aend = in.GetAcceleration();
		Astart = in.GetAcceleration();
		//The velocity is final, end
		return true;
	}
}

bool PathPartLine::ComputeSpeedUsingJerk(GCodeInterpreter& in, PathPart* next, PathPart* prev)
{
	//Initial guess for Newton's method
	double v = max(max(startingSpeed, endingSpeed), 0.1);
	v = _nextafter(v, v+1);

	double v0 = startingSpeed;
	double vb = endingSpeed;
	double J = in.GetJerk();
	double S = Vector3D(offsetStartingPoint, offsetEndingPoint).Length();
	if(S == 0)
	{
		//This is null part
		startingSpeed = speed = endingSpeed = next->GetStartingSpeed();
		Astart = Aend = in.GetAcceleration();
		return true;
	}
	
	//Inital set of difference
	double d = 1;
	//Newton's calculation
	for(size_t i = 0; i != numOfIterations && fabs(d) > precision; i++)
	{
		d = (2*(vb - v)*(-v0 + v)*(-2*sqrt(2.0)*S + 
			M_PI*(vb*sqrt((-vb + v)/J) + v0*sqrt((-v0 + v)/J) + 
			v*(sqrt((-vb + v)/J) + sqrt((-v0 + v)/J)))))/
			(M_PI*(v*(-3*v*(sqrt((-vb + v)/J) + sqrt((-v0 + v)/J)) + 
			v0*(3*sqrt((-vb + v)/J) + sqrt((-v0 + v)/J))) + 
			vb*(-(v0*(sqrt((-vb + v)/J) + sqrt((-v0 + v)/J))) + 
			v*(sqrt((-vb + v)/J) + 3*sqrt((-v0 + v)/J)))));

		v = v-d;
	}
	//Assign speed
	speed = min(v*speedFactor, designedSpeed);

	Astart = J*sqrt(2*(speed-startingSpeed)/J)/2;
	Aend = J*sqrt(2*(speed-endingSpeed)/J)/2;

	return false;
}


bool PathPartLine::ComputeSpeedUsingStartingJerk(GCodeInterpreter& in, PathPart* next, PathPart* prev)
{
	double v = max(max(startingSpeed, endingSpeed), 0.1);
	v = _nextafter(v, v+1);

	double o = startingSpeed;
	double b = endingSpeed;
	double J = in.GetJerk();
	double S = Vector3D(offsetStartingPoint, offsetEndingPoint).Length();
	double Ab = in.GetAcceleration();

	//Inital set of difference
	double d = 1;
	//Newton's calculation
	for(size_t i = 0; i != numOfIterations && fabs(d) > precision; i++)
	{
		d = (2*(o - v)*(-(pow(b,2)*M_PI) + M_PI*pow(v,2) + 
			Ab*(-4*S + sqrt(2.0)*M_PI*sqrt((-o + v)/J)*(o + v))))/
			(M_PI*(4*o*v + sqrt(2.0)*Ab*o*sqrt((-o + v)/J) - 
			v*(4*v + 3*sqrt(2.0)*Ab*sqrt((-o + v)/J))));
		//Debugging test
		assert(d == d);

		v = v-d;
	}

	//Assign speed
	speed = min(v*speedFactor, designedSpeed);

	Aend = in.GetAcceleration();
	Astart = J*sqrt(2*(speed-startingSpeed)/J)/2;

	return false;
}

bool PathPartLine::ComputeSpeedUsingEndingJerk(GCodeInterpreter& in, PathPart* next, PathPart* prev)
{
	double v = max(max(startingSpeed, endingSpeed), 0.1);
	v = _nextafter(v, v+1);

	double o = startingSpeed;
	double b = endingSpeed;
	double J = in.GetJerk();
	double S = Vector3D(offsetStartingPoint, offsetEndingPoint).Length();
	double A = in.GetAcceleration();

	//Inital set of difference
	double d = 1;
	//Newton's calculation
	for(size_t i = 0; i != numOfIterations && fabs(d) > precision; i++)
	{
		d = (2*(b - v)*(-(pow(o,2)*M_PI) + M_PI*pow(v,2) + 
			A*(-4*S + sqrt(2.0)*M_PI*sqrt((-b + v)/J)*(b + v))))/
			(M_PI*(4*b*v + sqrt(2.0)*A*b*sqrt((-b + v)/J) - 
			v*(4*v + 3*sqrt(2.0)*A*sqrt((-b + v)/J))));
		//Debugging test
		assert(d == d);

		v = v-d;
	}

	//Assign speed
	speed = min(v*speedFactor, designedSpeed);

	Astart = in.GetAcceleration();
	Aend = J*sqrt(2*(speed-endingSpeed)/J)/2;

	return false;
}

bool PathPartLine::ComputeSpeedWithoutJerk(GCodeInterpreter& in, PathPart* next, PathPart* prev)
{
	double v0 = startingSpeed;
	double vb = endingSpeed;
	double J = in.GetJerk();
	double S = Vector3D(offsetStartingPoint, offsetEndingPoint).Length();
	double v = sqrt((4*in.GetAcceleration()*S+M_PI*(v0*v0+vb*vb))/(2*M_PI));
	speed = min(v*speedFactor, designedSpeed);
	Astart = Aend = in.GetAcceleration();
	return false;
}

double PathPartLine::GetSFromStartMovement(double v, double o, double Ak, double J, bool save)
{
	double A = 0, T = 0;
	
	GetATFromMovement(v, o, Ak, J, A, T);

	if(save)
		Astart = A;

	assert(A*T*T/M_PI+o*T >= 0);

	//Return the length of trajectory
	return A*T*T/M_PI+o*T;
}

double PathPartLine::GetSFromEndMovement(double v, double b, double Ak, double J, bool save)
{
	double A = 0, T = 0;
	
	GetATFromMovement(v, b, Ak, J, A, T);

	if(save)
		Aend = A;

	assert(-A*T*T/M_PI+v*T >= 0);

	//Return the length of trajectory
	return -A*T*T/M_PI+v*T;
}

void PathPartLine::GetATFromMovement(double v, double v2, double Ak, double J, double& A, double& T)
{
	//Compute values
	double dv = v - v2;

	//Test if the speeds are different
	if(dv == 0)
	{
		A = Ak;
		T = 0;
		return;
	}
	T = M_PI/2*sqrt(2*dv/J);
	A = M_PI*dv/(2*T);

	//Test if A is over limit
	if(A > Ak)
	{
		A = Ak;
		T = M_PI*dv/(2*A);
	}
}