#include "Geometry.h"

Point operator*(Point p, double& f)
{
	p *= f;
	return p;
}

Point::Point(const wxPoint& p)
{
	x = p.x;
	y = p.y;
};

Point& Point::operator=(const wxPoint& p)
{
	x = p.x;
	y = p.y;
	return *this;
};

Point& Point::operator+=(const Point& p)
{
	x +=p.x; 
	y+=p.y; 
	z+=p.z; 
	a+=p.a; 
	b+=p.b; 
	return *this;
};

Point& Point::operator-=(const Point& p)
{
	x -=p.x; 
	y-=p.y; 
	z-=p.z; 
	a-=p.a; 
	b-=p.b; 
	return *this;
};

Point& Point::operator*=(double coefficient)
{
	x *= coefficient;
	y *= coefficient;
	z *= coefficient;
	return *this;
}

Point Point::operator+(const Point& p)
{
	Point r; 
	r.x = x+p.x; 
	r.y=y+p.y; 
	r.z=z+p.z; 
	r.a=a+p.a;
	r.b=b+p.b; 
	return r;
};

Point Point::operator-(const Point& p)
{
	Point r;
	r.x = x-p.x;
	r.y=y-p.y;
	r.z=z-p.z;
	r.a=a-p.a;
	r.b=b-p.b;
	return r;
};

Point& Point::operator/=(const double& f)
{
	x/=f;
	y/=f;
	z/=f;
	a/=f;
	b/=f;
	return *this;
};

Point Point::operator/(const double& f)
{
	Point ret = *this;
	ret/=f;
	return ret;
}

Vector2D& Vector2D::operator*=(const double& f)
{
	x *= f;
	y *= f;
	return *this;
};
	

Vector2D Vector2D::operator*(const double& f)
{
	Vector2D result = *this;
	result*= f;
	return result;
}

Vector2D& Vector2D::operator/=(const double& f)
{
	x /= f;
	y /= f;
	return *this;
};
	

Vector2D Vector2D::operator/(const double& f)
{
	Vector2D result = *this;
	result/= f;
	return result;
}

Vector2D& Vector2D::operator+=(const Vector2D& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

Vector2D Vector2D::operator+(const Vector2D& v)
{
	Vector2D result = *this;
	result += v;
	return result;
}

Vector2D& Vector2D::operator-=(const Vector2D& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

Vector2D Vector2D::operator-(const Vector2D& v)
{
	Vector2D result = *this;
	result -= v;
	return result;
}

void Vector2D::Normalize()
{
	double length = Length();
	if(length == 0)
	{
		x = y = 0;
		return;
	}
	x /= length;
	y /= length;
}

Vector2D Vector2D::GetNormalVector()
{
	return Vector2D(-y, x);
}

double Vector2D::Length()
{
	return sqrt(x*x+y*y);
}

Vector2D Vector2D::GetNormalVector(bool direction)
{
	Vector2D ret;
	ret.x = y;
	ret.y = x;
	if(!direction)
		ret.x = -ret.x;
	else
		ret.y = -ret.y;
	return ret;
}


Vector3D& Vector3D::operator*=(const double& f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
};
	

Vector3D Vector3D::operator*(const double& f)
{
	Vector3D result = *this;
	result*= f;
	return result;
}

Vector3D& Vector3D::operator/=(const double& f)
{
	x /= f;
	y /= f;
	z /=f;
	return *this;
};
	

Vector3D Vector3D::operator/(const double& f)
{
	Vector3D result = *this;
	result/= f;
	return result;
}

Vector3D& Vector3D::operator+=(const Vector3D& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector3D Vector3D::operator+(const Vector3D& v)
{
	Vector3D result = *this;
	result += v;
	return result;
}

Vector3D& Vector3D::operator-=(const Vector3D& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector3D Vector3D::operator-(const Vector3D& v)
{
	Vector3D result = *this;
	result -= v;
	return result;
}

void Vector3D::Normalize()
{
	double length = Length();
	if(length == 0)
	{
		x = y = z = 0;
		return;
	}
	x /= length;
	y /= length;
	z /= length;
}

double Vector3D::Length()
{
	return sqrt(x*x+y*y+z*z);
}

Vector3D::Vector3D(Point A, Point B)
{
	x = B.x-A.x;
	y = B.y-A.y;
	z = B.z-A.z;
}

Vector3D CrossProduct(Vector3D a, Vector3D b)
{
	Vector3D ret;
	ret.x = a.y*b.z-a.z*b.y;
	ret.y = a.z*b.x-a.x*b.z;
	ret.z = a.x*b.y-a.y*b.x;
	return ret;
}

double DotProduct(Vector3D a, Vector3D b)
{
	return a.x*b.x+a.y*b.y+a.z+b.z;
}

bool Compare3DVectors(Vector3D a, Vector3D b, double precision)
{
	return
		fabs(a.x-b.x) < precision &&
		fabs(a.y-b.y) < precision &&
		fabs(a.z-b.z) < precision;
}

bool Compare2DVectors(Vector2D a, Vector2D b, double precision)
{
	return
		fabs(a.x-b.x) < precision &&
		fabs(a.y-b.y) < precision;
}


bool HalfPlane(Vector2D edge, Vector2D point)
{
	edge = edge.GetNormalVector();
	return  (edge.x*point.x + edge.y*point.y) > 0;
}