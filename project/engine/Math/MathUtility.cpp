#include "MathUtility.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Math {
Vector4 Clamp(const Vector4& v, const Vector4& min, const Vector4& max) {
	return {
		std::clamp(v.x,min.x,max.x),
		std::clamp(v.y,min.y,max.y),
		std::clamp(v.z,min.z,max.z),
		std::clamp(v.w,min.w,max.w)
	};
}
Vector3 Clamp(const Vector3& v, const Vector3& min, const Vector3& max) {
	return {
			std::clamp(v.x,min.x,max.x),
			std::clamp(v.y,min.y,max.y),
			std::clamp(v.z,min.z,max.z)
	};
}
Vector2 Clamp(const Vector2& v, const Vector2& min, const Vector2& max) {
	return {
		std::clamp(v.x,min.x,max.x),
		std::clamp(v.y,min.y,max.y)
	};
}

Vector2 WorldToScreen(const Vector3& p, const Matrix4x4& view, const Matrix4x4& projection) {
	Vector3 ndc = (view * projection).Transform(p);

	Vector2 screenSize = GraphicsCore::GetWindowSize();

	float screenX = (ndc.x + 1.0f) * 0.5f * screenSize.x;
	float screenY = (1.0f - ndc.y) * 0.5f * screenSize.y;

	return Vector2(screenX, screenY);
}
}
}