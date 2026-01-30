#pragma once
#include "engine/Math/Types/Vector3.h"
#include "engine/Math/Types/Vector4.h"

namespace NoEngine {
/// <summary>
/// 色を表すクラス
/// </summary>
class Color {
public:
	float r, g, b, a;

	Color() = default;
	Color(float R, float G, float B, float A) : r(R), g(G), b(B), a(A) {};
	Color(float R, float G, float B) : r(R), g(G), b(B), a(1.f) {};
	Color(uint32_t rgba) {
		r = ((rgba >> 24) & 0xFF) / 255.0f;
		g = ((rgba >> 16) & 0xFF) / 255.0f;
		b = ((rgba >> 8) & 0xFF) / 255.0f;
		a = ((rgba >> 0) & 0xFF) / 255.0f;
	}

	Vector3 ToVector3() const {
		return Vector3(r, g, b);
	}
	Vector4 ToVector4() const {
		return Vector4(r, g, b, a);
	}

	// 十六進数へ変換
	uint32_t ToRGBA8() const;


	bool operator<(const Color& other) const {
		return r + g + b < other.r + other.g + other.b; // SpritePassで色をまとめて描画するために使用します。
	}

	float* ptr(void) { return reinterpret_cast<float*>(this); }
	float& operator[](int index) { return ptr()[index]; }
	Color operator*(float s) const {
		return Color(r * s, g * s, b * s, a * s);
	}
	Color operator+(const Color& other) const {
		return Color(r + other.r, g + other.g, b + other.b, a + other.a);
	}
	Color operator-(const Color& other) const {
		return Color(r - other.r, g - other.g, b - other.b, a - other.a);
	}

	static const Color WHITE;
	static const Color BLACK;
};
}