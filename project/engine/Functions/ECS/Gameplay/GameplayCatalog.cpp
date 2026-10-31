#include "stdafx.h"
#include "GameplayCatalog.h"
#include "engine/Functions/ECS/Component/Common/MoverComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent.h"

namespace NoEngine::Gameplay {

GameplayCatalog& GameplayCatalog::Get() {
	// 全システムで同じ登録一覧を見るため、プロセス内で一つだけ生成する。
	static GameplayCatalog catalog;
	return catalog;
}

GameplayCatalog::GameplayCatalog() {
	// エンジン標準のイベント／Action。アプリ固有の登録は起動時にRegister系関数で追加する。
	RegisterEvent({ Event::GameplayId::TriggerEnter, "Trigger Enter", "Trigger" });
	RegisterAction({ Event::GameplayId::MoverActivate, "Activate Mover", "Movement",
		[](ECS::Registry& registry, const Event::GameplayEvent&, ECS::Entity target, const nlohmann::json&) {
			// MoverSystemが起動イベントを処理した後、完了判定が新しい再生を待てるよう状態を戻す。
			if (auto* mover = registry.GetComponent<NoEngine::Component::MoverComponent>(target)) mover->active = false;
			registry.EmitEvent(Event::ActivateMoverEvent{ target });
		}, true, {{ "targetEntity", "Target Entity", ActionPropertyType::EntityReference, true }},
		[](ECS::Registry& registry, ECS::Entity target, const nlohmann::json&) {
			// 対象の移動ルートが非ループ再生を終えるまで、後続Actionを待たせる。
			auto* mover = registry.GetComponent<NoEngine::Component::MoverComponent>(target);
			auto* routine = registry.GetComponent<NoEngine::Component::TransformRoutineComponent>(target);
			return !mover || !routine || (mover->active && (!routine->playing || routine->keyframes.size() < 2));
		} });
}

bool GameplayCatalog::RegisterEvent(EventDescriptor descriptor) {
	// 空ID・表示名や重複IDは、保存データから一意に引けないので登録しない。
	if (descriptor.id.empty() || descriptor.displayName.empty() || eventIndices_.contains(descriptor.id)) return false;
	const size_t index = events_.size();
	eventIndices_.emplace(descriptor.id, index);
	events_.push_back(std::move(descriptor));
	return true;
}

bool GameplayCatalog::RegisterAction(ActionDescriptor descriptor) {
	// Actionは実行関数と一意なIDが必須。プロパティ定義も重複や空欄を受け付けない。
	if (descriptor.id.empty() || descriptor.displayName.empty() || !descriptor.execute || actionIndices_.contains(descriptor.id)) return false;
	std::unordered_set<std::string> propertyIds;
	bool hasRequiredEntityReference = false;
	for (const auto& property : descriptor.properties) {
		if (property.id.empty() || property.displayName.empty() || !propertyIds.insert(property.id).second) return false;
		if (property.required && property.type == ActionPropertyType::EntityReference) hasRequiredEntityReference = true;
	}
	if (descriptor.requiresTarget && !hasRequiredEntityReference) return false;
	const size_t index = actions_.size();
	actionIndices_.emplace(descriptor.id, index);
	actions_.push_back(std::move(descriptor));
	return true;
}

bool GameplayCatalog::HasEvent(const std::string& id) const {
	return eventIndices_.contains(id);
}

const EventDescriptor* GameplayCatalog::FindEvent(const std::string& id) const {
	auto it = eventIndices_.find(id);
	return it == eventIndices_.end() ? nullptr : &events_[it->second];
}

const ActionDescriptor* GameplayCatalog::FindAction(const std::string& id) const {
	auto it = actionIndices_.find(id);
	return it == actionIndices_.end() ? nullptr : &actions_[it->second];
}

bool EmitGameplayEvent(ECS::Registry& registry, ECS::Entity source,
	const std::string& eventId, ECS::Entity instigator) {
	// 未登録IDをイベントキューへ流すとノード側で解釈できないため、発行前に検査する。
	if (!GameplayCatalog::Get().HasEvent(eventId)) {
		LogWarning("Gameplay: event id is not registered: " + eventId);
		return false;
	}
	registry.EmitEvent(Event::GameplayEvent{ source, eventId, instigator });
	return true;
}

}
