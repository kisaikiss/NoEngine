#pragma once
#include "Vector2.h"

namespace NoEngine {
namespace Math {
struct Matrix4x4;
__declspec(align(4))struct Matrix3x3 {
public:
	float m[3][3];

	Matrix3x3() { operator=(IDENTITY); }
	Matrix3x3(const Matrix4x4& matrix4x4);
	Matrix3x3(float entry00, float entry01, float entry02,
		float entry10, float entry11, float entry12,
		float entry20, float entry21, float entry22);

	void MakeAffine(const Vector2& scale, const float& rotate, const Vector2& translate);
	void Inverse();

	static const Matrix3x3 ZERO;
	static const Matrix3x3 IDENTITY;
};
Matrix3x3 operator*(const Matrix3x3& matrix1, const Matrix3x3& matrix2);
}
}