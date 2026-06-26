#pragma once
#include "../ISystem.h"
#include "../../Component/Asset/AnimatorComponent.h"
#include "../../Component/Asset/MeshComponent.h" 
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Runtime/PipelineStateObject/ComputePSO.h"
#include "engine/Assets/Model/Skeleton.h"
#include "engine/Assets/Model/Animation.h"

namespace NoEngine {
namespace ECS {

class AnimationSystem :
	public ISystem {
public:
	AnimationSystem();
	void Update(ComputeContext& ctx, Registry& registry, float deltaTime) override;
	void Update(Registry& registry, float deltaTime) override;
private:
	void AnimationUpdate(Registry& registry, float deltaTime);
	void SkeletonUpdate(Component::AnimatorComponent* animeComp, Skeleton* skeleton, Animation* animation);
	void SkeletonDraw(Skeleton* skeleton);
	void SkinUpdate(Skeleton* skeleton, Component::MeshComponent* meshComp);

	void CalculateValue(const NodeAnimation& keyframes, Transform& transform, float time);
	Math::Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
	Math::Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);

	ComputePSO pso_;
	RootSignature rootSignature_;
};

}
}
