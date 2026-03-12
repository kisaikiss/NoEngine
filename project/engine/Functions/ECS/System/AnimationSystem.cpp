#include "AnimationSystem.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"
#include "engine/Runtime/PipelineStateObject/ComputePSO.h"
#include "engine/Functions/Shader/ShaderModule.h"


namespace NoEngine {
namespace ECS {
using namespace Math;


AnimationSystem::AnimationSystem() {
	ShaderModule animationCS(ShaderStage::Compute, L"resources/engine/Shaders/Compute/Skinning.CS.hlsl", L"cs_6_0");
}

void AnimationSystem::Update(ComputeContext& ctx, Registry& registry, float deltaTime) {
	(void)ctx;
	Update(registry, deltaTime);
}

void AnimationSystem::Update(Registry& registry, float deltaTime) {
	AnimationUpdate(registry, deltaTime);
}

void AnimationSystem::AnimationUpdate(Registry& registry, float deltaTime) {
	auto view = registry.View<Component::AnimatorComponent, Component::MeshComponent>();

	for (auto entity : view) {
		auto* animeComp = registry.GetComponent<Component::AnimatorComponent>(entity);
		auto* meshComp = registry.GetComponent<Component::MeshComponent>(entity);
		if (!animeComp->skeleton || !meshComp->mesh) continue;
		animeComp->time += deltaTime;
		uint32_t currentAnimation = animeComp->currentAnimation;
		animeComp->time = std::fmod(animeComp->time, animeComp->animation[currentAnimation].duration);
		if (animeComp->skeleton) {
			SkeletonUpdate(animeComp);
			SkeletonDraw(animeComp);
			SKinUpdate(animeComp, meshComp);
		}
	}
}

void AnimationSystem::SkeletonUpdate(Component::AnimatorComponent* animeComp) {
	uint32_t currentAnimation = animeComp->currentAnimation;
	for (Joint& joint : animeComp->skeleton->joints) {
		if (auto it = animeComp->animation[currentAnimation].nodeAnimations.find(joint.name); it != animeComp->animation[currentAnimation].nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			CalculateValue(rootNodeAnimation, joint.transform, animeComp->time);
		}

		joint.localMatrix = joint.transform.MakeAffineMatrix4x4();
		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * animeComp->skeleton->joints[*joint.parent].skeletonSpaceMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}

	}
}

void AnimationSystem::SkeletonDraw(Component::AnimatorComponent* animeComp) {

	if (!animeComp->drawSkeleton) return;

	std::vector<Joint>& joints = animeComp->skeleton->joints;
	for (Joint& joint : joints) {
		if (joint.parent.has_value()) {
			const Joint& parentJoint = joints[joint.parent.value()];

			Vector3 start = parentJoint.skeletonSpaceMatrix.GetTranslate();
			Vector3 end = joint.skeletonSpaceMatrix.GetTranslate();

			Primitive::DrawLine(start, end, Color::WHITE);
		}
	}
}

void AnimationSystem::SKinUpdate(Component::AnimatorComponent* animeComp, Component::MeshComponent* meshComp) {
	for (size_t jointIndex = 0; jointIndex < animeComp->skeleton->joints.size(); jointIndex++) {
		meshComp->mesh->mappedPalette[jointIndex].skeletonSpaceMatrix =
			animeComp->skeleton->inverseBindPoseMatrices[jointIndex] * animeComp->skeleton->joints[jointIndex].skeletonSpaceMatrix;
		meshComp->mesh->mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			MathCalculations::Transpose(MathCalculations::Inverse(meshComp->mesh->mappedPalette[jointIndex].skeletonSpaceMatrix));

		memcpy(meshComp->mesh->paletteUpload.Map(), meshComp->mesh->mappedPalette.data(), sizeof(SkeletonWell) * meshComp->mesh->mappedPalette.size());
		
	}
}

void AnimationSystem::CalculateValue(const NodeAnimation& keyframes, Transform& transform, float time) {
	transform.translate = CalculateValue(keyframes.translate.keyframes, time);
	transform.rotation = CalculateValue(keyframes.rotation.keyframes, time);
	transform.scale = CalculateValue(keyframes.scale.keyframes, time);
}

Vector3 AnimationSystem::CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
	if (keyframes.empty()) {
		return Vector3();
	}
	if (keyframes.size() == 1 || time <= keyframes[0].time) { // キーが1つか、時刻がキーフレーム前なら最初の値とする
		return keyframes[0].value;
	}
	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		// indexとnext Indexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			// 範囲内を補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Easing::Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}
	// ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;
}


Quaternion AnimationSystem::CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
	if (keyframes.empty()) {
		return Quaternion();
	}
	if (keyframes.size() == 1 || time <= keyframes[0].time) { // キーが1つか、時刻がキーフレーム前なら最初の値とする
		return keyframes[0].value;
	}
	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		// indexとnext Indexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			// 範囲内を補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return MathCalculations::Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}
	// ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;
}


}
}