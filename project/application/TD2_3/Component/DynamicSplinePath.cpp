#include "stdafx.h"
#include "DynamicSplinePath.h"
#include "../Random/RandomFanc.h"

No::Vector3 CatmullRom(const No::Vector3& p0, const No::Vector3& p1, const No::Vector3& p2, const No::Vector3& p3, float t) {
	float t2 = t * t;
	float t3 = t2 * t;

	return 0.5f * (
		(2.0f * p1) +
		(-p0 + p2) * t +
		(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
		(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
		);

}

No::Vector3 GetRandomPosition(No::Vector3 prePos, float minLength) {
	No::Vector3 result{};
	for (uint32_t i = 0; i < 10; i++) {
		result = No::Vector3(RNG::GetRandomVal(-4.f, 4.f), RNG::GetRandomVal(-4.f, 4.f), 0.f);
		prePos -= result;
		if (prePos.Length() > minLength) {
			break;
		}
	}


	return result;
}
