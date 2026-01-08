#include "Vector2.h"

namespace NoEngine {
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

}