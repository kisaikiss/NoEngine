#include "stdafx.h"
#include "GameplayCatalog.h"

namespace NoEngine::Gameplay {

GameplayCatalog& GameplayCatalog::Get() {
	static GameplayCatalog catalog;
	return catalog;
}

GameplayCatalog::GameplayCatalog() {
	RegisterEvent({ Event::GameplayId::TriggerEnter, "Trigger Enter", "Trigger" });
	RegisterAction({ Event::GameplayId::MoverActivate, "Activate Mover", "Movement",
		[](ECS::Registry& registry, const Event::GameplayEvent&, ECS::Entity target, const nlohmann::json&) {
			registry.EmitEvent(Event::ActivateMoverEvent{ target });
		}, true, {{ "targetEntity", "Target Entity", ActionPropertyType::EntityReference, true }} });
}

bool GameplayCatalog::RegisterEvent(EventDescriptor descriptor) {
	if (descriptor.id.empty() || descriptor.displayName.empty() || eventIndices_.contains(descriptor.id)) return false;
	const size_t index = events_.size();
	eventIndices_.emplace(descriptor.id, index);
	events_.push_back(std::move(descriptor));
	return true;
}

bool GameplayCatalog::RegisterAction(ActionDescriptor descriptor) {
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
	if (!GameplayCatalog::Get().HasEvent(eventId)) {
		LogWarning("Gameplay: event id is not registered: " + eventId);
		return false;
	}
	registry.EmitEvent(Event::GameplayEvent{ source, eventId, instigator });
	return true;
}

}
