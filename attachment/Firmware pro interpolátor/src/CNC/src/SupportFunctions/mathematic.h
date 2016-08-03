#ifndef MATHEMATIC_H_
#define MATHEMATIC_H_

extern const float sinLUT[];
extern const float cosLUT[];

void Normalize3DVector(float* vector);
void Multiply3DVector(float* vector, float a);
void Multiply3DVectorTo(float* vector, float a, float* v);
void Divide3DVector(float* vector, float a);
void Divide3DVectorTo(float* vector, float a, float* v);
float Abs3DVector(float* vector);
void Subtract3DVectorTo(float* a, float* b, float* v);
void Add3DVectorTo(float* a, float* b, float* v);
void AssignVector(float* a, float* b);
void CrossProduct3DVector(float* u, float* v, float* o);
float DotProduct3DVector(float* u, float* v);
float min(float a, float b);


float myFastSin (float angle );
float myFastCos (float angle );



#endif /* MATHEMATIC_H_ */
