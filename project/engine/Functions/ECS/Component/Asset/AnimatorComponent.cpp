#include "AnimatorComponent.h"
#include "engine/Assets/Model/ModelSaver.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {

AnimatorSkinningResources& AnimatorComponent::EnsureSkinningResources(const Mesh& mesh) {
	if (!runtime.skinning || runtime.skinning.use_count() > 1) {
		auto skinning = std::make_shared<AnimatorSkinningResources>();
		// 全頂点をComputeで上書きする。初期転送によるGENERIC_READへの遷移を避ける。
		skinning->vertexBuffer.Create(L"Animator vertices",
			static_cast<uint32_t>(mesh.vertices.size()), sizeof(Vertex));
		skinning->paletteResource.Create(L"Animator joints",
			static_cast<uint32_t>(runtime.palette.size()), sizeof(SkeletonWell));
		runtime.skinning = std::move(skinning);
	}
	return *runtime.skinning;
}

StructuredBuffer& AnimatorComponent::GetVertexBuffer(Mesh& mesh) const {
	return enableSkinning && runtime.skinning ? runtime.skinning->vertexBuffer : mesh.useVertexBuffer;
}

RaytracingMesh* AnimatorComponent::GetRaytracingMesh(Mesh& mesh) const {
	return enableSkinning && runtime.skinning ? runtime.skinning->raytracingMesh.get() : mesh.raytracingMesh.get();
}

std::vector<std::string> AnimatorComponent::GetAnimationNames() const {
	std::vector<std::string> names;
	names.reserve(animationHandles.size());
	for (const auto handle : animationHandles) {
		if (const auto* animation = ModelSaver::Get().GetAnimation(handle)) {
			names.push_back(animation->name);
		}
	}
	return names;
}

bool AnimatorComponent::SetAnimation(const std::string& name) {
	if (name.empty()) return false;
	for (size_t index = 0; index < animationHandles.size(); ++index) {
		const auto* animation = ModelSaver::Get().GetAnimation(animationHandles[index]);
		if (animation && animation->name == name) {
			currentAnimation = static_cast<uint32_t>(index);
			if (currentAnimation != preCurrentAnimation) OnAnimationChanged();
			return true;
		}
	}
	return false;
}

const Animation* AnimatorComponent::GetCurrentAnimation() const {
	if (currentAnimation >= animationHandles.size()) return nullptr;
	return ModelSaver::Get().GetAnimation(animationHandles[currentAnimation]);
}

float AnimatorComponent::GetDuration() const {
	const auto* animation = GetCurrentAnimation();
	return animation && std::isfinite(animation->duration) && animation->duration > 0.0f
		? animation->duration : 0.0f;
}

float AnimatorComponent::GetNormalizedTime() const {
	const float duration = GetDuration();
	return duration > 0.0f && std::isfinite(time) ? std::clamp(time / duration, 0.0f, 1.0f) : 0.0f;
}

bool AnimatorComponent::IsPlaying() const {
	return playbackState == AnimationPlaybackState::Playing && GetCurrentAnimation() != nullptr;
}

void AnimatorComponent::OnAnimationChanged() {
	previousAnimation = preCurrentAnimation;
	previousAnimationTime = time;
	isBlending = blendDuration > 0.0f;
	blendTimer = 0.0f;
	time = animationSpeedMagnification < 0.0f ? GetDuration() : 0.0f;
	preCurrentAnimation = currentAnimation;
	playbackState = AnimationPlaybackState::Playing;
}

bool AnimatorComponent::Play(const std::string& name, bool shouldLoop) {
	const uint32_t oldIndex = preCurrentAnimation;
	if (!SetAnimation(name)) return false;
	loop = shouldLoop;
	if (currentAnimation == oldIndex) return Restart();
	return true;
}

bool AnimatorComponent::Play() { return Restart(); }

bool AnimatorComponent::Restart() {
	if (!GetCurrentAnimation()) return false;
	if (currentAnimation != preCurrentAnimation) OnAnimationChanged();
	time = animationSpeedMagnification < 0.0f ? GetDuration() : 0.0f;
	isBlending = false;
	blendTimer = 0.0f;
	playbackState = AnimationPlaybackState::Playing;
	return true;
}

void AnimatorComponent::Pause() {
	if (GetCurrentAnimation() && currentAnimation != preCurrentAnimation) OnAnimationChanged();
	if (IsPlaying()) playbackState = AnimationPlaybackState::Paused;
}

bool AnimatorComponent::Resume() {
	if (!GetCurrentAnimation() || playbackState != AnimationPlaybackState::Paused) return false;
	if (currentAnimation != preCurrentAnimation) OnAnimationChanged();
	playbackState = AnimationPlaybackState::Playing;
	return true;
}

void AnimatorComponent::Stop() {
	preCurrentAnimation = currentAnimation;
	time = 0.0f;
	isBlending = false;
	blendTimer = 0.0f;
	playbackState = AnimationPlaybackState::Stopped;
}

bool AnimatorComponent::Seek(float seconds) {
	if (!GetCurrentAnimation() || !std::isfinite(seconds)) return false;
	preCurrentAnimation = currentAnimation;
	time = std::clamp(seconds, 0.0f, GetDuration());
	isBlending = false;
	blendTimer = 0.0f;
	if (IsFinished()) playbackState = AnimationPlaybackState::Paused;
	return true;
}

void AnimatorComponent::UpdatePlayback(float deltaTime) {
	if (currentAnimation != preCurrentAnimation) OnAnimationChanged();
	const float duration = GetDuration();
	if (!std::isfinite(time)) time = 0.0f;
	if (!IsPlaying()) return;
	if (duration <= 0.0f) {
		time = 0.0f;
		if (!loop) {
			playbackState = AnimationPlaybackState::Finished;
			isBlending = false;
		}
		return;
	}
	const double dt = std::isfinite(deltaTime) && deltaTime > 0.0f ? deltaTime : 0.0;
	const double speed = std::isfinite(animationSpeedMagnification) ? animationSpeedMagnification : 0.0;
	const double step = dt * speed;
	const double next = static_cast<double>(time) + step;
	if (loop) {
		if (step != 0.0) {
			double wrapped = std::fmod(next, static_cast<double>(duration));
			if (wrapped < 0.0) wrapped += duration;
			time = static_cast<float>(wrapped);
		} else {
			time = std::clamp(time, 0.0f, duration);
		}
	} else {
		time = static_cast<float>(std::clamp(next, 0.0, static_cast<double>(duration)));
		if ((speed > 0.0 && next >= duration) || (speed < 0.0 && next <= 0.0)) {
			playbackState = AnimationPlaybackState::Finished;
			isBlending = false;
		}
	}
}

}

REFLECT_STRUCT_BEGIN(NoEngine::Component::AnimatorComponent, "Asset")
REFLECT_FIELD(currentAnimation),
REFLECT_FIELD(animationSpeedMagnification),
REFLECT_FIELD(drawSkeleton),
REFLECT_FIELD(enableSkinning),
REFLECT_FIELD(loop),
REFLECT_FIELD(blendDuration)
REFLECT_STRUCT_END(NoEngine::Component::AnimatorComponent)
