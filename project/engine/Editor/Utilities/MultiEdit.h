#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "engine/Editor/Reflection/Reflection.h"
#include "engine/Math/Types/Matrix4x4.h"
#include "externals/nlohmann/json.hpp"

namespace NoEngine::Editor {

std::vector<ECS::Entity> GetEditorSelection(ECS::Registry& registry);
void SetEditorSelection(ECS::Registry& registry, std::span<const ECS::Entity> entities,
	ECS::Entity primary = ECS::INVALID_ENTITY);
std::vector<ECS::Entity> GetTransformSelectionRoots(ECS::Registry& registry,
	std::span<const ECS::Entity> selection, bool is2D);

struct BatchEntityValue {
	ECS::Entity entity;
	std::string name;
	nlohmann::json values;
};
struct BatchSnapshot {
	size_t componentType = 0;
	std::vector<BatchEntityValue> entities;
};

// Undoは生ポインタを保持せず、実行時にEntityとComponentを引き直す。
BatchSnapshot CaptureBatch(ECS::Registry& registry, std::span<const ECS::Entity> entities,
	size_t componentType, const FieldInfo* field = nullptr);
void ApplyBatch(ECS::Registry& registry, const BatchSnapshot& snapshot);
void RecordBatchEdit(ECS::Registry& registry, const BatchSnapshot& before, const BatchSnapshot& after);
void DrawMultiInspector(ECS::Registry& registry, const std::vector<ECS::Entity>& selection);
void ResetPendingMultiEdit();

struct GroupTransformItem {
	Math::Matrix4x4 world;
	ECS::Entity entity;
	Math::Vector3 offset;
	Math::Vector3 scale;
};
enum class GroupTransformOperation { Move, Rotate, Scale };
void ApplyGroupTransform(ECS::Registry& registry, std::span<const GroupTransformItem> items,
	const Math::Matrix4x4& delta, const Math::Vector3& scaleFactor, bool is2D, GroupTransformOperation operation);

}
