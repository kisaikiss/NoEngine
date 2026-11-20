#include "DialogueComponent.h"
#include "engine/Editor/Reflection/ComponentHooks.h"
#ifdef USE_IMGUI
#include "engine/Editor/Components/ComponentUI.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"
#include "engine/Functions/Command/EditCommand/FunctionCommand.h"
#include "externals/imgui/imgui.h"
#endif

#include "engine/Functions/ECS/Gameplay/GameplayCatalog.h"

void RegisterDialogueGameplay() {
	using namespace NoEngine::Gameplay;
	auto& catalog = GameplayCatalog::Get();
	catalog.RegisterEvent({ DialogueGameplayId::Started, "Dialogue Started", "Dialogue" });
	catalog.RegisterEvent({ DialogueGameplayId::Finished, "Dialogue Finished", "Dialogue" });
	catalog.RegisterEvent({ DialogueGameplayId::Cancelled, "Dialogue Cancelled", "Dialogue" });
	catalog.RegisterAction({ DialogueGameplayId::Play, "Play Dialogue", "Dialogue",
		[](No::Registry& registry, const NoEngine::Event::GameplayEvent&, No::Entity target, const nlohmann::json&) {
			auto* dialogue = registry.GetComponent<DialogueComponent>(target);
			if (!dialogue || !dialogue->enabled || dialogue->lines.empty()) {
				LogWarning("Play Dialogue: target has no enabled dialogue with lines");
				return;
			}
			registry.EmitEvent(DialogueStartEvent{ target });
		}, true, {{ "targetEntity", "Target Entity", ActionPropertyType::EntityReference, true }} });
	catalog.RegisterAction({ DialogueGameplayId::Stop, "Stop Dialogue", "Dialogue",
		[](No::Registry& registry, const NoEngine::Event::GameplayEvent&, No::Entity target, const nlohmann::json&) {
			registry.EmitEvent(DialogueStopEvent{ target });
		}, true, {{ "targetEntity", "Target Entity", ActionPropertyType::EntityReference, true }} });
	catalog.RegisterAction({ DialogueGameplayId::SetEnabled, "Set Dialogue Enabled", "Dialogue",
		[](No::Registry& registry, const NoEngine::Event::GameplayEvent&, No::Entity target, const nlohmann::json& properties) {
			if (auto* dialogue = registry.GetComponent<DialogueComponent>(target)) {
				// 無効化は次回以降の開始だけを抑止する。進行中の会話はStopで中断する。
				dialogue->enabled = properties.value("enabled", true);
			} else {
				LogWarning("Set Dialogue Enabled: target has no DialogueComponent");
			}
		}, true, {
			{ "targetEntity", "Target Entity", ActionPropertyType::EntityReference, true },
			{ "enabled", "Enabled", ActionPropertyType::Boolean, true }
		} });
}

const DialogueProfile* DialogueComponent::FindProfile(const std::string& id) const {
	if (id.empty()) return nullptr;
	const DialogueProfile* result = nullptr;
	for (const auto& profile : profiles) {
		if (profile.id != id) continue;
		if (result) return nullptr; // 重複IDも未解決として扱う
		result = &profile;
	}
	return result;
}

DialogueProfile DialogueComponent::ResolveProfile(const DialogueLine& line) const {
	if (!line.profileId.empty()) {
		if (const auto* profile = FindProfile(line.profileId)) return *profile;
		LogWarning("Dialogue: missing or duplicated profile: " + line.profileId);
	}
	if (!defaultProfileId.empty()) {
		if (const auto* profile = FindProfile(defaultProfileId)) return *profile;
		LogWarning("Dialogue: missing or duplicated default profile: " + defaultProfileId);
	}
	// 旧データの会話全体の設定を既定値として維持する。
	DialogueProfile profile;
	profile.portraitTexture = portraitTexture;
	profile.portraitFrameCount = portraitFrameCount;
	profile.fontName = fontName;
	profile.voiceFiles = voiceFiles;
	profile.voiceVolume = voiceVolume;
	return profile;
}

REFLECT_STRUCT_BEGIN(DialogueProfile)
REFLECT_FIELD(id),
REFLECT_ASSET_PATH_FIELD(portraitTexture, Texture),
REFLECT_FIELD(portraitFrameCount),
REFLECT_FIELD(showPortrait),
REFLECT_ASSET_PATH_ARRAY_FIELD(voiceFiles, Audio),
REFLECT_FIELD(voiceVolume),
REFLECT_ASSET_FIELD(fontName, Font),
REFLECT_FIELD(textScale)
REFLECT_STRUCT_END(DialogueProfile)

REFLECT_STRUCT_BEGIN(DialogueLine)
REFLECT_MULTILINE_FIELD(text),
REFLECT_FIELD(profileId)
REFLECT_STRUCT_END(DialogueLine)

REFLECT_STRUCT_BEGIN(DialogueComponent, "Application")
REFLECT_STRUCT_ARRAY_FIELD(lines),
REFLECT_STRUCT_ARRAY_FIELD(profiles),
REFLECT_FIELD(defaultProfileId),
REFLECT_ASSET_FIELD(fontName, Font),
REFLECT_ASSET_PATH_FIELD(portraitTexture, Texture),
REFLECT_FIELD(portraitFrameCount),
REFLECT_ASSET_PATH_ARRAY_FIELD(voiceFiles, Audio),
REFLECT_FIELD(voiceVolume),
REFLECT_FIELD(interactDistance),
REFLECT_FIELD(interactable),
REFLECT_FIELD(enabled)
REFLECT_STRUCT_END(DialogueComponent)

REFLECT_STRUCT_BEGIN(DialogueInteractPromptSettings, "Application")
REFLECT_FIELD(offset),
REFLECT_FIELD(size),
REFLECT_FIELD(interval)
REFLECT_STRUCT_END(DialogueInteractPromptSettings)

REFLECT_STRUCT_BEGIN(DialogueAutoTriggerComponent, "Application")
REFLECT_FIELD(radius),
REFLECT_FIELD(once)
REFLECT_STRUCT_END(DialogueAutoTriggerComponent)

namespace {
#ifdef USE_IMGUI
using namespace NoEngine;
using namespace NoEngine::ECS;
// 配列の再確保後もUndoできるよう、会話全体を値で記録する。
void RecordDialogueEdit(Registry& registry, Entity entity,
	const DialogueComponent& before, const DialogueComponent& after) {
	auto apply = [&registry, entity](const DialogueComponent& value) {
		if (auto* component = registry.GetComponent<DialogueComponent>(entity)) *component = value;
	};
	Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::FunctionCommand>(
		[apply, after] { apply(after); }, [apply, before] { apply(before); }, "Edit dialogue"));
}

bool DrawDialogueProfileCombo(const char* label, std::string& id, const DialogueComponent& component,
	const char* emptyLabel) {
	bool changed = false;
	const char* preview = id.empty() ? emptyLabel : id.c_str();
	if (ImGui::BeginCombo(label, preview)) {
		if (ImGui::Selectable(emptyLabel, id.empty())) { id.clear(); changed = true; }
		for (const auto& profile : component.profiles) {
			if (profile.id.empty() || !component.FindProfile(profile.id)) continue;
			if (ImGui::Selectable(profile.id.c_str(), id == profile.id)) { id = profile.id; changed = true; }
		}
		ImGui::EndCombo();
	}
	if (!id.empty() && !component.FindProfile(id))
		ImGui::TextDisabled("Missing or duplicated profile; using default settings.");
	return changed;
}

void DrawDialogueUI(Registry& registry, Entity entity, const TypeInfo& type, DialogueComponent& component) {
	static Registry* editingRegistry = nullptr;
	static Entity editingEntity = INVALID_ENTITY;
	static std::optional<DialogueComponent> editBefore;
	if (editingRegistry != &registry || editingEntity != entity) editBefore.reset();
	editingRegistry = &registry;
	editingEntity = entity;
	if (!editBefore) editBefore = component;
	bool changed = false;
	const bool fieldsChanged = CaptureFieldEditCommands([&] {
		changed = DrawDialogueProfileCombo("Default Profile", component.defaultProfileId, component, "(Legacy settings)");
		ImGui::TextDisabled("Empty line profile uses Default Profile. Legacy settings preserve existing dialogue.");
		if (ImGui::TreeNode("Profiles")) {
			int remove = -1;
			auto* profileType = ComponentRegistry::FindByTypeID(Utilities::TypeID<DialogueProfile>());
			for (size_t i = 0; i < component.profiles.size(); ++i) {
				ImGui::PushID(static_cast<int>(i));
				auto& profile = component.profiles[i];
				const std::string label = "[" + std::to_string(i) + "] " + profile.id;
				if (ImGui::TreeNode(label.c_str())) {
					char idBuffer[256];
					strncpy_s(idBuffer, profile.id.c_str(), _TRUNCATE);
					if (ImGui::InputText("Profile ID", idBuffer, sizeof(idBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
						const std::string newId = idBuffer;
						const bool duplicate = std::any_of(component.profiles.begin(), component.profiles.end(),
							[&](const auto& other) { return &other != &profile && other.id == newId; });
						if (!newId.empty() && !duplicate && newId != profile.id) {
							// 有効なIDの変更では会話内の参照も一緒に更新する。
							if (component.FindProfile(profile.id) == &profile) {
								for (auto& line : component.lines) if (line.profileId == profile.id) line.profileId = newId;
								if (component.defaultProfileId == profile.id) component.defaultProfileId = newId;
							}
							profile.id = newId;
							changed = true;
						}
					}
					ImGui::TextDisabled("Unique, non-empty ID; Enter to apply. Volume 0 mutes voice.");
					if (!component.FindProfile(profile.id)) ImGui::TextDisabled("Profile ID is empty or duplicated.");
					if (profileType) for (const auto& field : profileType->fields)
						if (field.name != "id") DrawFieldUI(registry, entity, field, &profile);
					if (ImGui::Button("Remove Profile")) remove = static_cast<int>(i);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			if (remove >= 0) {
				component.profiles.erase(component.profiles.begin() + remove);
				changed = true;
			}
			if (ImGui::Button("Add Profile")) {
				DialogueLine legacy;
				// 新規プロフィールは会話全体の従来設定を複製する。
				auto defaults = component;
				defaults.defaultProfileId.clear();
				auto profile = defaults.ResolveProfile(legacy);
				int suffix = 1;
				do { profile.id = "profile_" + std::to_string(suffix++); }
				while (std::any_of(component.profiles.begin(), component.profiles.end(),
					[&](const auto& other) { return other.id == profile.id; }));
				component.profiles.push_back(std::move(profile));
				changed = true;
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Lines")) {
			int remove = -1;
			auto* lineType = ComponentRegistry::FindByTypeID(Utilities::TypeID<DialogueLine>());
			for (size_t i = 0; i < component.lines.size(); ++i) {
				ImGui::PushID(static_cast<int>(i));
				const std::string label = "Line " + std::to_string(i + 1);
				if (ImGui::TreeNode(label.c_str())) {
					auto& line = component.lines[i];
					if (lineType) for (const auto& field : lineType->fields)
						if (field.name == "text") DrawFieldUI(registry, entity, field, &line);
					changed |= DrawDialogueProfileCombo("Profile", line.profileId, component, "(Default Profile)");
					if (ImGui::Button("Move Up") && i > 0) {
						std::swap(component.lines[i], component.lines[i - 1]); changed = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("Move Down") && i + 1 < component.lines.size()) {
						std::swap(component.lines[i], component.lines[i + 1]); changed = true;
					}
					if (ImGui::Button("Remove Line")) remove = static_cast<int>(i);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			if (remove >= 0) { component.lines.erase(component.lines.begin() + remove); changed = true; }
			if (ImGui::Button("Add Line")) { component.lines.emplace_back(); changed = true; }
			ImGui::TreePop();
		}
		for (const auto& field : type.fields) {
			if (field.name != "lines" && field.name != "profiles" && field.name != "defaultProfileId")
				DrawFieldUI(registry, entity, field, &component);
		}
	});
	if (changed || fieldsChanged) {
		RecordDialogueEdit(registry, entity, *editBefore, component);
		editBefore.reset();
	} else if (!ImGui::IsAnyItemActive()) editBefore.reset();
}

#endif

// 型固有の処理を起動前に登録し、エンジンから会話の型を参照させない。
const bool dialogueHooksRegistered = [] {
	NoEngine::ComponentHooks hooks;
#ifdef USE_IMGUI
	hooks.drawInspector = [](No::Registry& registry, No::Entity entity, const NoEngine::TypeInfo& type, void* component) {
		DrawDialogueUI(registry, entity, type, *static_cast<DialogueComponent*>(component));
	};
#endif
	hooks.beforeDeserialize = [](void* component, nlohmann::json& values) {
		// シーンとPrefabで同じ移行を通し、新旧のセリフ形式の混在も許可する。
		if (values.contains("lines") && values["lines"].is_array()) {
			for (auto& line : values["lines"]) {
				if (line.is_string()) line = nlohmann::json{{"text", line.get<std::string>()}, {"profileId", ""}};
			}
		}
		// 旧データの再読込時に、直前のプロフィールを残さない。
		*static_cast<DialogueComponent*>(component) = DialogueComponent{};
	};
	NoEngine::ComponentHookRegistry::Register(NoEngine::Utilities::TypeID<DialogueComponent>(), std::move(hooks));
	return true;
}();
}
