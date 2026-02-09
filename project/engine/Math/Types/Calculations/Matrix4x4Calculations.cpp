#include "Matrix4x4Calculations.h"
#include "QuaternionCalculations.h"

namespace NoEngine {
namespace MathCalculations {
using namespace Math;

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);
	return result;
}


Matrix4x4 Multiply(Matrix4x4 const& matrix1, Matrix4x4 const& matrix2) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = 0;
			for (int k = 0; k < 4; k++) {
				result.m[i][j] += matrix1.m[i][k] * matrix2.m[k][j];
			}
		}
	}

	return result;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result{};
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakePitchMatrix(float theta) {
	Matrix4x4 result{};
	result.m[0][0] = 1.0f;
	result.m[1][1] = cosf(theta);
	result.m[1][2] = sinf(theta);
	result.m[2][1] = -sinf(theta);
	result.m[2][2] = cosf(theta);
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeYowMatrix(float theta) {
	Matrix4x4 result{};
	result.m[0][0] = std::cos(theta);
	result.m[1][1] = 1.0f;
	result.m[2][0] = std::sin(theta);
	result.m[0][2] = -std::sin(theta);
	result.m[2][2] = std::cos(theta);
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeRollMatrix(float theta) {
	Matrix4x4 result{};
	result.m[0][0] = cosf(theta);
	result.m[0][1] = sinf(theta);
	result.m[1][0] = -sinf(theta);
	result.m[1][1] = cosf(theta);
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeRotateMatrix(const Vector3& theta) {
	Matrix4x4 result{};
	result = Multiply(MakePitchMatrix(theta.x), Multiply(MakeYowMatrix(theta.y), MakeRollMatrix(theta.z)));
	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result{};
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 result{};
	result = Multiply(MakeScaleMatrix(scale), MakeRotateMatrix(rotate));
	result = Multiply(result, MakeTranslateMatrix(translate));
	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate) {
	Matrix4x4 result;
	result = Multiply(MakeScaleMatrix(scale), MakeRotateMatrix(rotate));
	result = Multiply(result, MakeTranslateMatrix(translate));
	return result;
}

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result{};
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}

Vector3 TransforNormal(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result{
		vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0],
		vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1],
		vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2]
	};

	return result;
}

Matrix4x4 Inverse(Matrix4x4 matrix) {
	Matrix4x4 result{};

	float determinant =
		matrix.m[0][3] * matrix.m[1][2] * matrix.m[2][1] * matrix.m[3][0] - matrix.m[0][2] * matrix.m[1][3] * matrix.m[2][1] * matrix.m[3][0] -
		matrix.m[0][3] * matrix.m[1][1] * matrix.m[2][2] * matrix.m[3][0] + matrix.m[0][1] * matrix.m[1][3] * matrix.m[2][2] * matrix.m[3][0] +
		matrix.m[0][2] * matrix.m[1][1] * matrix.m[2][3] * matrix.m[3][0] - matrix.m[0][1] * matrix.m[1][2] * matrix.m[2][3] * matrix.m[3][0] -
		matrix.m[0][3] * matrix.m[1][2] * matrix.m[2][0] * matrix.m[3][1] + matrix.m[0][2] * matrix.m[1][3] * matrix.m[2][0] * matrix.m[3][1] +
		matrix.m[0][3] * matrix.m[1][0] * matrix.m[2][2] * matrix.m[3][1] - matrix.m[0][0] * matrix.m[1][3] * matrix.m[2][2] * matrix.m[3][1] -
		matrix.m[0][2] * matrix.m[1][0] * matrix.m[2][3] * matrix.m[3][1] + matrix.m[0][0] * matrix.m[1][2] * matrix.m[2][3] * matrix.m[3][1] +
		matrix.m[0][3] * matrix.m[1][1] * matrix.m[2][0] * matrix.m[3][2] - matrix.m[0][1] * matrix.m[1][3] * matrix.m[2][0] * matrix.m[3][2] -
		matrix.m[0][3] * matrix.m[1][0] * matrix.m[2][1] * matrix.m[3][2] + matrix.m[0][0] * matrix.m[1][3] * matrix.m[2][1] * matrix.m[3][2] +
		matrix.m[0][1] * matrix.m[1][0] * matrix.m[2][3] * matrix.m[3][2] - matrix.m[0][0] * matrix.m[1][1] * matrix.m[2][3] * matrix.m[3][2] -
		matrix.m[0][2] * matrix.m[1][1] * matrix.m[2][0] * matrix.m[3][3] + matrix.m[0][1] * matrix.m[1][2] * matrix.m[2][0] * matrix.m[3][3] +
		matrix.m[0][2] * matrix.m[1][0] * matrix.m[2][1] * matrix.m[3][3] - matrix.m[0][0] * matrix.m[1][2] * matrix.m[2][1] * matrix.m[3][3] -
		matrix.m[0][1] * matrix.m[1][0] * matrix.m[2][2] * matrix.m[3][3] + matrix.m[0][0] * matrix.m[1][1] * matrix.m[2][2] * matrix.m[3][3];


	if (determinant == 0) {
		return result;
	}

	Matrix4x4 cofactorMatrix{};
	cofactorMatrix.m[0][0] = matrix.m[1][1] * (matrix.m[2][2] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][2]) - matrix.m[1][2] * (matrix.m[2][1] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][1]) + matrix.m[1][3] * (matrix.m[2][1] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][1]);
	cofactorMatrix.m[0][1] = -(matrix.m[0][1] * (matrix.m[2][2] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][2]) - matrix.m[0][2] * (matrix.m[2][1] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][1]) + matrix.m[0][3] * (matrix.m[2][1] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][1]));
	cofactorMatrix.m[0][2] = matrix.m[0][1] * (matrix.m[1][2] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][2]) - matrix.m[0][2] * (matrix.m[1][1] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][1]) + matrix.m[0][3] * (matrix.m[1][1] * matrix.m[3][2] - matrix.m[1][2] * matrix.m[3][1]);
	cofactorMatrix.m[0][3] = -(matrix.m[0][1] * (matrix.m[1][2] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][2]) - matrix.m[0][2] * (matrix.m[1][1] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][1]) + matrix.m[0][3] * (matrix.m[1][1] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][1]));
	cofactorMatrix.m[1][0] = -(matrix.m[1][0] * (matrix.m[2][2] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][2]) - matrix.m[1][2] * (matrix.m[2][0] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][0]) + matrix.m[1][3] * (matrix.m[2][0] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][0]));
	cofactorMatrix.m[1][1] = matrix.m[0][0] * (matrix.m[2][2] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][2]) - matrix.m[0][2] * (matrix.m[2][0] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][0]) + matrix.m[0][3] * (matrix.m[2][0] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][0]);
	cofactorMatrix.m[1][2] = -(matrix.m[0][0] * (matrix.m[1][2] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][2]) - matrix.m[0][2] * (matrix.m[1][0] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][0]) + matrix.m[0][3] * (matrix.m[1][0] * matrix.m[3][2] - matrix.m[1][2] * matrix.m[3][0]));
	cofactorMatrix.m[1][3] = matrix.m[0][0] * (matrix.m[1][2] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][2]) - matrix.m[0][2] * (matrix.m[1][0] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][0]) + matrix.m[0][3] * (matrix.m[1][0] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][0]);
	cofactorMatrix.m[2][0] = matrix.m[1][0] * (matrix.m[2][1] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][1]) - matrix.m[1][1] * (matrix.m[2][0] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][0]) + matrix.m[1][3] * (matrix.m[2][0] * matrix.m[3][1] - matrix.m[2][1] * matrix.m[3][0]);
	cofactorMatrix.m[2][1] = -matrix.m[0][0] * (matrix.m[2][1] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][1]) + matrix.m[0][1] * (matrix.m[2][0] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][0]) - matrix.m[0][3] * (matrix.m[2][0] * matrix.m[3][1] - matrix.m[2][1] * matrix.m[3][0]);
	cofactorMatrix.m[2][2] = matrix.m[0][0] * (matrix.m[1][1] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][1]) + matrix.m[0][1] * (matrix.m[1][3] * matrix.m[3][0] - matrix.m[1][0] * matrix.m[3][3]) + matrix.m[0][3] * (matrix.m[1][0] * matrix.m[3][1] - matrix.m[1][1] * matrix.m[3][0]);
	cofactorMatrix.m[2][3] = -matrix.m[0][0] * (matrix.m[1][1] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][1]) - matrix.m[0][1] * (matrix.m[1][3] * matrix.m[2][0] - matrix.m[1][0] * matrix.m[2][3]) - matrix.m[0][3] * (matrix.m[1][0] * matrix.m[2][1] - matrix.m[1][1] * matrix.m[2][0]);
	cofactorMatrix.m[3][0] = matrix.m[1][0] * (-matrix.m[2][1] * matrix.m[3][2] + matrix.m[2][2] * matrix.m[3][1]) + matrix.m[1][1] * (-matrix.m[2][2] * matrix.m[3][0] + matrix.m[2][0] * matrix.m[3][2]) + matrix.m[1][2] * (matrix.m[2][1] * matrix.m[3][0] - matrix.m[2][0] * matrix.m[3][1]);
	cofactorMatrix.m[3][1] = -matrix.m[0][0] * (-matrix.m[2][1] * matrix.m[3][2] + matrix.m[2][2] * matrix.m[3][1]) - matrix.m[0][1] * (-matrix.m[2][2] * matrix.m[3][0] + matrix.m[2][0] * matrix.m[3][2]) - matrix.m[0][2] * (matrix.m[2][1] * matrix.m[3][0] - matrix.m[2][0] * matrix.m[3][1]);
	cofactorMatrix.m[3][2] = matrix.m[3][2] * (-matrix.m[0][0] * matrix.m[1][1] + matrix.m[0][1] * matrix.m[1][0]) + matrix.m[3][0] * (-matrix.m[0][1] * matrix.m[1][2] + matrix.m[0][2] * matrix.m[1][1]) + matrix.m[3][1] * (matrix.m[0][0] * matrix.m[1][2] - matrix.m[0][2] * matrix.m[1][0]);
	cofactorMatrix.m[3][3] = -matrix.m[2][2] * (-matrix.m[0][0] * matrix.m[1][1] + matrix.m[0][1] * matrix.m[1][0]) - matrix.m[2][0] * (-matrix.m[0][1] * matrix.m[1][2] + matrix.m[0][2] * matrix.m[1][1]) - matrix.m[2][1] * (matrix.m[0][0] * matrix.m[1][2] - matrix.m[0][2] * matrix.m[1][0]);

	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			result.m[row][column] = cofactorMatrix.m[row][column] / determinant;
		}
	}

	return result;
}

Matrix4x4 Transpose(Matrix4x4 matrix) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = matrix.m[j][i];
		}
	}
	return result;
}

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float zNear, float zFar) {
	Matrix4x4 result{};
	result.m[0][0] = 2.f / (right - left);
	result.m[1][1] = 2.f / (top - bottom);
	result.m[2][2] = 1.f / (zFar - zNear);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = zNear / (zNear - zFar);
	result.m[3][3] = 1.f;

	return result;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result{};
	result.m[0][0] = (1.f / aspectRatio) * (1.f / tanf(fovY / 2.f));
	result.m[1][1] = 1.f / tanf(fovY / 2.f);
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	result.m[2][3] = 1.f;
	return result;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result{};
	result.m[0][0] = width / 2.f;
	result.m[1][1] = -1 * (height / 2.f);
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + result.m[0][0];
	result.m[3][1] = top + (height / 2.f);
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.f;
	return result;
}
}
}