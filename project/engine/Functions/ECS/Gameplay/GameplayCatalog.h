#pragma once
#include "engine/Functions/ECS/Event/GameplayEvents.h"
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine::Gameplay {

struct EventDescriptor {
	std::string id;
	std::string displayName;
	std::string category;
};

struct ActionDescriptor {
	std::string id;
	std::string displayName;
	std::string category;
	std::function<void(ECS::Registry&, const Event::GameplayEvent&, ECS::Entity)> execute;
	bool requiresTarget = true;
};

// イベント／Actionの安定IDと表示情報を登録するカタログ。
// エンジン共通の登録に加えて、アプリケーションのStartupから独自Actionを登録できる。
class GameplayCatalog {
public:
	static GameplayCatalog& Get();

	bool RegisterEvent(EventDescriptor descriptor);
	bool RegisterAction(ActionDescriptor descriptor);
	bool HasEvent(const std::string& id) const;
	const ActionDescriptor* FindAction(const std::string& id) const;
	const std::vector<EventDescriptor>& GetEvents() const { return events_; }
	const std::vector<ActionDescriptor>& GetActions() const { return actions_; }

private:
	GameplayCatalog();
	std::vector<EventDescriptor> events_;
	std::vector<ActionDescriptor> actions_;
	std::unordered_map<std::string, size_t> eventIndices_;
	std::unordered_map<std::string, size_t> actionIndices_;
};

// 未登録イベントは発行せずfalseを返す。アプリケーション側もこの関数を使う。
bool EmitGameplayEvent(ECS::Registry& registry, ECS::Entity source,
	const std::string& eventId, ECS::Entity instigator = ECS::INVALID_ENTITY);

}
