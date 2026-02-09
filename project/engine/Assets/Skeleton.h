#pragma once
#include "engine/Math/MathInclude.h"

namespace NoEngine {
struct JointVertex {
	float weights[4];
	uint32_t jointIndices[4];
};

struct Joint {
	Transform transform;
	Math::Matrix4x4 localMatrix;
	Math::Matrix4x4 skeletonSpaceMatrix;
	std::string name;
	std::vector<int32_t> children;
	int32_t index;
	std::optional<int32_t> parent;
};

struct Skeleton {
	int32_t root;
	std::map<std::string, int32_t> jointMap;
	std::vector<Joint> joints;
	std::vector<Math::Matrix4x4> inverseBindPoseMatrices;
};
}