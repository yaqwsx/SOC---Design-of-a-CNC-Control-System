#include "PathPartOffsetCircle.h"
#include "global.h"
#include "CommunicationEnumerations.h"
#include "GCodeInterpreter.h"

//Constructor via offset
PathPartOffsetCircle::PathPartOffsetCircle(Point beginning, Point ending, Point c, double feed, PathPartType t, size_t line):
	PathPartMovable(beginning, ending, feed, XY, line), center(c), type(t)
{
	assert(type == OFFSET_CIRCLE_CW || type == OFFSET_CIRCLE_CCW);
	assert(feed != 0);
	Vector3D r1(center, ending), r2(center, startingPoint);
	r = r1.Length();
	double len = r2.Length();
	if(fabs(r - len) >= 0.001f)
	{
		throw exception((string("Špatnì zadaný odsazený oblouk na øádku ") + GetLine()).c_str());
	}
}


PathPartOffsetCircle::~PathPartOffsetCircle(void)
{
}


Vector3D PathPartOffsetCircle::GetStartingTangent()
{
	return ComputeTangent(Vector3D(center, GetStartingPoint()));
}
Vector3D PathPartOffsetCircle::GetEndingTangent()
{
	return ComputeTangent(Vector3D(center, GetEndingPoint()));
}

Vector3D PathPartOffsetCircle::GetOffsetStartingTangent()
{
	return GetStartingTangent();
}

Vector3D PathPartOffsetCircle::GetOffsetEndingTangent()
{
	return GetEndingTangent();
}

PathPartSimple PathPartOffsetCircle::ToOffsetSimple()
{
	PathPartSimple ret;
	ret.type = type;
	ret.line = GetLine();
	ret.start = startingPoint;
	ret.end = endingPoint;
	ret.center = center;
	return ret;
}

PathPartSimple PathPartOffsetCircle::ToSimple()
{
	PathPartSimple ret;
	ret.type = type;
	ret.line = GetLine();
	ret.start = Point(0, 0, 0);
	ret.end = Point(0, 0, 0);
	ret.center = Point(0, 0, 0);
	return ret;
}


void PathPartOffsetCircle::ComputeSpeeds(GCodeInterpreter& in)
{
	PathPart* prev = &GetPreviousMovable();
	PathPart* next = &GetNextMovable();

	//Test ending speed of element
	if(next->GetType() == PROGRAM_END)
		endingSpeed = 0;
	else
		endingSpeed = next->GetStartingSpeed();

	//Compute length of the circle
	double r = Vector3D(center, startingPoint).Length();
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

ProcessedData PathPartOffsetCircle::TransformToCommand()
{
	ProcessedData temp;
	double l1 = Vector3D(center, startingPoint).Length();
	double l2 = Vector3D(center, endingPoint).Length();
	assert(fabs(l1-l2) < 0.001);
	temp.line = GetLine();
	temp.data.ID = IDcounter.GetID();
	temp.data.type = COM_AXIS_CIRCLE;
	temp.data.receiver = COM_RECEIVER_AXIS;
	temp.data.circle.C[0] = endingPoint.x;
	temp.data.circle.C[1] = endingPoint.y;
	temp.data.circle.C[2] = endingPoint.z;
	Point add = GetCirclePoint();
	temp.data.circle.B[0] = add.x;
	temp.data.circle.B[1] = add.y;
	temp.data.circle.B[2] = add.z;
	temp.data.circle.v = speed;
	temp.data.circle.v0 = startingSpeed;
	temp.data.circle.vb = endingSpeed;
	return temp;
}

double PathPartOffsetCircle::GetCircleLength()
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
	if(HalfPlane(u1, u1.GetNormalVector()) != HalfPlane(u1, u2))
		ret= 2*M_PI-ret;
	return ret;
}

Point PathPartOffsetCircle::GetCirclePoint()
{
	Point ret;
	Vector3D u(center, startingPoint);
	Vector3D v(center, endingPoint);
	if(Compare3DVectors(u, v, 0.001))
		return startingPoint;
	bool halfPlane = HalfPlane(Get3DVectorProjection(u), Get3DVectorProjection(v));
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
		/*if(type == CIRCLE_CCW)
			normal = -normal;*/
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
	u *= Vector3D(center, endingPoint).Length();
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

Vector2D PathPartOffsetCircle::Get3DVectorProjection(Vector3D a)
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

Vector3D PathPartOffsetCircle::Project2DVectorTo3D(Vector2D a)
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

Vector3D PathPartOffsetCircle::ComputeTangent(Vector3D a)
{
	Vector2D projection = Get3DVectorProjection(a);
	projection = projection.GetNormalVector();
	if(GetType() == CIRCLE_CW)
		projection = -projection;
	projection.Normalize();
	return Project2DVectorTo3D(projection);
}

double PathPartOffsetCircle::GetSFromStartMovement(double v, double o, double r, double Ak, double J, bool save)
{
	double A = 0, T = 0;
	
	GetATFromMovement(v, o, r, Ak, J, A, T);

	if(save)
		Astart = A;

	assert(A*T*T/M_PI+o*T >= 0);

	//Return the length of trajectory
	return A*T*T/M_PI+o*T;
}

double PathPartOffsetCircle::GetSFromEndMovement(double v, double b, double r, double Ak, double J, bool save)
{
	double A = 0, T = 0;
	
	GetATFromMovement(v, b, r, Ak, J, A, T);

	if(save)
		Aend = A;

	assert(-A*T*T/M_PI+v*T >= 0);

	//Return the length of trajectory
	return -A*T*T/M_PI+v*T;
}

void PathPartOffsetCircle::ComputeATLimitedByJerk(double dv, double r, double J, bool sign, double& A, double& T)
{
	//Initial guess
	//A should contain high positive value
	T = -(M_PI*dv)/(2.*A);

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

void PathPartOffsetCircle::GetATFromMovement(double v, double o, double r, double Ak, double J, double& A, double& T)
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
			A = 1.5*Ak;
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
			A = 1.5*Ak;
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