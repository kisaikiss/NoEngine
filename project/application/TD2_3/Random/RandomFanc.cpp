#include "RandomFanc.h"
#include <random>

namespace {
std::random_device seedGenerator;
std::mt19937 randomEngine(seedGenerator());
std::uniform_real_distribution<float> normalizeDistribution(-1.0f, 1.0f);
}

float RNG::GetRandomValNormalized() {
    return normalizeDistribution(randomEngine);;
}

float RNG::GetRandomVal(float min, float max) {
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine);
}

NoEngine::Vector3 RNG::GetRandomVector3(float min, float max) {
	return NoEngine::Vector3(GetRandomVal(min, max), GetRandomVal(min, max), GetRandomVal(min, max));
}
