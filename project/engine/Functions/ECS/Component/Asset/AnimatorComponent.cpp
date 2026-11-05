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

bool AnimatorComponent::AddEventMarker(const std::string& animationName, float seconds,
	const std::string& name, const std::string& parameter) {
	if (animationName.empty() || name.empty() || !std::isfinite(seconds) || seconds < 0.0f) return false;
	for (const auto handle : animationHandles) {
		const auto* animation = ModelSaver::Get().GetAnimation(handle);
		if (animation && animation->name == animationName && std::isfinite(animation->duration) &&
			animation->duration >= seconds) {
			animationEvents.push_back({animationName, seconds, name, parameter});
			return true;
		}
	}
	return false;
}

std::vector<TriggeredAnimationEvent> AnimatorComponent::ConsumeEvents() {
	auto events = std::move(triggeredEvents);
	triggeredEvents.clear();
	return events;
}

void AnimatorComponent::CollectAnimationEvents(double step, bool reverse) {
	const auto* animation = GetCurrentAnimation();
	if (!animation) return;
	const double duration = GetDuration();
	const double start = std::clamp(static_cast<double>(time), 0.0, duration);
	const double travel = loop && duration > 0.0 ? std::abs(step)
		: std::abs(std::clamp(start + step, 0.0, duration) - start);
	struct Crossing { double distance; size_t index; };
	// 周回境界では、正再生は終端→先頭、逆再生は先頭→終端の順に発火する。
	auto later = [&](const Crossing& a, const Crossing& b) {
		if (a.distance != b.distance) return a.distance > b.distance;
		const float aTime = animationEvents[a.index].time;
		const float bTime = animationEvents[b.index].time;
		if (aTime != bTime) return reverse ? aTime > bTime : aTime < bTime;
		return a.index > b.index;
	};
	std::priority_queue<Crossing, std::vector<Crossing>, decltype(later)> crossings(later);
	double total = 0.0;
	for (size_t index = 0; index < animationEvents.size(); ++index) {
		const auto& marker = animationEvents[index];
		if (marker.animationName != animation->name || marker.name.empty() ||
			!std::isfinite(marker.time) || marker.time < 0.0f || marker.time > duration) continue;
		double distance = reverse ? start - marker.time : marker.time - start;
		if (step == 0.0 || duration == 0.0) {
			if (!eventStartPending || distance != 0.0) continue;
		} else if (loop) {
			if (distance < 0.0 || (distance == 0.0 && !eventStartPending)) distance += duration;
		} else if (distance < 0.0 || (distance == 0.0 && !eventStartPending)) {
			continue;
		}
		if (distance > travel) continue;
		crossings.push({distance, index});
		total += loop && duration > 0.0 && step != 0.0
			? std::floor((travel - distance) / duration) + 1.0 : 1.0;
	}
	while (!crossings.empty() && triggeredEvents.size() < kMaxEventsPerUpdate) {
		const Crossing crossing = crossings.top();
		crossings.pop();
		const auto& marker = animationEvents[crossing.index];
		triggeredEvents.push_back({marker.animationName, marker.time, marker.name, marker.parameter, reverse});
		if (loop && duration > 0.0 && step != 0.0 && crossing.distance + duration <= travel) {
			crossings.push({crossing.distance + duration, crossing.index});
		}
	}
	// 異常に大きい経過時間でも、イベント生成でフリーズさせない。
	const double dropped = std::max(0.0, total - static_cast<double>(triggeredEvents.size()));
	droppedEventCount = dropped >= static_cast<double>(std::numeric_limits<uint64_t>::max())
		? std::numeric_limits<uint64_t>::max() : static_cast<uint64_t>(dropped);
	eventStartPending = false;
}

void AnimatorComponent::OnAnimationChanged() {
	previousAnimation = preCurrentAnimation;
	previousAnimationTime = time;
	isBlending = blendDuration > 0.0f;
	blendTimer = 0.0f;
	time = animationSpeedMagnification < 0.0f ? GetDuration() : 0.0f;
	preCurrentAnimation = currentAnimation;
	playbackState = AnimationPlaybackState::Playing;
	triggeredEvents.clear();
	droppedEventCount = 0;
	eventStartPending = true;
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
	triggeredEvents.clear();
	droppedEventCount = 0;
	eventStartPending = true;
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
	triggeredEvents.clear();
	droppedEventCount = 0;
	eventStartPending = false;
}

bool AnimatorComponent::Seek(float seconds) {
	if (!GetCurrentAnimation() || !std::isfinite(seconds)) return false;
	preCurrentAnimation = currentAnimation;
	time = std::clamp(seconds, 0.0f, GetDuration());
	isBlending = false;
	blendTimer = 0.0f;
	if (IsFinished()) playbackState = AnimationPlaybackState::Paused;
	triggeredEvents.clear();
	droppedEventCount = 0;
	eventStartPending = false;
	return true;
}

void AnimatorComponent::UpdatePlayback(float deltaTime) {
	if (currentAnimation != preCurrentAnimation) OnAnimationChanged();
	const float duration = GetDuration();
	if (!std::isfinite(time)) time = 0.0f;
	if (!IsPlaying()) return;
	const double dt = std::isfinite(deltaTime) && deltaTime > 0.0f ? deltaTime : 0.0;
	const double speed = std::isfinite(animationSpeedMagnification) ? animationSpeedMagnification : 0.0;
	const double step = dt * speed;
	time = std::clamp(time, 0.0f, duration);
	if (duration <= 0.0f) {
		CollectAnimationEvents(0.0, speed < 0.0);
		time = 0.0f;
		if (!loop) {
			playbackState = AnimationPlaybackState::Finished;
			isBlending = false;
		}
		return;
	}
	const double next = static_cast<double>(time) + step;
	if (loop) {
		if (step != 0.0) {
			double wrapped = std::fmod(next, static_cast<double>(duration));
			if (wrapped < 0.0) wrapped += duration;
			float sampledTime = static_cast<float>(wrapped);
			// floatのポーズ時刻に丸めて到達したマーカーも、取りこぼさず発火する。
			const double rounded = sampledTime >= duration ? duration : static_cast<double>(sampledTime);
			const double eventStep = step + (rounded - wrapped);
			CollectAnimationEvents(speed < 0.0 ? std::min(0.0, eventStep) : std::max(0.0, eventStep), speed < 0.0);
			if (sampledTime >= duration) sampledTime = 0.0f;
			time = sampledTime;
		} else {
			CollectAnimationEvents(0.0, speed < 0.0);
			time = std::clamp(time, 0.0f, duration);
		}
	} else {
		const float sampledTime = static_cast<float>(std::clamp(next, 0.0, static_cast<double>(duration)));
		CollectAnimationEvents(static_cast<double>(sampledTime) - time, speed < 0.0);
		time = sampledTime;
		if ((speed > 0.0 && time >= duration) || (speed < 0.0 && time <= 0.0)) {
			playbackState = AnimationPlaybackState::Finished;
			isBlending = false;
		}
	}
}

}

REFLECT_STRUCT_BEGIN(NoEngine::Component::AnimationEventMarker)
REFLECT_FIELD(animationName),
REFLECT_FIELD(time),
REFLECT_FIELD(name),
REFLECT_FIELD(parameter)
REFLECT_STRUCT_END(NoEngine::Component::AnimationEventMarker)

REFLECT_STRUCT_BEGIN(NoEngine::Component::AnimatorComponent, "Asset")
REFLECT_FIELD(currentAnimation),
REFLECT_FIELD(animationSpeedMagnification),
REFLECT_FIELD(drawSkeleton),
REFLECT_FIELD(enableSkinning),
REFLECT_FIELD(loop),
REFLECT_STRUCT_ARRAY_FIELD(animationEvents),
REFLECT_FIELD(blendDuration)
REFLECT_STRUCT_END(NoEngine::Component::AnimatorComponent)
