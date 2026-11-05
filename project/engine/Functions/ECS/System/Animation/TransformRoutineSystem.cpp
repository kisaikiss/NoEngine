#include "stdafx.h"
#include "TransformRoutineSystem.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Math/MathInclude.h"

namespace NoEngine {
namespace ECS {
using namespace Component;

void TransformRoutineSystem::Update(Registry& registry, float deltaTime) {
	auto view = registry.View<TransformRoutineComponent, TransformComponent>();
	for (auto e : view) {
		auto* routine = registry.GetComponent<TransformRoutineComponent>(e);
		auto* transform = registry.GetComponent<TransformComponent>(e);

		if (!routine->playing) continue;
		const auto* route = routine->GetActiveRoutine();
		if (!route || route->keyframes.size() < 2) {
			routine->playing = false;
			continue;
		}
		if (routine->currentIndex < 0 || routine->currentIndex >= static_cast<int>(route->keyframes.size())) {
			routine->currentIndex = 0;
			routine->elapsed = 0.0f;
		}

		const int count = static_cast<int>(route->keyframes.size());
		int currentIndex = routine->currentIndex;
		int nextIndex = (currentIndex + 1) % count;
		float rawT;
		float time;
		bool routineComplete = false;

		if (route->useGlobalTiming) {
			// 全体時間に一度だけeasingを適用し、その進捗をキーフレーム間に割り当てる。
			const int segmentCount = count - 1;
			routine->elapsed += deltaTime * routine->playbackSpeed;
			rawT = std::clamp(routine->elapsed / std::max(route->globalDuration, 0.0001f), 0.0f, 1.0f);
			routineComplete = rawT >= 1.0f;
			const float routeT = ApplyEasing(route->globalEasing, rawT) * segmentCount;
			currentIndex = std::min(static_cast<int>(routeT), segmentCount - 1);
			nextIndex = currentIndex + 1;
			time = std::clamp(routeT - currentIndex, 0.0f, 1.0f);
			routine->currentIndex = currentIndex;
		} else {
			const auto& to = route->keyframes[nextIndex];
			routine->elapsed += deltaTime * routine->playbackSpeed;
			rawT = std::clamp(routine->elapsed / std::max(to.duration, 0.0001f), 0.0f, 1.0f);
			time = ApplyEasing(to.easing, rawT);
		}

		const auto& from = route->keyframes[currentIndex];
		const auto& to = route->keyframes[nextIndex];

		switch (route->interpolation) {
		case InterpolationType::Linear:
			transform->translate = Easing::Lerp(from.translate, to.translate, time);
			break;
		case InterpolationType::CatmullRom: {
			const auto& prev = route->keyframes[GetRoutinePrevIndex(currentIndex, count, route->loop)];
			const auto& next2 = route->keyframes[GetRoutineNext2Index(nextIndex, count, route->loop)];

			transform->translate = CatmullRomVec3(
				prev.translate, from.translate, to.translate, next2.translate, time);
			break;
		}
		case InterpolationType::Bezier: {
			const auto& prev = route->keyframes[GetRoutinePrevIndex(currentIndex, count, route->loop)];
			const auto& next2 = route->keyframes[GetRoutineNext2Index(nextIndex, count, route->loop)];

			Math::Vector3 ctrl1, ctrl2;
			MakeBezierControlPoints(prev.translate, from.translate, to.translate, next2.translate, ctrl1, ctrl2);

			transform->translate = CubicBezierVec3(from.translate, ctrl1, ctrl2, to.translate, time);
			break;
		}
		case InterpolationType::Step:
			transform->translate = (time >= 1.0f) ? to.translate : from.translate;
			break;
		}

		transform->rotation = Math::Quaternion::Slerp(from.rotation, to.rotation, time);
		transform->scale = Easing::Lerp(from.scale, to.scale, time);

		if (route->useGlobalTiming && routineComplete) {
			routine->elapsed = 0.0f;
			routine->currentIndex = 0;
			if (!route->loop) routine->playing = false;
		} else if (!route->useGlobalTiming && rawT >= 1.0f) {
			routine->elapsed = 0.0f;
			routine->currentIndex = nextIndex;
			if (!route->loop && routine->currentIndex == route->keyframes.size() - 1) {
				routine->playing = false;
			}
		}
	}
}
}
}
#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::TransformRoutineSystem, "TransformRoutineSystem", "Animation")
