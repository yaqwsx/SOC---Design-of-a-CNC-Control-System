#include "PathPartOffsetLine.h"
#include "CommunicationEnumerations.h"
#include "GCodeInterpreter.h"


PathPartOffsetLine::PathPartOffsetLine(Point beginning, Point end, double f, WorkingPlane p, size_t line):
	PathPartMovable(beginning, end, f, p, line)
{
}


PathPartOffsetLine::~PathPartOffsetLine(void)
{
}


Vector3D PathPartOffsetLine::GetStartingTangent()
{
	Vector3D ret(startingPoint, endingPoint);
	ret.Normalize();
	return ret;
}


Vector3D PathPartOffsetLine::GetEndingTangent()
{
	return GetStartingTangent();
}

Vector3D PathPartOffsetLine::GetOffsetStartingTangent()
{
	return GetStartingTangent();
}


Vector3D PathPartOffsetLine::GetOffsetEndingTangent()
{
	return GetOffsetStartingTangent();
}

PathPartSimple PathPartOffsetLine::ToSimple()
{
	PathPartSimple ret;
	ret.type = LINE_OFFSET;
	ret.line = GetLine();
	ret.start = startingPoint;
	ret.end = endingPoint;
	return ret;
}

PathPartSimple PathPartOffsetLine::ToOffsetSimple()
{
	PathPartSimple ret;
	ret.type = LINE_NORMAL;
	ret.line = GetLine();
	ret.start = startingPoint;
	ret.end = endingPoint;
	return ret;
}

void PathPartOffsetLine::ComputeSpeeds(GCodeInterpreter& in)
{
	PathPart* prev = &GetPreviousMovable();
	PathPart* next = &GetNextMovable();

	//Test ending speed of element
	if(next->GetType() == PROGRAM_END)
		endingSpeed = 0;
	else
		endingSpeed = next->GetStartingSpeed();

	//Compute length of the circle
	double S = Vector3D(startingPoint, endingPoint).Length();

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


ProcessedData PathPartOffsetLine::TransformToCommand()
{
	ProcessedData temp;
	temp.line = GetLine();
	temp.data.ID = IDcounter.GetID();
	temp.data.type = COM_AXIS_LINE;
	temp.data.receiver = COM_RECEIVER_AXIS;
	temp.data.line.axes[0] = endingPoint.x;
	temp.data.line.axes[1] = endingPoint.y;
	temp.data.line.axes[2] = endingPoint.z;
	temp.data.line.v = speed;
	temp.data.line.v0 = startingSpeed;
	temp.data.line.vb = endingSpeed;
	temp.data.line.As = Astart;
	temp.data.line.Ab = Aend;
	return temp;
}

double PathPartOffsetLine::GetSFromStartMovement(double v, double o, double Ak, double J, bool save)
{
	double A = 0, T = 0;
	
	GetATFromMovement(v, o, Ak, J, A, T);

	if(save)
		Astart = A;

	assert(A*T*T/M_PI+o*T >= 0);

	//Return the length of trajectory
	return A*T*T/M_PI+o*T;
}

double PathPartOffsetLine::GetSFromEndMovement(double v, double b, double Ak, double J, bool save)
{
	double A = 0, T = 0;
	
	GetATFromMovement(v, b, Ak, J, A, T);

	if(save)
		Aend = A;

	assert(-A*T*T/M_PI+v*T >= 0);

	//Return the length of trajectory
	return -A*T*T/M_PI+v*T;
}

void PathPartOffsetLine::GetATFromMovement(double v, double v2, double Ak, double J, double& A, double& T)
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