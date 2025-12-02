#pragma once
namespace NoEngine {
/// <summary>
/// 色を表すクラス
/// </summary>
class Color {
public:
	float r, g, b, a;

	Color() = default;
	Color(float R, float G, float B, float A) : r(R), g(G), b(B), a(A) {};
	Color(float R, float G, float B) : r(R), g(G), b(B) {};
	Color(uint32_t rgba) {
		r = ((rgba >> 24) & 0xFF) / 255.0f;
		g = ((rgba >> 16) & 0xFF) / 255.0f;
		b = ((rgba >> 8) & 0xFF) / 255.0f;
		a = ((rgba >> 0) & 0xFF) / 255.0f;
	}

	float* ptr(void) { return reinterpret_cast<float*>(this); }
	float& operator[](int index) { return ptr()[index]; }
};
}