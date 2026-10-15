#include "stdafx.h"
#include "TransformRoutineSystem2D.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent2D.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include "engine/Math/MathInclude.h"
#include "engine/Math/TransformRoutineCurves2D.h"

namespace NoEngine {
namespace ECS {
using namespace Component;

void TransformRoutineSystem2D::Update(Registry& registry, float deltaTime) {
	auto view = registry.View<TransformRoutineComponent2D, Transform2DComponent>();
	for (auto e : view) {
		auto* routine = registry.GetComponent<TransformRoutineComponent2D>(e);
		auto* transform = registry.GetComponent<Transform2DComponent>(e);

		if (!routine->playing || routine->keyframes.size() < 2) continue;

		const int count = static_cast<int>(routine->keyframes.size());
		int currentIndex = routine->currentIndex;
		int nextIndex = (currentIndex + 1) % count;
		float rawT;
		float time;
		bool routineComplete = false;

		if (routine->useGlobalTiming) {
			// 全体時間に一度だけeasingを適用し、その進捗をキーフレーム間に割り当てる。
			const int segmentCount = count - 1;
			routine->elapsed += deltaTime * routine->playbackSpeed;
			rawT = std::clamp(routine->elapsed / std::max(routine->globalDuration, 0.0001f), 0.0f, 1.0f);
			routineComplete = rawT >= 1.0f;
			const float routeT = ApplyEasing(routine->globalEasing, rawT) * segmentCount;
			currentIndex = std::min(static_cast<int>(routeT), segmentCount - 1);
			nextIndex = currentIndex + 1;
			time = std::clamp(routeT - currentIndex, 0.0f, 1.0f);
			routine->currentIndex = currentIndex;
		} else {
			const auto& to = routine->keyframes[nextIndex];
			routine->elapsed += deltaTime * routine->playbackSpeed;
			rawT = std::clamp(routine->elapsed / std::max(to.duration, 0.0001f), 0.0f, 1.0f);
			time = ApplyEasing(to.easing, rawT);
		}

		const auto& from = routine->keyframes[currentIndex];
		const auto& to = routine->keyframes[nextIndex];

		switch (routine->interpolation) {
		case InterpolationType::Linear:
			transform->translate = Easing::Lerp(from.translate, to.translate, time);
			break;
		case InterpolationType::CatmullRom: {
			const auto& prev = routine->keyframes[GetRoutinePrevIndex(currentIndex, count, routine->loop)];
			const auto& next2 = routine->keyframes[GetRoutineNext2Index(nextIndex, count, routine->loop)];

			transform->translate = CatmullRomVec2(
				prev.translate, from.translate, to.translate, next2.translate, time);
			break;
		}
		case InterpolationType::Bezier: {
			const auto& prev = routine->keyframes[GetRoutinePrevIndex(currentIndex, count, routine->loop)];
			const auto& next2 = routine->keyframes[GetRoutineNext2Index(nextIndex, count, routine->loop)];

			Math::Vector2 ctrl1, ctrl2;
			MakeBezierControlPoints2D(prev.translate, from.translate, to.translate, next2.translate, ctrl1, ctrl2);

			transform->translate = CubicBezierVec2(from.translate, ctrl1, ctrl2, to.translate, time);
			break;
		}
		case InterpolationType::Step:
			transform->translate = (time >= 1.0f) ? to.translate : from.translate;
			break;
		}

		transform->rotation = Easing::Lerp(from.rotation, to.rotation, time);
		transform->scale = Easing::Lerp(from.scale, to.scale, time);

		if (routine->useGlobalTiming && routineComplete) {
			routine->elapsed = 0.0f;
			routine->currentIndex = 0;
			if (!routine->loop) routine->playing = false;
		} else if (!routine->useGlobalTiming && rawT >= 1.0f) {
			routine->elapsed = 0.0f;
			routine->currentIndex = nextIndex;
			if (!routine->loop && routine->currentIndex == routine->keyframes.size() - 1) {
				routine->playing = false;
			}
		}
	}
}
}
}

#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::TransformRoutineSystem2D, "TransformRoutineSystem2D", "Animation")
