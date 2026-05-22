#include "Random.h"

namespace NoEngine {
namespace Random {
namespace {
std::random_device seedGenerator;
std::mt19937 randomEngine(seedGenerator());
std::uniform_real_distribution<float> normalizeDistribution(-1.0f, 1.0f);
}

float GetRandomValNormalized() {
	return normalizeDistribution(randomEngine);
}
float GetRandomVal(float min, float max) {
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine);
}

Math::Vector3 GetRandomVal(const Math::Vector3& min, const Math::Vector3& max) {
	return Math::Vector3(GetRandomVal(min.x,max.x), GetRandomVal(min.y, max.y), GetRandomVal(min.z, max.z));
}
Math::Vector2 GetRandomVal(const Math::Vector2& min, const Math::Vector2& max) {
	return Math::Vector2(GetRandomVal(min.x, max.x), GetRandomVal(min.y, max.y));
}
}
}