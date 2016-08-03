#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <wx\wx.h>

//This header file defines auxiliary classes for mathematics

struct Point
{
	Point(double a, double b, double c): x(a), y(b), z(c){};
	Point(double f):x(f), y(f), z(f){};
	Point():x(0), y(0), z(0){};
	double x, y, z, a, b;
	bool operator==(const Point& v){return x == v.x && y == v.y && z == v.z;};
	bool operator!=(const Point& v){return !(*this == v);};
	Point& operator*=(double coefficient);
	Point(const wxPoint& p);
	Point& operator=(const wxPoint& p);
	Point& operator+=(const Point& p);
	Point& operator-=(const Point& p);
	Point operator+(const Point& p);
	Point operator-(const Point& p);
	Point& operator/=(const double& f);
	//Point& operator*=(const double& f){x*=f; y*=f; z*=f; a*=f; b*=f;};
	Point operator/(const double& f);
};

struct Vector2D
{
	double x, y;
	void Normalize();
	double Length();
	Vector2D GetNormalVector();
	bool operator!=(const Vector2D& v){return !(*this == v);};
	Vector2D(double a = 0, double b = 0): x(a), y(b){};
	Vector2D(Point& a, Point& b){x=b.x-a.x; y=b.y-a.y;};
	Vector2D GetNormalVector(bool direction);
	bool operator==(const Vector2D& v){return x == v.x && y == v.y;};
	Vector2D& operator*=(const double& f);
	Vector2D operator*(const double& f);
	Vector2D& operator/=(const double& f);
	Vector2D operator/(const double& f);
	Vector2D& operator+=(const Vector2D& v);
	Vector2D operator+(const Vector2D& v);
	Vector2D& operator-=(const Vector2D& v);
	Vector2D operator-(const Vector2D& v);
	Vector2D operator-(){return Vector2D(-x, -y);};
};

bool HalfPlane(Vector2D edge, Vector2D point);

struct Vector3D
{
	double x, y, z;
	void Normalize();
	double Length();
	Vector3D(double f):x(f), y(f), z(f){};
	Vector3D(): x(0), y(0), z(0){};
	Vector3D(double a, double b, double c): x(a), y(b), z(c){};
	Vector3D(Point A, Point B);
	bool operator==(const Vector3D& v){return (x==v.x && y == v.y && z == v.z);};
	Vector3D& operator*=(const double& f);
	Vector3D operator*(const double& f);
	Vector3D& operator/=(const double& f);
	Vector3D operator/(const double& f);
	Vector3D& operator+=(const Vector3D& v);
	Vector3D operator+(const Vector3D& v);
	Vector3D& operator-=(const Vector3D& v);
	Vector3D operator-(const Vector3D& v);
	Vector3D operator-(){return Vector3D(-x, -y, -z);};
};

Vector3D CrossProduct(Vector3D a, Vector3D b);
double DotProduct(Vector3D a, Vector3D b);
bool Compare3DVectors(Vector3D a, Vector3D b, double precision);
bool Compare2DVectors(Vector2D a, Vector2D b, double precision);