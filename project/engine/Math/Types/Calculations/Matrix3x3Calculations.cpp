#include "Matrix3x3Calculations.h"

namespace NoEngine {
namespace MathCalculations {
Matrix3x3 Inverse(Matrix3x3 const& matrix) {
	Matrix3x3 result{};
	float determinant = matrix.m[0][0] * matrix.m[1][1] * matrix.m[2][2] +
		matrix.m[0][1] * matrix.m[1][2] * matrix.m[2][0] +
		matrix.m[0][2] * matrix.m[1][0] * matrix.m[2][1] -
		matrix.m[0][2] * matrix.m[1][1] * matrix.m[2][0] -
		matrix.m[0][1] * matrix.m[1][0] * matrix.m[2][2] -
		matrix.m[0][0] * matrix.m[1][2] * matrix.m[2][1];
	if (determinant == 0) {
		return result;
	}

	Matrix3x3 cofactorMatrix{};
	cofactorMatrix.m[0][0] = matrix.m[1][1] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][1];
	cofactorMatrix.m[0][1] = (matrix.m[0][1] * matrix.m[2][2] - matrix.m[0][2] * matrix.m[2][1]) * -1.f;
	cofactorMatrix.m[0][2] = matrix.m[0][1] * matrix.m[1][2] - matrix.m[0][2] * matrix.m[1][1];
	cofactorMatrix.m[1][0] = (matrix.m[1][0] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][0]) * -1.f;
	cofactorMatrix.m[1][1] = matrix.m[0][0] * matrix.m[2][2] - matrix.m[0][2] * matrix.m[2][0];
	cofactorMatrix.m[1][2] = (matrix.m[0][0] * matrix.m[1][2] - matrix.m[0][2] * matrix.m[1][0]) * -1.f;
	cofactorMatrix.m[2][0] = matrix.m[1][0] * matrix.m[2][1] - matrix.m[1][1] * matrix.m[2][0];
	cofactorMatrix.m[2][1] = (matrix.m[0][0] * matrix.m[2][1] - matrix.m[0][1] * matrix.m[2][0]) * -1.f;
	cofactorMatrix.m[2][2] = matrix.m[0][0] * matrix.m[1][1] - matrix.m[0][1] * matrix.m[1][0];

	for (int row = 0; row < 3; ++row) {
		for (int column = 0; column < 3; ++column) {
			result.m[row][column] = cofactorMatrix.m[row][column] / determinant;
		}
	}

	return result;
}

Matrix3x3 Multiply(Matrix3x3 const& matrix1, Matrix3x3 const& matrix2) {
	Matrix3x3 result{};

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				result.m[i][j] += matrix1.m[i][k] * matrix2.m[k][j];
			}
		}
	}

	return result;
}

Matrix3x3 MakeOrthographicMatrix(float const& left, float const& top, float const& right, float const& bottom) {
	Matrix3x3 result{};
	result.m[0][0] = 2.f / (right - left);
	result.m[1][1] = 2.f / (top - bottom);
	result.m[2][2] = 1.f;
	result.m[2][0] = (left + right) / (left - right);
	result.m[2][1] = (top + bottom) / (bottom - top);

	return result;
}

Matrix3x3 MakeViewportMatrix(float const& left, float const& top, float const& width, float const& height) {
	Matrix3x3 result{};
	result.m[0][0] = width / 2.f;
	result.m[1][1] = -1 * (height / 2.f);
	result.m[2][0] = left + result.m[0][0];
	result.m[2][1] = top + (height / 2.f);
	result.m[2][2] = 1.f;
	return result;
}

Matrix3x3 MakeScaleMatrix(Vector2 const& scale) {
	Matrix3x3 result{};
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = 1.0f;
	return result;
}

Matrix3x3 MakeRotateMatrix(float const& theta) {
	Matrix3x3 result{};
	result.m[0][0] = cosf(theta);
	result.m[0][1] = sinf(theta);
	result.m[1][0] = -sinf(theta);
	result.m[1][1] = cosf(theta);
	result.m[2][2] = 1.0f;
	return result;
}

Matrix3x3 MakeTranslateMatrix(Vector2 const& translate) {
	Matrix3x3 result{};
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][0] = translate.x;
	result.m[2][1] = translate.y;
	result.m[2][2] = 1.0f;
	return result;
}

Matrix3x3 MakeAffineMatrix(Vector2 const& scale, float const& rotate, Vector2 const& translate) {
	Matrix3x3 result{};
	result = Multiply(MakeScaleMatrix(scale), MakeRotateMatrix(rotate));
	result = Multiply(result, MakeTranslateMatrix(translate));
	return result;
}

Vector2 TransformPoint(Vector2 const& vector, Matrix3x3 const& matrix) {
	Vector2 result{};
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + 1.0f * matrix.m[2][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + 1.0f * matrix.m[2][1];
	float w = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + 1.0f * matrix.m[2][2];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	return result;
}
}
}