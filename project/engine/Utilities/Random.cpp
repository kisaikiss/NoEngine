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
}
}