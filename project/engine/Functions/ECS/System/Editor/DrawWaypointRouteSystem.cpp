#include "stdafx.h"
#include "DrawWaypointRouteSystem.h"
#include "engine/Editor/EditTag.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent.h"
#include "engine/Functions/Renderer/Primitive.h"

namespace NoEngine {
namespace ECS {

using namespace Editor;
using namespace Component;

namespace {

// DrawManipulatorSystem::GetParentWorld3Dと同じロジック。
// TransformRoutineSystemはkeyframeの値をt->translate等へそのまま書き込むため、
// keyframeはエンティティ自身の現在のワールド行列ではなく、t->translateと同じ空間＝
// 親のワールド行列（親がなければ単位行列）を基準にした値として扱う必要がある。
// DrawManipulatorSystemと重複しているので、共有ヘッダに切り出しても良い。
Math::Matrix4x4 GetParentWorld3D(Registry& registry, TransformComponent* t) {
	if (t->parent != ECS::INVALID_ENTITY) {
		if (auto* parentTransform = registry.GetComponent<TransformComponent>(t->parent)) {
			return parentTransform->MakeAffineMatrix4x4(registry);
		}
	}
	return Math::Matrix4x4::IDENTITY;
}

} // namespace

void DrawWaypointRouteSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	const Math::Color kLineColor{ 0.2f, 0.9f, 1.0f, 1.0f };

	// EditTagを持つンティティのルートを常時表示する。
	auto view = registry.View<TransformRoutineComponent, TransformComponent, EditTag, EditSelectedTag>();
	for (auto e : view) {
		auto* routine = registry.GetComponent<TransformRoutineComponent>(e);
		auto* t = registry.GetComponent<TransformComponent>(e);
		if (routine->keyframes.size() < 2) {
			continue;
		}

		Math::Matrix4x4 parentWorld = GetParentWorld3D(registry, t);

		const size_t count = routine->keyframes.size();
		// loopがtrueなら最後のwaypointから最初のwaypointへも線を引いて輪を閉じる
		const size_t segments = routine->loop ? count : count - 1;

		for (size_t i = 0; i < segments; ++i) {
			const auto& from = routine->keyframes[i];
			const auto& to = routine->keyframes[(i + 1) % count];

			Math::Vector3 worldFrom = (from.MakeAffineMatrix4x4() * parentWorld).GetTranslate();
			Math::Vector3 worldTo = (to.MakeAffineMatrix4x4() * parentWorld).GetTranslate();

			DebugPrimitive::DrawLine(worldFrom, worldTo, kLineColor);
		}
	}
}

}
}