#pragma once
#include "ISystem.h"
#include "../Component/AnimatorComponent.h"
#include "../Component/MeshComponent.h" 

namespace NoEngine {
namespace ECS {

class AnimationSystem :
	public ISystem {
public:
	void Update(Registry& registry, float deltaTime) override;
private:
	void AnimationUpdate(Registry& registry, float deltaTime);
	void SkeletonUpdate(Component::AnimatorComponent* animeComp);
	void SkeletonDraw(Component::AnimatorComponent* animeComp);
	void SKinUpdate(Component::AnimatorComponent* animeComp, Component::MeshComponent* meshComp);

	void CalculateValue(const NodeAnimation& keyframes, Transform& transform, float time);
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);
};

}
}
