#include "Vector2.h"
#include "Calculations/Vector2Calculations.h"

namespace NoEngine {
namespace Math {
const Vector2 Vector2::ZERO(0.f, 0.f);
const Vector2 Vector2::RIGHT(1.f, 0.f);
const Vector2 Vector2::UNIT_SCALE(1.f, 1.f);

Vector2& Vector2::operator+=(const Vector2& other) {
    x += other.x;
    y += other.y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2& Vector2::operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
}

Vector2& Vector2::operator/=(float s) {
    x /= s;
    y /= s;
    return *this;
}

Vector2 operator+(const Vector2& v1, const Vector2& v2) {
    return Vector2(v1.x + v2.x, v1.y + v2.y);
}

Vector2 operator-(const Vector2& v1, const Vector2& v2) {
    return Vector2(v1.x - v2.x, v1.y - v2.y);
}

Vector2 operator*(float s, const Vector2& v) {
    return Vector2(v.x * s, v.y * s);
}

Vector2 operator*(const Vector2& v, float s) {
    return s * v;
}

Vector2 operator/(const Vector2& v, float s) {
    return Vector2(v.x / s, v.y / s);
}

float Vector2::Closs(const Vector2& other) const noexcept {
    return  MathCalculations::Closs(*this, other);
}

float Vector2::Dot(const Vector2& other) const noexcept {
    return MathCalculations::Dot(*this, other);
}

float Vector2::Length() const noexcept {
    return MathCalculations::Length(*this);
}

float Vector2::LengthSquared() const noexcept {
    return MathCalculations::LengthSquared(*this);
}

Vector2 Vector2::Normalize() const {
    return MathCalculations::Normalize(*this);
}
}
}