#include "AnimationSystem.h"
#include "../Component/MeshComponent.h"

namespace NoEngine {
namespace ECS {

void AnimationSystem::Update(Registry& registry, float deltaTime) {
	AnimationUpdate(registry, deltaTime);
}

void AnimationSystem::AnimationUpdate(Registry& registry, float deltaTime) {
	auto view = registry.View<Component::AnimationComponent, Component::MeshComponent>();

	for (auto entity : view) {
		auto* animeComp = registry.GetComponent<Component::AnimationComponent>(entity);
		//auto* meshComp = registry.GetComponent<Component::MeshComponent>(entity);
		animeComp->time += deltaTime;
		animeComp->time = std::fmod(animeComp->time, animeComp->animation->duration);
		if (animeComp->skeleton) {
			SkeletonUpdate(animeComp);
		}

	}
}

void AnimationSystem::SkeletonUpdate(Component::AnimationComponent* animeComp) {

	for (Joint& joint : animeComp->skeleton->joints) {

		if (auto it = animeComp->animation->nodeAnimations.find(joint.name); it != animeComp->animation->nodeAnimations.end()) {
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
			return Easing::Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}
	// ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;
}
}
}