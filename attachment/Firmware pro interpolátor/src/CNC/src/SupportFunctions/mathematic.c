#include "mathematic.h"
#include <math.h>


void Normalize3DVector(float* v)
{
	float length = Abs3DVector(v);
	if(length == 0)
	{
		v[0] = v[1] = v[2] = 0;
	}
	else
	{
		v[0] /= length;
		v[1] /= length;
		v[2] /= length;
	}
}

void Multiply3DVector(float* v, float a)
{
	v[0] *= a;
	v[1] *= a;
	v[2] *= a;
}

void Multiply3DVectorTo(float* vec, float a, float* v)
{
	v[0] = vec[0]*a;
	v[1] = vec[1]*a;
	v[2] = vec[2]*a;
}

void Divide3DVector(float* v, float a)
{
	v[0] /= a;
	v[1] /= a;
	v[2] /= a;
}

void Divide3DVectorTo(float* vec, float a, float* v)
{
	v[0] = vec[0]/a;
	v[1] = vec[1]/a;
	v[2] = vec[2]/a;
}

float Abs3DVector(float* v)
{
	return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
}

void Subtract3DVectorTo(float* a, float* b, float* v)
{
	v[0] = a[0]-b[0];
	v[1] = a[1]-b[1];
	v[2] = a[2]-b[2];
}
void Add3DVectorTo(float* a, float* b, float* v)
{
	v[0] = a[0]+b[0];
	v[1] = a[1]+b[1];
	v[2] = a[2]+b[2];
}

void AssignVector(float* a, float* b)
{
	b[0] = a[0];
	b[1] = a[1];
	b[2] = a[2];
}

float min(float a, float b)
{
	if(a<b)
		return a;
	return b;
}

void CrossProduct3DVector(float* u, float* v, float* o)
{
	o[0] = u[1]*v[2]-v[1]*u[2];
	o[1] = u[2]*v[0]-v[2]*u[0];
	o[2] = u[0]*v[1]-v[0]*u[1];
}

float DotProduct3DVector(float* u, float* v)
{
	return u[0]*v[0]+u[1]*v[1]+u[2]*v[2];
}

float myFastSin (float angle )
{
	int index = angle*1273.2395447351626861510701069801;
	if(index < 0)
		index+= 8000;
	if(index > 8000)
		index -= 8000;
	//Parse quadrants
	if(index > 4000)
	{
		if(index > 6000)
			index = 8000 - 6000;
		else index -= 4000;
		return -sinLUT[index];
	}
	if(index > 2000)
		index = 4000-index;
	return sinLUT[index];
}

float myFastCos (float angle )
{
	int index = angle*1273.2395447351626861510701069801;
	//Make it cos
	index += 2000;
	if(index < 0)
		index+= 8000;
	if(index > 8000)
		index -= 8000;
	//Parse quadrants
	if(index > 4000)
	{
		if(index > 6000)
			index = 8000 - 6000;
		else index -= 4000;
		return -sinLUT[index];
	}
	if(index > 2000)
		index = 4000-index;
	return sinLUT[index];
}
