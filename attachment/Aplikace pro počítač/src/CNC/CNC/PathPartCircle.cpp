#define _USE_MATH_DEFINES
#include "PathPartCircle.h"
#include "global.h"
#include "PathPartOffsetCircle.h"
#include "GCodeInterpreter.h"
#include "PathPartOffsetLine.h"
#include "CommunicationEnumerations.h"
#include <cmath>

//Constuctor via offsets
PathPartCircle::PathPartCircle(Point beginning, Point ending, Point c, double s, PathPartType t, PathOffsets o, size_t line):
	PathPartMovable(beginning, ending, s, XY, line), r(0), center(c), type(t), offset(o),
		offsetStartingPoint(beginning), offsetEndingPoint(ending)
{
	assert(type == CIRCLE_CW || type == CIRCLE_CCW);
	//Construct via offsets - done, just check and calculate radius
	Vector3D r1(center, beginning), r2(center, ending);
	r = r1.Length();
	double len = r2.Length();
	if(fabs(r - len) >= 0.001f)
	{
		r = 0;
		throw exception((string("Špatnì zadaný oblouk pomocí IJK na øádku ") + GetLine()).c_str());
	}
}

//Constructor via radius
PathPartCircle::PathPartCircle(Point beginning, Point ending, double radius, WorkingPlane p, double s, PathPartType t, PathOffsets o, size_t line):
	PathPartMovable(beginning, ending, s, p, line), r(0), center(0), type(t), offset(o),
	offsetStartingPoint(beginning), offsetEndingPoint(ending)
{
	assert(type == CIRCLE_CW || type == CIRCLE_CCW);
	//Construct via radius
	switch(p)
	{
		case XY:
			{
				if(beginning.z != ending.z)
					throw exception((string("Oblouk zadaný pomocí R neleží v rovinì, øádek ") + GetLine()).c_str());
				Vector2D result;
				result = GetCircleCenter(Vector2D(beginning.x, beginning.y), Vector2D(ending.x, ending.y));
				center.x = result.x;
				center.y = result.y;
				center.z = ending.z;
			}
			break;
		case XZ:
			{
				if(beginning.y != ending.y)
					throw exception((string("Oblouk zadaný pomocí R neleží v rovinì, øádek ") + GetLine()).c_str());
				Vector2D result;
				result = GetCircleCenter(Vector2D(beginning.x, beginning.z), Vector2D(ending.x, ending.z));
				center.x = result.x;
				center.z = result.y;
				center.y = ending.y;
			}
			break;
		case YZ:
			{
				if(beginning.x != ending.x)
					throw exception((string("Oblouk zadaný pomocí R neleží v rovinì, øádek ") + GetLine()).c_str());
				Vector2D result;
				result = GetCircleCenter(Vector2D(beginning.y, beginning.z), Vector2D(ending.y, ending.z));
				center.y = result.x;
				center.z = result.y;
				center.x = ending.x;
			}
			break;
	}
}

PathPartCircle::~PathPartCircle(void)
{
}


Vector3D PathPartCircle::GetStartingTangent()
{
	return ComputeTangent(Vector3D(center, GetStartingPoint()));
}

Vector3D PathPartCircle::GetOffsetStartingTangent()
{
	return ComputeTangent(Vector3D(center, GetOffsetStartingPoint()));
}

Vector3D PathPartCircle::GetEndingTangent()
{
	return ComputeTangent(Vector3D(center, GetEndingPoint()));
}

Vector3D PathPartCircle::GetOffsetEndingTangent()
{
	return ComputeTangent(Vector3D(center, GetOffsetEndingPoint()));
}

PathPartSimple PathPartCircle::ToSimple()
{
	PathPartSimple ret;
	ret.type = type;
	ret.line = GetLine();
	ret.start = startingPoint;
	ret.end = endingPoint;
	ret.center = center;
	return ret;
}

Vector2D PathPartCircle::GetCircleCenter(Vector2D A, Vector2D B)
{
	Vector2D half = (A+B)/2;
	Vector2D part = (B-A);
	Vector2D normal = part.GetNormalVector(type==CIRCLE_CW);
	normal.Normalize();
	//Get length
	double length = sqrt(r*r - part.Length()*part.Length()/4);
	normal *= length;
	return half+normal;
}

void PathPartCircle::ProcessUnchangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in)
{
	if(plane != XY)
	{
		throw exception("Nelze zpracovat offset oblouku v jiné, než XY rovinì.");
	}
	if(GetStartingPoint() != GetOffsetStartingPoint())
	{
		//Allready processed, skip
		return;
	}
	
	//Process corner
	Vector2D tangent = GetProjection(GetStartingTangent());
	Vector2D norm = tangent.GetNormalVector();
	Vector2D prevTangent = GetProjection(prev->GetEndingTangent());

	if(Compare2DVectors(tangent, prevTangent, tangentTolerance))
	{
		//Parts are tangential, simple offset
		norm *= GetOffset().GetToolComp();
		Point start = GetStartingPoint();
		start.x += norm.x;
		start.y += norm.y;
		SetOffsetStartingPoint(start);
		return;
	}
	if(prevTangent == Vector2D(0, 0))
	{
		//The previous line is parallel to the Z-axis
		//Find the previous line
		PathPart* another;
		while(prevTangent == Vector2D(0, 0))
		{
			another = &prev->GetPreviousMovable();
			prevTangent = GetProjection(another->GetStartingTangent());
		}
		prev = another;
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
	if(!corner)
	{
		//Outer corner
		norm *= GetOffset().GetToolComp();
		Point starting = GetStartingPoint();
		starting.x += norm.x;
		starting.y += norm.y;
		SetOffsetStartingPoint(starting);
		//OuterCorner, we need to add transition circle
		PathPartType type;
		if(prev->GetOffset().GetToolComp() > 0)
			type = OFFSET_CIRCLE_CW;
		else
			type = OFFSET_CIRCLE_CCW;
		Point prevOffEnd = prev->GetOffsetEndingPoint();
		prevOffEnd.z = starting.z;
		in.AddPathPart(unique_ptr<PathPart>(new 
			PathPartOffsetCircle(prevOffEnd, GetOffsetStartingPoint(), GetStartingPoint(), GetDesignedSpeed(), type, GetLine())),
			item);
	}
	else
	{
		//InnerCorner
		//It's just start - use previously defined point
		Point offEnd = prev->GetOffsetEndingPoint();
		Point start = GetStartingPoint();
		start.x = offEnd.x;
		start.y = offEnd.y;
		SetOffsetStartingPoint(start);
	}

}

void PathPartCircle::ProcessChangedOffsetStart(PathPart* prev, PathPart* next, GCodeInterpreter& in)
{
	if(plane != XY)
	{
		throw exception("Nelze provést korekci nástroje na oblouku v jiné než XY rovinì");
	}

	if(GetCircleLength() < (M_PI/2.0))
	{
		throw exception("Nelze provést korekci nástroje na oblouku kratším než 90°");
	}

	if(prev->GetType() != PROGRAM_BEGIN)
	{
		Vector2D prevTangent = GetProjection(prev->GetStartingTangent());
		while(prev->GetType() != PROGRAM_BEGIN && prevTangent == Vector2D())
		{
			prev = &GetPreviousMovable();
			prevTangent = GetProjection(prev->GetStartingTangent());
		}
	}
	double correction = GetOffset().GetToolComp();
	if(prev->GetType() != PROGRAM_BEGIN)
	{
		correction -= prev->GetOffset().GetToolComp();
	}

	if(type == CIRCLE_CW)
	{
		if(correction > 0)
			ProcessStartingToBigger(prev, in);
		else
			ProcessStartingToSmaller(prev, in);
	}
	else
	{
		if(correction < 0)
			ProcessStartingToBigger(prev, in);
		else
			ProcessStartingToSmaller(prev, in);
	}
}

void PathPartCircle::ProcessNoOffsetStart(PathPart* prev)
{

}

void PathPartCircle::ProcessUnchangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in)
{
	if(plane != XY)
	{
		throw exception("Nelze zpracovat offset oblouku v jiné, než XY rovinì.");
	}
	if(next->GetType() == PROGRAM_END)
	{
		//This is the last element
		Vector2D of = GetProjection(GetEndingTangent()).GetNormalVector()*GetOffset().GetToolComp();
		Point end = GetEndingPoint();
		end.x += of.x;
		end.y += of.y;
		SetOffsetEndingPoint(end);
		return;
	}
	Vector2D tangent = GetProjection(GetEndingTangent());
	Vector2D nextTangent = GetProjection(next->GetStartingTangent());
	if(Compare2DVectors(tangent, nextTangent, tangentTolerance))
	{
		//Paths are tangential, simple offset
		Vector2D normal = tangent.GetNormalVector()*GetOffset().GetToolComp();
		Point end = GetEndingPoint();
		end.x += normal.x;
		end.y += normal.y;
		SetOffsetEndingPoint(end);
		return;
	}
	if(nextTangent == Vector2D(0, 0))
	{
		//The next part is parallel to Z axis, compare with the next one
		PathPart* another = &next->GetNextMovable();
		while(another->GetType() != PROGRAM_END &&
			GetProjection(another->GetStartingTangent()) != Vector2D(0, 0))
			another = &another->GetNextMovable();
		ProcessUnchangedOffsetEndMod(another, prev, in);
		return;
	}
	ProcessUnchangedOffsetEndMod(next, prev, in);
}

void PathPartCircle::ProcessChangedOffsetEnd(PathPart* next, PathPart* prev, GCodeInterpreter& in)
{
	ProcessUnchangedOffsetEnd(next, prev, in);
}

void PathPartCircle::ProcessNoOffsetEnd(PathPart* next)
{

}

bool PathPartCircle::IsOffset()
{
	return startingPoint != offsetStartingPoint;
}

PathPartSimple PathPartCircle::ToOffsetSimple()
{
	PathPartSimple ret;
	ret.type = type;
	ret.line = GetLine();
	ret.start = offsetStartingPoint;
	ret.end = offsetEndingPoint;
	ret.center = center;
	double r1 = Vector3D(center, offsetEndingPoint).Length();
	double r2 = Vector3D(center, offsetStartingPoint).Length();
	return ret;
}

void PathPartCircle::ProcessUnchangedOffsetEndMod(PathPart* next, PathPart* prev, GCodeInterpreter& in)
{
	Vector2D tangent = GetProjection(GetEndingTangent());
	Vector2D norm = tangent.GetNormalVector();
	if(next->GetType() == PROGRAM_END)
	{
		norm *= GetOffset().GetToolComp();
		Point end;
		end = GetEndingPoint();
		end.x += norm.x;
		end.y += norm.y;
		SetOffsetEndingPoint(end);
		return;
	}
	Vector2D nextTangent = GetProjection(next->GetStartingTangent());
	PathPart* another = next;
	while(nextTangent == Vector2D(0, 0))
	{
		//It is a line, which is parallel with Z axis
		//Process next one
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
		//Inner corner
		switch(another->GetType())
		{
		case LINE_NORMAL:
		case LINE_RAPID:
			ProcessUnchangedInnerCornerLine(another);
			break;
		case CIRCLE_CW:
		case CIRCLE_CCW:
			ProcessUnchangedInnerCornerCircle(another);
			break;
		default:
			throw exception("BUG: Neimplementovaný pøechod");
			break;
		}
	}
	else
	{
		switch(another->GetType())
		{
		case LINE_NORMAL:
		case LINE_RAPID:
			ProcessUnchangedOuterCornerLine(another);
			break;
		case CIRCLE_CW:
		case CIRCLE_CCW:
			ProcessUnchangedOuterCornerCircle(another);
			break;
		default:
			throw exception("BUG: Neimplementovaný pøechod");
			break;
		}
	}
}

void PathPartCircle::ProcessUnchangedInnerCornerLine(PathPart* next)
{
	//We need to find intersection circle-line
	Vector2D tangent = GetProjection(next->GetStartingTangent());
	Vector2D normal = tangent.GetNormalVector()*next->GetOffset().GetToolComp();
	Point start = next->GetStartingPoint();
	double k1 = normal.x + start.x-center.x;
	double k2 = normal.y + start.y-center.y;
	double b = 2*(tangent.x*k1+tangent.y*k2);
	double rr;
	if(type == CIRCLE_CCW)
		rr = r - GetOffset().GetToolComp();
	else
		rr = r + GetOffset().GetToolComp();
	double a = (tangent.x*tangent.x+tangent.y*tangent.y);
		double D = b*b-4*a*(k1*k1+k2*k2-rr*rr);
	if(D < 0)
		throw exception("Nelze nalézt spoleèný prùseèík pøi vytvoøení odsazení. Korekce je pøíliš velká.");
	double t = (-b-sqrt(D))/(2*a);
	Vector2D res = tangent*t;
	res.x += start.x + normal.x;
	res.y += start.y + normal.y;

	Point end = GetEndingPoint();
	end.x = res.x;
	end.y = res.y;

	SetOffsetEndingPoint(end);
}

void PathPartCircle::ProcessUnchangedInnerCornerCircle(PathPart* next)
{
	//We need to find intersection circle-circle
	//Everything is positioned relatively to the center of this circle
	PathPartCircle* n = dynamic_cast<PathPartCircle*>(next);
	double r1;
	if(type == CIRCLE_CCW)
		r1 = r - GetOffset().GetToolComp();
	else
		r1 = r + GetOffset().GetToolComp();
	double r2;
	if(n->type == CIRCLE_CCW)
		r2 = n->r - n->GetOffset().GetToolComp();
	else
		r2 = n->r + n->GetOffset().GetToolComp();
	double x0 = n->center.x - center.x;
	double y0 = n->center.y - center.y;
	double temp = -r2*r2+x0*x0+y0*y0;
	double D=-y0*y0*(r1*r1*r1*r1+temp*temp-2*r1*r1*(temp+2*r2*r2));
	if(D < 0)
		throw exception("BUG: Kružnice nemají prùseèík");
	D = sqrt(D);
	double x = x0*(r1*r1-r2*r2+x0*x0+y0*y0);
	double xf = 2*(x0*x0+y0*y0);
	double y = y0*y0*(r1*r1-r2*r2+x0*x0+y0*y0);
	double yf = 2*y0*(x0*x0+y0*y0);
	double x1 = (x-D)/xf+center.x;
	double x2 = (x+D)/xf+center.x;
	double y1 = (y+x0*D)/yf+center.y;
	double y2 = (y-x0*D)/yf+center.y;
	//Find the minimum value
	Point end = GetEndingPoint();
	Vector2D a(end, Point(x1, y1, 0));
	Vector2D b(end, Point(x2, y2, 0));
	if(a.Length() < b.Length())
	{
		//First root is near
		end.x = x1;
		end.y = y1;
	}
	else
	{
		//Second root is near
		end.x = x2;
		end.y = y2;
	}

	SetOffsetEndingPoint(end);
}

void PathPartCircle::ProcessUnchangedOuterCornerLine(PathPart* next)
{
	//This is an outter corner - make a simple offset
	Vector2D normal = GetProjection(GetEndingTangent()).GetNormalVector();
	normal *= GetOffset().GetToolComp();
	Point ending = GetEndingPoint();
	ending.x += normal.x;
	ending.y += normal.y;
	SetOffsetEndingPoint(ending);
}

void PathPartCircle::ProcessUnchangedOuterCornerCircle(PathPart* next)
{
	ProcessUnchangedOuterCornerLine(next);
}

double PathPartCircle::GetCircleLength()
{
	double ret;
	Vector3D v1(center, offsetEndingPoint);
	v1.Normalize();
	Vector3D v2(center, offsetStartingPoint);
	v2.Normalize();
	ret = acos(DotProduct(v1, v2));
	if(v1 == -v2)
	{
		//The vector are opposite
		return M_PI;
	}
	Vector2D u1, u2;
	switch(plane)
	{
		case XY:
			u1.x = v1.x;
			u1.y = v1.y;
			u2.x = v2.x;
			u2.y = v2.y;
			break;
		case XZ:
			u1.x = v1.x;
			u1.y = v1.z;
			u2.x = v2.x;
			u2.y = v2.z;
			break;
		case YZ:
			u1.x = v1.z;
			u1.y = v1.y;
			u2.x = v2.z;
			u2.y = v2.y;
			break;
	}
	if(type == CIRCLE_CW)
	{
		if(HalfPlane(u1, u1.GetNormalVector()) != HalfPlane(u1, u2))
			ret= 2*M_PI-ret;
	}
	else
		if(HalfPlane(u1, u1.GetNormalVector()) == HalfPlane(u1, u2))
			ret= 2*M_PI-ret;

	return ret;
}

double PathPartCircle::GetOriginalCircleLength()
{
	double ret;
	Vector3D v1(center, endingPoint);
	v1.Normalize();
	Vector3D v2(center, startingPoint);
	v2.Normalize();
	ret = acos(DotProduct(v1, v2));
	if(v1 == -v2)
	{
		//The vector are opposite
		return M_PI;
	}
	Vector2D u1, u2;
	switch(plane)
	{
		case XY:
			u1.x = v1.x;
			u1.y = v1.y;
			u2.x = v2.x;
			u2.y = v2.y;
			break;
		case XZ:
			u1.x = v1.x;
			u1.y = v1.z;
			u2.x = v2.x;
			u2.y = v2.z;
			break;
		case YZ:
			u1.x = v1.z;
			u1.y = v1.y;
			u2.x = v2.z;
			u2.y = v2.y;
			break;
	}
	if(GetType() == CIRCLE_CW)
	{
		if(HalfPlane(u1, u1.GetNormalVector()) != HalfPlane(u1, u2))
			ret= 2*M_PI-ret;
	}
	else
		if(HalfPlane(u1, u1.GetNormalVector()) == HalfPlane(u1, u2))
			ret= 2*M_PI-ret;

	return ret;
}

void PathPartCircle::ProcessStartingToBigger(PathPart* prev, GCodeInterpreter& in)
{
	Vector2D startingTangent = GetProjection(GetStartingTangent());
	Point start;
	if(prev->GetType() == PROGRAM_BEGIN)
		start = GetStartingPoint();
	else
		start = prev->GetOffsetEndingPoint();
	double correction = GetOffset().GetToolComp();
	if(prev->GetType() != PROGRAM_BEGIN)
	{
		correction -= prev->GetOffset().GetToolComp();
	}
	correction = abs(correction);
	double radius = Vector2D(start.x - center.x, start.y - center.y).Length();
	Point lineEnd = start;
	lineEnd.x += startingTangent.x*correction;
	lineEnd.y += startingTangent.y*correction;
	Point offCenter = center;
	offCenter.x += startingTangent.x*correction;
	offCenter.y += startingTangent.y*correction;
	Point circleStart = center;
	circleStart.x += startingTangent.x*(correction+radius);
	circleStart.y += startingTangent.y*(correction+radius);

	PathPartType offType = OFFSET_CIRCLE_CW;
	if(type == CIRCLE_CCW)
		offType = OFFSET_CIRCLE_CCW;


	SetOffsetStartingPoint(circleStart);
	in.AddPathPart(unique_ptr<PathPart>(new 
			PathPartOffsetLine(start, lineEnd, GetSpeed(), XY, GetLine())),
			item);
	in.AddPathPart(unique_ptr<PathPart>(new 
			PathPartOffsetCircle(lineEnd, circleStart, offCenter, GetDesignedSpeed(), offType, GetLine())),
			item);
}

void PathPartCircle::ProcessStartingToSmaller(PathPart* prev, GCodeInterpreter& in)
{
	Vector2D startingTangent = GetProjection(GetStartingTangent());
	Point start;
	if(prev->GetType() == PROGRAM_BEGIN)
		start = GetStartingPoint();
	else
		start = prev->GetOffsetEndingPoint();
	double correction = GetOffset().GetToolComp();
	if(prev->GetType() != PROGRAM_BEGIN)
	{
		correction -= prev->GetOffset().GetToolComp();
	}
	correction = abs(correction);
	Vector2D norm(-start.x + center.x, -start.y + center.y);
	double radius = norm.Length();
	norm.Normalize();
	Point circleEnd = start;
	circleEnd.x += (norm.x+startingTangent.x)*(radius-correction);
	circleEnd.y += (norm.y+startingTangent.y)*(radius-correction);
	Point lineEnd = center;
	lineEnd.x += startingTangent.x*(radius-correction);
	lineEnd.y += startingTangent.y*(radius-correction);
	Point offCenter = start;
	offCenter.x += norm.x*(radius-correction);
	offCenter.y += norm.y*(radius-correction);

	PathPartType offType = OFFSET_CIRCLE_CW;
	if(type == CIRCLE_CCW)
		offType = OFFSET_CIRCLE_CCW;

	SetOffsetStartingPoint(lineEnd);
	in.AddPathPart(unique_ptr<PathPart>(new 
			PathPartOffsetCircle(start, circleEnd, offCenter, GetDesignedSpeed(), offType, GetLine())),
			item);
	in.AddPathPart(unique_ptr<PathPart>(new 
			PathPartOffsetLine(circleEnd, lineEnd, GetSpeed(), XY, GetLine())),
			item);
}


void PathPartCircle::ComputeSpeeds(GCodeInterpreter& in)
{
	PathPart* prev = &GetPreviousMovable();
	PathPart* next = &GetNextMovable();

	//Test ending speed of element
	if(next->GetType() == PROGRAM_END)
		endingSpeed = 0;
	else
		endingSpeed = next->GetStartingSpeed();

	//Compute length of the circle
	double r = Vector3D(center, offsetStartingPoint).Length();
	double S = r*GetCircleLength();
	
	//validate speed
	double Akr = sqrt(in.GetAcceleration()*r);
	if(designedSpeed > Akr)
	{
		designedSpeed = Akr;
	}
	if(startingSpeed > designedSpeed)
		startingSpeed = designedSpeed;
	if(endingSpeed > designedSpeed)
	{
		endingSpeed = designedSpeed;
		next->SetStartingSpeed(endingSpeed);
		next->ComputeSpeeds(in);
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
			double S2 = GetSFromEndMovement(startingSpeed, endingSpeed, r,
				in.GetAcceleration(), in.GetJerk(), true);
			if(S2 > S)
			{
				//It's impossible to brake;
				//Find new speed using bisection
				double v = startingSpeed;
				double dv = startingSpeed - endingSpeed;
				bool last = true;
				for(int i = 0; i != numOfIterations && (fabs(S - S2) > 0.01 || S2 > S); i++)
				{
					dv /= 2;
					if(last)
						v -= dv;
					else
						v += dv;
					S2 = GetSFromEndMovement(v, endingSpeed, r,
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
	double S1 = GetSFromStartMovement(designedSpeed, startingSpeed, r,
				in.GetAcceleration(), in.GetJerk(), true);
	double S2 = GetSFromEndMovement(designedSpeed, endingSpeed, r,
				in.GetAcceleration(), in.GetJerk(), true);

	if(S2 < 0.001 && S1 > S)
	{
		//We are not able to accelerate
		//Find new speed using bisection
		double v = designedSpeed;
		double dv = designedSpeed - startingSpeed;
		bool last = true;
		for(int i = 0; i != numOfIterations && (fabs(S - S1) > 0.01 || S1 > S); i++)
		{
			dv /= 2;
			if(last)
				v -= dv;
			else
				v += dv;
			S1 = GetSFromStartMovement(v, startingSpeed, r,
				in.GetAcceleration(), in.GetJerk(), true);
			if(S1 > S)
				last = true;
			else
				last = false;
		}
		speed = endingSpeed =  designedSpeed = v;
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
		vector<double> hist;
		for(int i = 0; i != numOfIterations && (fabs(S - S1 - S2) > 0.01 || S1 + S2 > S); i++)
		{
			dv /= 2;
			if(last)
				v -= dv;
			else
				v += dv;
			S1 = GetSFromStartMovement(v, startingSpeed, r,
				in.GetAcceleration(), in.GetJerk(), true);
			S2 = GetSFromEndMovement(v, endingSpeed, r,
				in.GetAcceleration(), in.GetJerk(), true);
			if(S1 + S2 > S)
				last = true;
			else
				last = false;
			hist.push_back(v);
		}
		speed = designedSpeed = v;
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

ProcessedData PathPartCircle::TransformToCommand()
{
	ProcessedData temp;
	temp.line = GetLine();
	temp.data.ID = IDcounter.GetID();
	temp.data.type = COM_AXIS_CIRCLE;
	temp.data.receiver = COM_RECEIVER_AXIS;
	temp.data.circle.C[0] = offsetEndingPoint.x;
	temp.data.circle.C[1] = offsetEndingPoint.y;
	temp.data.circle.C[2] = offsetEndingPoint.z;
	temp.data.circle.S[0] = center.x;
	temp.data.circle.S[1] = center.y;
	temp.data.circle.S[2] = center.z;
	Point add = GetCirclePoint();
	temp.data.circle.B[0] = add.x;
	temp.data.circle.B[1] = add.y;
	temp.data.circle.B[2] = add.z;
	temp.data.circle.v = speed*.98;
	temp.data.circle.v0 = startingSpeed*.98;
	temp.data.circle.vb = endingSpeed*.98;
	temp.data.circle.As = Astart;
	temp.data.circle.Ab = Aend;
	temp.data.circle.alpha = GetCircleLength();
	return temp;
}


Point PathPartCircle::GetCirclePoint()
{
	Point ret;
	Vector3D u(center, offsetStartingPoint);
	Vector3D v(center, offsetEndingPoint);
	if(Compare3DVectors(u, v, 0.001))
		return offsetStartingPoint;
	bool halfPlane = HalfPlane(Get3DVectorProjection(u), Get3DVectorProjection(v));
	assert(fabs(u.Length() - v.Length()) < 0.001);
	if(Compare3DVectors(u, -v, 0.001))
	{
		Vector2D u2;
		switch(plane)
		{
			case XY:
				u2.x = v.x;
				u2.y = v.y;
				break;
			case XZ:
				u2.x = v.x;
				u2.y = v.z;
				break;
			case YZ:
				u2.x = v.z;
				u2.y = v.y;
				break;
		}
		Vector2D normal = u2.GetNormalVector();
		if(type == CIRCLE_CCW)
			normal = -normal;
		switch(plane)
		{
			case XY:
				ret.x = normal.x;
				ret.y = normal.y;
				ret.z = u.z;
				break;
			case XZ:
				ret.x = normal.x;
				ret.z = normal.y;
				ret.y = u.y;
				break;
			case YZ:
				ret.z = normal.x;
				ret.y = normal.y;
				ret.x = u.x;
				break;
		}
		return ret+center;
	}
	u += v;
	u.Normalize();
	u *= v.Length();
	//Check direction
	if(halfPlane)
	{
		if(GetType() == CIRCLE_CW)
			u *= -1;
	}
	else
	{
		if(GetType() == CIRCLE_CCW)
			u *= -1;
	}
	//
	ret.x = center.x + u.x;
	ret.y = center.y + u.y;
	ret.z = center.z + u.z;
	return ret;
}

void PathPartCircle::SetOffsetEndingPoint(Point& p)
{
	offsetEndingPoint = p;
	assert(fabs(Vector3D(center, offsetEndingPoint).Length() - Vector3D(center, offsetStartingPoint).Length()) < 0.001);
}

Vector2D PathPartCircle::Get3DVectorProjection(Vector3D a)
{
	Vector2D ret;
	switch(plane)
	{
		case XY:
			ret.x = a.x;
			ret.y = a.y;
			break;
		case XZ:
			ret.x = a.x;
			ret.y = a.z;
			break;
		case YZ:
			ret.x = a.y;
			ret.y = a.z;
			break;
	}
	return ret;
}

Vector3D PathPartCircle::Project2DVectorTo3D(Vector2D a)
{
	Vector3D b(0);
	switch(plane)
	{
		case XY:
			b.x = a.x;
			b.y = a.y;
			break;
		case XZ:
			b.x = a.x;
			b.z = a.y;
			break;
		case YZ:
			b.y = a.x;
			b.z = a.y;
			break;
	}
	return b;
}

Vector3D PathPartCircle::ComputeTangent(Vector3D a)
{
	Vector2D projection = Get3DVectorProjection(a);
	projection = projection.GetNormalVector();
	if(GetType() == CIRCLE_CW)
		projection = -projection;
	projection.Normalize();
	return Project2DVectorTo3D(projection);
}

double PathPartCircle::GetSFromStartMovement(double v, double o, double r, double Ak, double J, bool save)
{
	double A = 0, T = 0;
	
	GetATFromMovement(v, o, r, Ak, J, A, T);

	if(save)
		Astart = A;

	assert(A*T*T/M_PI+o*T >= 0);

	//Return the length of trajectory
	return A*T*T/M_PI+o*T;
}

double PathPartCircle::GetSFromEndMovement(double v, double b, double r, double Ak, double J, bool save)
{
	double A = 0, T = 0;
	
	GetATFromMovement(v, b, r, Ak, J, A, T);

	if(save)
		Aend = A;

	assert(-A*T*T/M_PI+v*T >= 0);

	//Return the length of trajectory
	return -A*T*T/M_PI+v*T;
}

void PathPartCircle::ComputeATLimitedByJerk(double dv, double r, double J, bool sign, double& A, double& T)
{
	//Initial guess
	//A should contain high positive value
	T = (M_PI*dv)/(2.*A);

	vector<pair<double, double>> history;
	//difference for A and T
	double dA = 1, dT = 1;
	
	for(size_t i = 0; i != numOfIterations && dA*dA + dT*dT > precision*precision; i++)
	{
		history.push_back(pair<double, double>(A, T));

		//Matrix {{a, b},{c, d}} with partial derivates
		double am = -1;
		double bm = -(dv*M_PI)/(2.*T*T);
		double cm = (-2*A*pow(M_PI,8)*pow(r,4) + 6*(7 + 5*M_SQRT2)*pow(A,3)*pow(M_PI,4)*pow(r,2)*pow(T,4) + (478 + 338*M_SQRT2)*pow(A,5)*pow(T,8))/
					(pow(r,2)*sqrt((2*pow(A,2)*pow(M_PI,4)*pow(r,2) + (17 + 12*M_SQRT2)*pow(A,4)*pow(T,4))/pow(r,2))*
					(2*pow(M_PI,5)*pow(r,2)*T + (17 + 12*M_SQRT2)*pow(A,2)*M_PI*pow(T,5)));
		double dm = (2*pow(A,2)*pow(M_PI,8)*pow(r,4) + (93 + 66*M_SQRT2)*pow(A,4)*pow(M_PI,4)*pow(r,2)*pow(T,4) + (239 + 169*M_SQRT2)*pow(A,6)*pow(T,8))/
					(pow(r,2)*pow(T,2)*sqrt((2*pow(A,2)*pow(M_PI,4)*pow(r,2) + (17 + 12*M_SQRT2)*pow(A,4)*pow(T,4))/pow(r,2))*
					(2*pow(M_PI,5)*pow(r,2) + (17 + 12*M_SQRT2)*pow(A,2)*M_PI*pow(T,4)));

		//Inverse matrix {{Am, Bm},{Cm, Dm}}
		double Det = am*dm-bm*cm;
		//For debugging
		assert(Det==Det);
		double Am = dm/Det;
		double Bm = -bm/Det;
		double Cm = -cm/Det;
		double Dm = am/Det;

		//Function value
		double Ah = -A + (dv*M_PI)/(2.*T);
		double Th = (-(pow(A,2)*pow(M_PI,4)*pow(r,2)) + (7 + 5*M_SQRT2)*pow(A,4)*pow(T,4))/
					(M_PI*pow(r,2)*T*sqrt((2*pow(A,2)*pow(M_PI,4)*pow(r,2) + (17 + 12*M_SQRT2)*pow(A,4)*pow(T,4))/pow(r,2)));
		if(sign)
			Th -= J;
		else
			Th += J;

		dA = (Ah*Am+Th*Bm);
		dT = (Ah*Cm+Th*Dm);
		
		A = A - dA;
		T = T - dT;
	}

#ifdef _DEBUG
	double Ah = -A + (dv*M_PI)/(2.*T);
	double Th = (-(pow(A,2)*pow(M_PI,4)*pow(r,2)) + (7 + 5*M_SQRT2)*pow(A,4)*pow(T,4))/
				(M_PI*pow(r,2)*T*sqrt((2*pow(A,2)*pow(M_PI,4)*pow(r,2) + (17 + 12*M_SQRT2)*pow(A,4)*pow(T,4))/pow(r,2)));
	if(sign)
		Th -= J;
	else
		Th += J;
#endif
}

void PathPartCircle::GetATFromMovement(double v, double o, double r, double Ak, double J, double& A, double& T)
{
	//Test if the given speed isn't out of limit
	double Akr = sqrt(Ak*r);
	if(v > Akr)
	{
		//It's impossible to perform this kind of motion
		assert(false);//Shouldn't occur
	}
	else if(v == Akr)
	{
		//Special case, max A is in T
		T = M_PI/2*sqrt(2*(v-o)/J);
		if(T == 0)
		{
			//There's no difference between speeds
			A = Ak;
			return;
		}
		A = M_PI*(v-o)/(2*T);
		assert(A==A);
		//Test, if the maximal jerk is valid:
		double jerkMax;//value of jerk in 3/4T
		jerkMax = (-(pow(A,2)*pow(M_PI,4)*pow(r,2)) + (7 + 5*M_SQRT2)*pow(A,4)*pow(T,4))/
				(M_PI*pow(r,2)*T*sqrt((2*pow(A,2)*pow(M_PI,4)*pow(r,2) + 
				(17 + 12*M_SQRT2)*pow(A,4)*pow(T,4))/pow(r,2)));
		if(abs(jerkMax) > J)
		{
			//The maximal jerk is in t = 3T/4 calculate new value
			A = 10*Ak;
			ComputeATLimitedByJerk(v-o, r, J, true, A, T);
			assert(A==A);
		}
	}
	else
	{
		//There's no special case
		//Calculate the values using jerk

		//First, suppose max jerk occurs in t = 0
		T = M_PI*sqrt((v-o)/(2*J));
		A = sqrt(J*(v-o)/2);
		assert(A==A);
		//Test, if the maximal jerk is valid:
		double jerkMax;//value of jerk in 3/4T
		jerkMax = (-(pow(A,2)*pow(M_PI,4)*pow(r,2)) + (7 + 5*M_SQRT2)*pow(A,4)*pow(T,4))/
				(M_PI*pow(r,2)*T*sqrt((2*pow(A,2)*pow(M_PI,4)*pow(r,2) + 
				(17 + 12*M_SQRT2)*pow(A,4)*pow(T,4))/pow(r,2)));
		if(abs(jerkMax) > J)
		{
			//The maximal jerk is in t = 3T/4 calculate new value
			A = 10*Ak;
			ComputeATLimitedByJerk(v-o, r, J, false, A, T);
			assert(A==A);
		}

		//Check if the maximal acceleration wasn't exceeded
		double AkMax = (A*sqrt(pow(M_PI,4)*pow(r,2) + pow(A,2)*pow(T,4)))/(pow(M_PI,2)*r);
		if(AkMax > Ak)
		{
			//Limit the motion by acceleration, not by jerk
			A = sqrt(16*Ak*Ak*r*r-pow(o-v, 4))/(4*r);
			assert(A==A);
			T = M_PI*(v-o)/(2*A);
		}
	}
}