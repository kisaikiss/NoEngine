#include "MathUtility.h"

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
}
}