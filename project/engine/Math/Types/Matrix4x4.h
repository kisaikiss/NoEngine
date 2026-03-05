#pragma once
#include "Vector3.h"
#include "Quaternion.h"

namespace NoEngine {
namespace Math {
__declspec(align(16))struct Matrix4x4 {
	float m[4][4];

	Matrix4x4() { operator=(IDENTITY); }
	Matrix4x4(const Matrix4x4& matrix4x4);
	Matrix4x4(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& w);
	Matrix4x4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33);

	Vector3 GetScale() const {
		Vector3 result;
		result.x = m[0][0];
		result.y = m[1][1];
		result.z = m[2][2];
		return result;
	}

	Quaternion GetRotation() const {
		return Quaternion(*this);
	}

	Vector3 GetTranslate() const { return Vector3(m[3][0], m[3][1], m[3][2]); }

	Vector3 Transform(const Vector3& vector);
	Vector3 TransformNormal(const Vector3& vector);

	void MakeScale(const Vector3& scale);
	void MakeRotate(const Quaternion& quaternion);
	void MakeRotate(const Vector3& rotate);
	void MakeTranslate(const Vector3& translate);

	void MakeAffine(const Vector3& scale, const Quaternion& rotate, const Vector3& translate);
	void MakeOrthographic(float left, float top, float right, float bottom, float zNear, float zFar);
	void MakePerspectiveFov(float fovY, float aspectRatio, float nearClip, float farClip);
	void MakeViewport(float left, float top, float width, float height, float minDepth, float maxDepth);

	void DirectionToDirection(const Vector3& from, const Vector3& to);

	void Inverse();
	void Transpose();


	static const Matrix4x4 ZERO;
	static const Matrix4x4 ZEROAFFINE;
	static const Matrix4x4 IDENTITY;
};

Matrix4x4 operator+(const Matrix4x4& matrix1, const Matrix4x4& matrix2);
Matrix4x4 operator-(const Matrix4x4& matrix1, const Matrix4x4& matrix2);
Matrix4x4 operator*(const Matrix4x4& matrix1, const Matrix4x4& matrix2);
Matrix4x4 operator*(float s, const Matrix4x4& matrix);
Matrix4x4 operator*(const Matrix4x4& matrix, float s);
Matrix4x4 operator-(const Matrix4x4& matrix);
}
}