#include "DialogueComponent.h"
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

REFLECT_STRUCT_BEGIN(DialogueComponent, "Application")
REFLECT_ARRAY_FIELD(lines),
REFLECT_ASSET_FIELD(fontName, Font),
REFLECT_ASSET_PATH_FIELD(portraitTexture, Texture),
REFLECT_FIELD(portraitFrameCount),
REFLECT_ASSET_PATH_ARRAY_FIELD(voiceFiles, Audio),
REFLECT_FIELD(voiceVolume),
REFLECT_FIELD(interactDistance),
REFLECT_FIELD(interactable),
REFLECT_FIELD(enabled)
REFLECT_STRUCT_END(DialogueComponent)

REFLECT_STRUCT_BEGIN(DialogueAutoTriggerComponent, "Application")
REFLECT_FIELD(radius),
REFLECT_FIELD(once)
REFLECT_STRUCT_END(DialogueAutoTriggerComponent)
