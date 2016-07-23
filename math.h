#pragma once

// ÊýÑ§¿â

struct Vector {
	float x, y, z, w;
};

struct Matrix
{
	float m[4][4];
};

// length
float VectorLength(Vector v);

// add
void VectorAdd(Vector& v, const Vector& x, const Vector& y);

// sub
void VectorSub(Vector& v, const Vector& x, const Vector& y);

// dot product
float VectorDotProduct(const Vector& x, const Vector& y);

// cross product
void VectorCrossProduct(Vector& v, const Vector& x, const Vector& y);

// normalize
void VectorNormalize(Vector& v);

// interpolation
void VectorInterp(Vector& v, const Vector& x, const Vector& y, float t);


// matrix identity
void MatrixSetIdentity(Matrix& m);

// matrix zero
void MatrixSetZero(Matrix& m);

// matrix m = a + b
void MatrixAdd(Matrix& m, const Matrix& a, const Matrix& b);

// matrix m = a - b
void MatrixSub(Matrix& m, const Matrix& a, const Matrix& b);

// matrix m = a * b
void MatrixMul(Matrix& m, const Matrix& a, const Matrix& b);

// matrix m = a * f
void MatrixScale(Matrix& m, const Matrix& a, const float f);

// matrix v = x * m
void MatrixApply(Vector& v, const Vector& x, const Matrix& m);

// translate
void MatrixSetTranslate(Matrix& m, float x, float y, float z);

// scale
void MatrixSetScale(Matrix& m, float x, float y, float z);

// rotate
void MatrixSetRotate(Matrix& m, float x, float y, float z, float theta);

// lookat
void MatrixSetLookAt(Matrix& m, const Vector& eye, const Vector& at, const Vector& up);

// perspective
void MatrixSetPerspective(Matrix& m, float fovy, float aspect, float zn, float fn);
