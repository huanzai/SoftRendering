#include "math.h"
#include <math.h>

// 数学库

// length
float VectorLength(Vector v)
{
	float sq = v.x * v.x + v.y * v.y + v.z * v.z;
	return (float)sqrt(sq);
}

// add
void VectorAdd(Vector& v, const Vector& x, const Vector& y)
{
	v.x = x.x + y.x;
	v.y = x.y + y.y;
	v.z = x.z + y.z;
	v.w = 1.0;
}

// sub
void VectorSub(Vector& v, const Vector& x, const Vector& y)
{
	v.x = x.x - y.x;
	v.y = x.y - y.y;
	v.z = x.z - y.z;
	v.w = 1.0f;
}

// dot product
float VectorDotProduct(const Vector& x, const Vector& y)
{
	return x.x * y.x + x.y * y.y + x.z * y.z;
}

// cross product
void VectorCrossProduct(Vector& v, const Vector& x, const Vector& y)
{
	float m1, m2, m3;
	v.x = x.y * y.z - x.z * y.y;
	v.y = x.z * y.x - x.x * y.z;
	v.z = x.x * y.y - x.y * y.x;
	v.w = 1.0f;
}

// normalize
void VectorNormalize(Vector& v)
{
	float len = VectorLength(v);
	if (len != 0.0f) {
		float inv = 1.0f / len;
		v.x *= inv;
		v.y *= inv;
		v.z *= inv;
	}
}

float interp(float x1, float x2, float t) { return x1 + (x2 - x1)*t; }

// interpolation
void VectorInterp(Vector& v, const Vector& x, const Vector& y, float t)
{
	v.x = interp(x.x, y.x, t);
	v.y = interp(x.y, y.y, t);
	v.z = interp(x.z, y.z, t);
	v.w = 1.0f;
}

// matrix identity
void MatrixSetIdentity(Matrix& m)
{
	m.m[0][0] = m.m[1][1] = m.m[2][2] = m.m[3][3] = 1.0f;
	m.m[0][1] = m.m[0][2] = m.m[0][3] = 0.0f;
	m.m[1][0] = m.m[1][2] = m.m[1][3] = 0.0f;
	m.m[2][0] = m.m[2][1] = m.m[2][3] = 0.0f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
}

// matrix zero
void MatrixSetZero(Matrix& m)
{
	m.m[0][0] = m.m[0][1] = m.m[0][2] = m.m[0][3] = 0.0f;
	m.m[1][0] = m.m[1][1] = m.m[1][2] = m.m[1][3] = 0.0f;
	m.m[2][0] = m.m[2][1] = m.m[2][2] = m.m[2][3] = 0.0f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = m.m[3][3] = 0.0f;
}

// matrix m = a + b
void MatrixAdd(Matrix& m, const Matrix& a, const Matrix& b)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			m.m[i][j] = a.m[i][j] + b.m[i][j];
	}
}

// matrix m = a - b
void MatrixSub(Matrix& m, const Matrix& a, const Matrix& b)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			m.m[i][j] = a.m[i][j] - b.m[i][j];
	}
}

// matrix m = a * b
void MatrixMul(Matrix& m, const Matrix& a, const Matrix& b)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m.m[j][i] = (a.m[j][0] * b.m[0][i]) +
						(a.m[j][1] * b.m[1][i]) +
						(a.m[j][2] * b.m[2][i]) +
						(a.m[j][3] * b.m[3][i]);
		}
	}
}

// matrix m = a * f
void MatrixScale(Matrix& m, const Matrix& a, const float f)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			m.m[i][j] = a.m[i][j] * f;
	}
}

// matrix v = x * m
void MatrixApply(Vector& v, const Vector& x, const Matrix& m)
{
	float X = x.x, Y = x.y, Z = x.z, W = x.w;
	v.x = X * m.m[0][0] + Y * m.m[1][0] + Z * m.m[2][0] + W * m.m[3][0];
	v.y = X * m.m[0][1] + Y * m.m[1][1] + Z * m.m[2][1] + W * m.m[3][1];
	v.z = X * m.m[0][2] + Y * m.m[1][2] + Z * m.m[2][2] + W * m.m[3][2];
	v.w = X * m.m[0][3] + Y * m.m[1][3] + Z * m.m[2][3] + W * m.m[3][3];
}

// translate
void MatrixSetTranslate(Matrix& m, float x, float y, float z)
{
	MatrixSetIdentity(m);
	m.m[3][0] = x;
	m.m[3][1] = y;
	m.m[3][2] = z;
}

// scale
void MatrixSetScale(Matrix& m, float x, float y, float z)
{
	MatrixSetIdentity(m);
	m.m[0][0] = x;
	m.m[1][1] = y;
	m.m[2][2] = z;
}

// rotate
void MatrixSetRotate(Matrix& m, float x, float y, float z, float theta)
{
	float qsin = (float)sin(theta * 0.5f);
	float qcos = (float)cos(theta * 0.5f);
	Vector vec = { x,y,z,1.0f };
	float w = qcos;
	VectorNormalize(vec);
	x = vec.x * qsin;
	y = vec.y * qsin;
	z = vec.z * qsin;
	m.m[0][0] = 1 - 2 * y * y - 2 * z * z;
	m.m[1][0] = 2 * x * y - 2 * w * z;
	m.m[2][0] = 2 * x * z + 2 * w * y;
	m.m[0][1] = 2 * x * y + 2 * w * z;
	m.m[1][1] = 1 - 2 * x * x - 2 * z * z;
	m.m[2][1] = 2 * y * z - 2 * w * x;
	m.m[0][2] = 2 * x * z - 2 * w * y;
	m.m[1][2] = 2 * y * z - 2 * w * x;
	m.m[2][2] = 1 - 2 * x * x - 2 * y * y;
	m.m[0][3] = m.m[1][3] = m.m[2][3] = 0.0f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
	m.m[3][3] = 1.0f;
}

// lookat
void MatrixSetLookAt(Matrix& m, const Vector& eye, const Vector& at, const Vector& up)
{
	Vector xaxis, yaxis, zaxis;

	VectorSub(zaxis, at, eye); // 眼睛前方 z 轴
	VectorNormalize(zaxis);
	VectorCrossProduct(xaxis, up, zaxis); // up 和 z = x 轴
	VectorNormalize(xaxis);
	VectorCrossProduct(yaxis, zaxis, xaxis);

	m.m[0][0] = xaxis.x;
	m.m[1][0] = xaxis.y;
	m.m[2][0] = xaxis.z;
	m.m[3][0] = -VectorDotProduct(xaxis, eye);

	m.m[0][1] = yaxis.x;
	m.m[1][1] = yaxis.y;
	m.m[2][1] = yaxis.z;
	m.m[3][1] = -VectorDotProduct(yaxis, eye);

	m.m[0][2] = zaxis.x;
	m.m[1][2] = zaxis.y;
	m.m[2][2] = zaxis.z;
	m.m[3][2] = -VectorDotProduct(zaxis, eye);

	m.m[0][3] = m.m[1][3] = m.m[2][3] = 0.0f;
	m.m[3][3] = 1.0f;
}

// perspective
void MatrixSetPerspective(Matrix& m, float fovy, float aspect, float zn, float zf)
{
	float fax = 1.0f / (float)tan(fovy * 0.5f);
	MatrixSetZero(m);
	m.m[0][0] = (float)(fax / aspect);
	m.m[1][1] = (float)(fax);
	m.m[2][2] = zf / (zf - zn);
	m.m[3][2] = -zn*zf / (zf - zn);
	m.m[2][3] = 1;
}