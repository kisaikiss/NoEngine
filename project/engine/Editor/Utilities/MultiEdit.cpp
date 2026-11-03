#include "stdafx.h"
#include "MultiEdit.h"
#include "engine/Editor/Commands/EditTag.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Functions/Command/EditCommand/FunctionCommand.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include <algorithm>
#include <cmath>

namespace NoEngine::Editor {

std::vector<ECS::Entity> GetEditorSelection(ECS::Registry& registry) {
	std::vector<ECS::Entity> result;
	for (auto e : registry.View<EditMultiSelectedTag, EditTag>()) result.push_back(e);
	for (auto e : registry.View<EditSelectedTag, EditTag>()) {
		if (std::find(result.begin(), result.end(), e) == result.end()) result.push_back(e);
	}
	std::sort(result.begin(), result.end());
	return result;
}

void SetEditorSelection(ECS::Registry& registry, std::span<const ECS::Entity> entities, ECS::Entity primary) {
	std::vector<ECS::Entity> old;
	for (auto e : registry.View<EditSelectedTag>()) old.push_back(e);
	for (auto e : old) registry.RemoveComponent<EditSelectedTag>(e);
	old.clear();
	for (auto e : registry.View<EditMultiSelectedTag>()) old.push_back(e);
	for (auto e : old) registry.RemoveComponent<EditMultiSelectedTag>(e);
	if (entities.empty()) return;
	if (std::find(entities.begin(), entities.end(), primary) == entities.end()) primary = entities.back();
	if (registry.Has<EditTag>(primary)) registry.AddComponent<EditSelectedTag>(primary);
	for (auto e : entities) if (registry.Has<EditTag>(e)) registry.AddComponent<EditMultiSelectedTag>(e);
}

std::vector<ECS::Entity> GetTransformSelectionRoots(ECS::Registry& registry,
	std::span<const ECS::Entity> selection, bool is2D) {
	std::vector<ECS::Entity> roots;
	for (auto e : selection) {
		auto parentOf = [&](ECS::Entity current) {
			if (is2D) {
				auto* t = registry.GetComponent<Component::Transform2DComponent>(current);
				return t ? t->parent : ECS::INVALID_ENTITY;
			}
			auto* t = registry.GetComponent<Component::TransformComponent>(current);
			return t ? t->parent : ECS::INVALID_ENTITY;
		};
		bool covered = false;
		std::set<ECS::Entity> visited;
		for (auto parent = parentOf(e); parent != ECS::INVALID_ENTITY && visited.insert(parent).second; parent = parentOf(parent)) {
			if (std::find(selection.begin(), selection.end(), parent) != selection.end()) { covered = true; break; }
		}
		if (!covered) roots.push_back(e);
	}
	return roots;
}

BatchSnapshot CaptureBatch(ECS::Registry& registry, std::span<const ECS::Entity> entities,
	size_t componentType, const FieldInfo* field) {
	BatchSnapshot snapshot;
	snapshot.componentType = componentType;
	const auto* type = ComponentRegistry::FindByTypeID(componentType);
	if (!type) return snapshot;
	for (auto e : entities) {
		auto* component = static_cast<uint8_t*>(registry.GetComponent(componentType, e));
		const auto* tag = registry.GetComponent<EditTag>(e);
		if (!component || !tag) continue;
		BatchEntityValue entry{ e, tag->name, nlohmann::json::object() };
		for (const auto& f : type->fields) {
			if ((field && &f != field) || (!field && f.type == FieldType::Entity)) continue;
			WriteFieldToJson(registry, entry.values, f, component + f.offset);
		}
		snapshot.entities.push_back(std::move(entry));
	}
	return snapshot;
}

void ApplyBatch(ECS::Registry& registry, const BatchSnapshot& snapshot) {
	const auto* type = ComponentRegistry::FindByTypeID(snapshot.componentType);
	if (!type) return;
	for (const auto& entry : snapshot.entities) {
		auto entity = entry.entity;
		const auto* tag = registry.GetComponent<EditTag>(entity);
		if (!tag || tag->name != entry.name) entity = FindEntityByName(registry, entry.name);
		auto* component = static_cast<uint8_t*>(registry.GetComponent(snapshot.componentType, entity));
		if (!component) continue;
		for (const auto& field : type->fields) {
			if (entry.values.contains(field.name)) ReadFieldFromJson(registry, entry.values, field, component + field.offset);
		}
	}
}

void RecordBatchEdit(ECS::Registry& registry, const BatchSnapshot& before, const BatchSnapshot& after) {
	bool changed = before.entities.size() != after.entities.size();
	for (size_t i = 0; !changed && i < before.entities.size(); ++i) changed = before.entities[i].values != after.entities[i].values;
	if (!changed) return;
	EditorCommandOperator::AddCommand(std::make_unique<Command::FunctionCommand>(
		[&registry, after] { ApplyBatch(registry, after); }, [&registry, before] { ApplyBatch(registry, before); }, "Batch Edit"));
}

void ApplyGroupTransform(ECS::Registry& registry, std::span<const GroupTransformItem> items,
	const Math::Matrix4x4& delta, const Math::Vector3& scaleFactor, bool is2D, GroupTransformOperation operation) {
	using namespace Component;
	for (const auto& item : items) {
		auto* t = registry.GetComponent<TransformComponent>(item.entity);
		auto* t2 = registry.GetComponent<Transform2DComponent>(item.entity);
		if ((is2D && !t2) || (!is2D && !t)) continue;
		auto local = item.world * delta;
		local.m[3][0] -= item.offset.x;
		local.m[3][1] -= item.offset.y;
		const auto parent = is2D ? t2->parent : t->parent;
		if (parent != ECS::INVALID_ENTITY) {
			auto parentWorld = Math::Matrix4x4::IDENTITY;
			if (is2D) { if (auto* p = registry.GetComponent<Transform2DComponent>(parent)) parentWorld = p->MakeAffineMatrix4x4(registry); }
			else { if (auto* p = registry.GetComponent<TransformComponent>(parent)) parentWorld = p->MakeAffineMatrix4x4(registry); }
			const Math::Vector3 x{parentWorld.m[0][0],parentWorld.m[0][1],parentWorld.m[0][2]};
			const Math::Vector3 y{parentWorld.m[1][0],parentWorld.m[1][1],parentWorld.m[1][2]};
			const Math::Vector3 z{parentWorld.m[2][0],parentWorld.m[2][1],parentWorld.m[2][2]};
			if (std::abs(x.Dot(y.Cross(z))) < 1e-8f) continue;
			parentWorld.Inverse();
			local = local * parentWorld;
		}
		if (is2D) t2->translate = { local.m[3][0], local.m[3][1] };
		else t->translate = local.GetTranslate();
		if (operation == GroupTransformOperation::Scale) {
			// 向きの異なる物体にもTRSで表せる拡縮を使い、せん断を作らない。
			if (is2D) t2->scale = { item.scale.x * scaleFactor.x, item.scale.y * scaleFactor.y };
			else t->scale = { item.scale.x * scaleFactor.x, item.scale.y * scaleFactor.y, item.scale.z * scaleFactor.z };
		} else if (operation == GroupTransformOperation::Rotate) {
			// 回転行列から元の負スケールを分離し、鏡像の向きを保つ。
			for (int axis = 0; axis < 3; ++axis) {
				const Math::Vector3 row{ local.m[axis][0], local.m[axis][1], local.m[axis][2] };
				const float original = axis == 0 ? item.scale.x : axis == 1 ? item.scale.y : item.scale.z;
				const float length = row.Length() * (original < 0.f ? -1.f : 1.f);
				if (std::abs(length) < 1e-8f) continue;
				for (int column = 0; column < 3; ++column) local.m[axis][column] /= length;
			}
			if (is2D) t2->rotation = std::atan2(local.m[0][1], local.m[0][0]);
			else { t->rotation.FromRotationMatrix(local); t->rotation.Normalize(); }
		}
	}
}

}

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_internal.h"
#include "engine/Assets/AssetManager.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine::Editor {
namespace {
struct PendingFieldEdit {
	ECS::Registry* registry = nullptr;
	BatchSnapshot before;
	BatchSnapshot after;
	const FieldInfo* field = nullptr;
	std::vector<ECS::Entity> selection;
};
PendingFieldEdit sPending;

void FinishFieldEdit() {
	if (!sPending.registry) return;
	RecordBatchEdit(*sPending.registry, sPending.before, sPending.after);
	sPending = {};
}

void DrawBatchField(ECS::Registry& registry, const std::vector<ECS::Entity>& selection, const TypeInfo& type, const FieldInfo& field) {
	const auto before = CaptureBatch(registry, selection, type.typeId, &field);
	if (before.entities.empty() || !before.entities.front().values.contains(field.name)) return;
	const auto original = before.entities.front().values.at(field.name);
	auto value = original;
	bool mixed = false;
	for (const auto& entry : before.entities) mixed |= entry.values.at(field.name) != original;
	ImGui::PushID(&field);
	bool changed = false;
	int editedAxis = -1;
	const float low = field.attributes.hasRange ? field.attributes.minValue : 0.f;
	const float high = field.attributes.hasRange ? field.attributes.maxValue : 0.f;
	const auto mixedFlag = [&](bool state) { ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, state); };
	if (field.type == FieldType::Float || field.type == FieldType::Float2 || field.type == FieldType::Float3 || field.type == FieldType::Float4 || field.type == FieldType::Color) {
		const int count = value.is_array() ? static_cast<int>(value.size()) : 1;
		ImGui::Text("%s%s", field.name.c_str(), mixed ? " (Mixed)" : "");
		for (int axis = 0; axis < count; ++axis) {
			if (axis) ImGui::SameLine();
			ImGui::PushID(axis);
			const auto scalar = value.is_array() ? original[axis] : original;
			bool axisMixed = false;
			for (const auto& entry : before.entities) axisMixed |= (value.is_array() ? entry.values[field.name][axis] : entry.values[field.name]) != scalar;
			mixedFlag(axisMixed);
			float number = scalar.get<float>();
			ImGui::SetNextItemWidth(std::max(35.f, (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * (count - axis - 1)) / (count - axis)));
			if (ImGui::DragFloat("##Value", &number, field.attributes.valueSpeed, low, high)) {
				if (std::isfinite(number)) {
					if (value.is_array()) value[axis] = number; else value = number;
					editedAxis = axis;
					changed = true;
				}
			}
			ImGui::PopItemFlag();
			ImGui::PopID();
		}
	} else {
		if (mixed && (field.type == FieldType::String || field.type == FieldType::WString)) ImGui::TextDisabled("Mixed values");
		mixedFlag(mixed);
		switch (field.type) {
		case FieldType::Bool: {
			bool number = value.get<bool>();
			changed = ImGui::Checkbox(field.name.c_str(), &number); value = number; break;
		}
		case FieldType::Int: {
			int number = value.get<int>();
			changed = ImGui::DragInt(field.name.c_str(), &number, field.attributes.valueSpeed,
				static_cast<int>(low), static_cast<int>(high));
			value = number; break;
		}
		case FieldType::Uint: {
			uint32_t number = value.get<uint32_t>();
			const uint32_t minimum = 0, maximum = UINT32_MAX;
			changed = ImGui::DragScalar(field.name.c_str(), ImGuiDataType_U32, &number, field.attributes.valueSpeed, &minimum, &maximum);
			value = number; break;
		}
		case FieldType::String:
		case FieldType::WString: {
			char buffer[1024];
			const auto text = field.type == FieldType::WString ? ConvertString(value.get<std::wstring>()) : value.get<std::string>();
			strncpy_s(buffer, text.c_str(), _TRUNCATE);
			changed = field.attributes.multiline
				? ImGui::InputTextMultiline(field.name.c_str(), buffer, sizeof(buffer), ImVec2(-1.f, ImGui::GetTextLineHeight() * 4.f))
				: ImGui::InputText(field.name.c_str(), buffer, sizeof(buffer));
			value = field.type == FieldType::WString ? nlohmann::json(ConvertString(std::string(buffer))) : nlohmann::json(buffer);
			if (field.attributes.assetType != EBrowserItemType::Unknown && ImGui::BeginDragDropTarget()) {
				if (const auto* payload = ImGui::AcceptDragDropPayload(GetAssetDragPayloadType(field.attributes.assetType))) {
					if (payload->DataSize > 0 && static_cast<const char*>(payload->Data)[payload->DataSize - 1] == '\0') {
						std::string asset = static_cast<const char*>(payload->Data);
						if (field.attributes.assetSourcePath && field.attributes.assetType != EBrowserItemType::Prefab) asset = AssetManager::GetFilePathFromAddressableName(asset);
						if (!asset.empty()) { value = asset; changed = true; }
					}
				}
				ImGui::EndDragDropTarget();
			}
			break;
		}
		case FieldType::Enum: {
			if (field.enumOps && ImGui::BeginCombo(field.name.c_str(), mixed ? "Mixed" : value.get_ref<const std::string&>().c_str())) {
				for (const auto& name : field.enumOps->names()) if (ImGui::Selectable(name.c_str(), value == name)) { value = name; changed = true; }
				ImGui::EndCombo();
			}
			break;
		}
		case FieldType::Entity: {
			const auto preview = mixed ? std::string("Mixed") : value.is_null() ? std::string("None") : value.get<std::string>();
			if (ImGui::BeginCombo(field.name.c_str(), preview.c_str())) {
				if (ImGui::Selectable("None", value.is_null())) { value = nullptr; changed = true; }
				for (auto target : registry.View<EditTag>()) {
					const auto& name = registry.GetComponent<EditTag>(target)->name;
					if (ImGui::Selectable(name.c_str(), value == name)) { value = name; changed = true; }
				}
				ImGui::EndCombo();
			}
			if (ImGui::BeginDragDropTarget()) {
				const auto* payload = ImGui::AcceptDragDropPayload("ENTITY_MULTI");
				if (!payload) payload = ImGui::AcceptDragDropPayload("ENTITY");
				if (payload && payload->DataSize >= static_cast<int>(sizeof(ECS::Entity))) {
					const auto* tag = registry.GetComponent<EditTag>(*static_cast<const ECS::Entity*>(payload->Data));
					if (tag) { value = tag->name; changed = true; }
				}
				ImGui::EndDragDropTarget();
			}
			break;
		}
		default: ImGui::TextDisabled("%s (edit individually)", field.name.c_str()); break;
		}
		ImGui::PopItemFlag();
	}
	if (changed) {
		if (sPending.registry && (sPending.registry != &registry || sPending.field != &field || sPending.selection != selection)) FinishFieldEdit();
		if (!sPending.registry) sPending = { &registry, before, before, &field, selection };
		auto after = before;
		for (auto& entry : after.entities) {
			// ベクトルは操作した軸だけを揃え、他の軸の個別値を保つ。
			if (value.is_array() && editedAxis >= 0) entry.values[field.name][editedAxis] = value[editedAxis];
			else entry.values[field.name] = value;
		}
		ApplyBatch(registry, after);
		sPending.after = std::move(after);
	}
	ImGui::PopID();
}
}

void ResetPendingMultiEdit() { sPending = {}; }

void DrawMultiInspector(ECS::Registry& registry, const std::vector<ECS::Entity>& selection) {
	if (sPending.registry && sPending.registry != &registry) sPending = {};
	if (sPending.registry && sPending.selection != selection) FinishFieldEdit();
	if (selection.size() > 1) {
		ImGui::Text("%zu objects selected", selection.size());
		ImGui::TextDisabled("Common components / local values");
		for (const auto& type : ComponentRegistry::GetAll()) {
			if (type.typeId == Utilities::TypeID<EditTag>() || !std::all_of(selection.begin(), selection.end(), [&](auto e) { return registry.Has(type.typeId, e); })) continue;
			ImGui::PushID(&type);
			if (ImGui::CollapsingHeader(type.name.c_str())) {
				for (const auto& field : type.fields) {
					if (!field.attributes.editable || (field.type == FieldType::Entity && field.name == "parent")) continue;
					// 配列や入れ子の構造体を丸ごと上書きして、個別設定を失わないようにする。
					if (field.type == FieldType::Array || field.type == FieldType::Struct || field.type == FieldType::Unknown) {
						ImGui::TextDisabled("%s (edit individually)", field.name.c_str());
						continue;
					}
					DrawBatchField(registry, selection, type, field);
				}
			}
			ImGui::PopID();
		}
	}
	if (sPending.registry && !ImGui::IsAnyItemActive()) FinishFieldEdit();
}
}
#else
namespace NoEngine::Editor {
void DrawMultiInspector(ECS::Registry&, const std::vector<ECS::Entity>&) {}
void ResetPendingMultiEdit() {}
}
#endif
