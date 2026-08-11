#pragma once
#include "engine/NoEngine.h"

class FollowCameraSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	std::vector<int> candidateIndices_; // BVH問い合わせ結果の使い回し用
};

