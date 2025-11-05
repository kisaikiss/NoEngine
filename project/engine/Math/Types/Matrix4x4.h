#pragma once

namespace NoEngine {

struct Matrix4x4 {
	float m[4][4];
};

Matrix4x4 operator*(const Matrix4x4& matrix1, const Matrix4x4& matrix2);
}