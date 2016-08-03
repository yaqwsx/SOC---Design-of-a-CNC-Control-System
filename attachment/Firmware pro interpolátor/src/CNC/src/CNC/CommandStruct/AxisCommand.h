#ifndef AXISCOMMAND_H_
#define AXISCOMMAND_H_

typedef struct
{
	float axes[3];//new position of axes
	float v0, v, vb;//starting velocity, target velocity, braking velocity
	float As, Ab;
}	AxisLine;

typedef struct
{
		float B[3], C[3], S[3];
		float v, v0, vb;
		float As, Ab;
		float alpha;
}	AxisCircle;

typedef struct
{
		float B[3], C[3];//C - direction point, B - ending point
		float v;
}	AxisSine;

#endif /* AXISCOMMAND_H_ */
