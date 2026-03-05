#include "Matrix4x4.h"
#include "Calculations/Matrix4x4Calculations.h"
#include "Calculations/QuaternionCalculations.h"
namespace NoEngine {
namespace Math {
const Matrix4x4 Matrix4x4::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const Matrix4x4 Matrix4x4::ZEROAFFINE(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
const Matrix4x4 Matrix4x4::IDENTITY(
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1);

Matrix4x4 operator+(const Matrix4x4& matrix1, const Matrix4x4& matrix2) {
	return MathCalculations::Add(matrix1, matrix2);
}

Matrix4x4 operator-(const Matrix4x4& matrix1, const Matrix4x4& matrix2) {
	return MathCalculations::Subtract(matrix1, matrix2);
}

Matrix4x4 operator*(const Matrix4x4& matrix1, const Matrix4x4& matrix2) {
	return MathCalculations::Multiply(matrix1, matrix2);
}

Matrix4x4 operator*(float s, const Matrix4x4& matrix) {
	Matrix4x4 result;
	for (uint32_t i = 0; i < 4; i++) {
		for (uint32_t j = 0; j < 4; j++) {
			result.m[i][j] = matrix.m[i][j] * s;
		}
	}
	return result;
}

Matrix4x4 operator*(const Matrix4x4& matrix, float s) {
	return operator*(s, matrix);
}

Matrix4x4 operator-(const Matrix4x4& matrix) {
	Matrix4x4 result;
	for (uint32_t i = 0; i < 4; i++) {
		for (uint32_t j = 0; j < 4; j++) {
			result.m[i][j] = matrix.m[i][j] * -1.0f;
		}
	}

	return result;
}

Matrix4x4::Matrix4x4(const Matrix4x4& matrix4x4) {
	m[0][0] = matrix4x4.m[0][0];
	m[0][1] = matrix4x4.m[0][1];
	m[0][2] = matrix4x4.m[0][2];
	m[0][3] = matrix4x4.m[0][3];
	m[1][0] = matrix4x4.m[1][0];
	m[1][1] = matrix4x4.m[1][1];
	m[1][2] = matrix4x4.m[1][2];
	m[1][3] = matrix4x4.m[1][3];
	m[2][0] = matrix4x4.m[2][0];
	m[2][1] = matrix4x4.m[2][1];
	m[2][2] = matrix4x4.m[2][2];
	m[2][3] = matrix4x4.m[2][3];
	m[3][0] = matrix4x4.m[3][0];
	m[3][1] = matrix4x4.m[3][1];
	m[3][2] = matrix4x4.m[3][2];
	m[3][3] = 1.0f;
}

Matrix4x4::Matrix4x4(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& w) {
	m[0][0] = x.x;
	m[0][1] = x.y;
	m[0][2] = x.z;
	m[0][3] = 0.0f;
	m[1][0] = y.x;
	m[1][1] = y.y;
	m[1][2] = y.z;
	m[1][3] = 0.0f;
	m[2][0] = z.x;
	m[2][1] = z.y;
	m[2][2] = z.z;
	m[2][3] = 0.0f;
	m[3][0] = w.x;
	m[3][1] = w.y;
	m[3][2] = w.z;
	m[3][3] = 1.0f;
}

Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33) {
	m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
	m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
	m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
	m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
}

Vector3 Matrix4x4::Transform(const Vector3& vector) {
	return MathCalculations::Transform(vector, *this);
}

Vector3 Matrix4x4::TransformNormal(const Vector3& vector) {
	return MathCalculations::TransformNormal(vector, *this);
}

void Matrix4x4::MakeScale(const Vector3& scale) {
	*this = MathCalculations::MakeScaleMatrix(scale);
}

void Matrix4x4::MakeRotate(const Quaternion& quaternion) {
	*this = MathCalculations::MakeRotateMatrix(quaternion);
}

void Matrix4x4::MakeRotate(const Vector3& rotate) {
	*this = MathCalculations::MakeRotateMatrix(rotate);
}

void Matrix4x4::MakeTranslate(const Vector3& translate) {
	*this = MathCalculations::MakeTranslateMatrix(translate);
}

void Matrix4x4::MakeAffine(const Vector3& scale, const Quaternion& rotate, const Vector3& translate) {
	*this = MathCalculations::MakeAffineMatrix(scale, rotate, translate);
}

void Matrix4x4::MakeOrthographic(float left, float top, float right, float bottom, float zNear, float zFar) {
	*this = MathCalculations::MakeOrthographicMatrix(left, top, right, bottom, zNear, zFar);
}

void Matrix4x4::MakePerspectiveFov(float fovY, float aspectRatio, float nearClip, float farClip) {
	*this = MathCalculations::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
}

void Matrix4x4::MakeViewport(float left, float top, float width, float height, float minDepth, float maxDepth) {
	*this = MathCalculations::MakeViewportMatrix(left, top, width, height, minDepth, maxDepth);
}

void Matrix4x4::DirectionToDirection(const Vector3& from, const Vector3& to) {
	*this = MathCalculations::DirectionToDirection(from, to);
}

void Matrix4x4::Inverse() {
	*this = MathCalculations::Inverse(*this);
}

void Matrix4x4::Transpose() {
	*this = MathCalculations::Transpose(*this);
}
}
}