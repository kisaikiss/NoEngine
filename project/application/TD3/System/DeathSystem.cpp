#include "DeathSystem.h"
#include <vector>

void DeathSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);// 未使用パラメータ警告回避
	
	auto view = registry.View<DeathFlag>();
	if (view.Empty()) return;//ないとエラー

	// 一括削除のため、削除予定のエンティティを一時的に保存するベクター
	std::vector<No::Entity> toDestroy;

	for (auto entity : view) {
		auto* flag = registry.GetComponent<DeathFlag>(entity);
		if (flag && flag->isDead) {
			toDestroy.push_back(entity);
		}
	}

	for (auto entity : toDestroy) {
		registry.DestroyEntity(entity);
	}
}
