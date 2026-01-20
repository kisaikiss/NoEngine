#include "Easing.h"

namespace NoEngine {
namespace Easing {
Vector3 Lerp(Vector3 start, Vector3 end, float t) {
	Vector3 result;
	result.x = std::lerp(start.x, end.x, t);
	result.y = std::lerp(start.y, end.y, t);
	result.z = std::lerp(start.z, end.z, t);
	return result;
}

Quaternion Lerp(Quaternion start, Quaternion end, float t) {
	Quaternion result;
	result.x = std::lerp(start.x, end.x, t);
	result.y = std::lerp(start.y, end.y, t);
	result.z = std::lerp(start.z, end.z, t);
	result.w = std::lerp(start.w, end.w, t);
	return result;
}
}
}
