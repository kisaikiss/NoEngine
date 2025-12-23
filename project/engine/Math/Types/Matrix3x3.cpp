#include "Matrix3x3.h"
#include "Matrix4x4.h"
namespace NoEngine {

const Matrix3x3 Matrix3x3::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0);
const Matrix3x3 Matrix3x3::IDENTITY(1, 0, 0, 0, 1, 0, 0, 0, 1);

Matrix3x3::Matrix3x3(const Matrix4x4& matrix4x4) {
    m[0][0] = matrix4x4.m[0][0];
    m[0][1] = matrix4x4.m[0][1];
    m[0][2] = matrix4x4.m[0][2];
    m[1][0] = matrix4x4.m[1][0];
    m[1][1] = matrix4x4.m[1][1];
    m[1][2] = matrix4x4.m[1][2];
    m[2][0] = matrix4x4.m[2][0];
    m[2][1] = matrix4x4.m[2][1];
    m[2][2] = matrix4x4.m[2][2];
}

Matrix3x3::Matrix3x3(float entry00, float entry01, float entry02,
	float entry10, float entry11, float entry12,
	float entry20, float entry21, float entry22) {
	m[0][0] = entry00; m[0][1] = entry01; m[0][2] = entry02;
	m[1][0] = entry10; m[1][1] = entry11; m[1][2] = entry12;
	m[2][0] = entry20; m[2][1] = entry21; m[2][2] = entry22;
}

}