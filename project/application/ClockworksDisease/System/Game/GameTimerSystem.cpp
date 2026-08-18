#include "stdafx.h"
#include "GameTimerSystem.h"
#include "../../Component/Game/GameProgressComponent.h"

void GameTimerSystem::Update(No::Registry& registry, float deltaTime) {
	for (auto e : registry.View<GameProgressComponent>()) {
		registry.GetComponent<GameProgressComponent>(e)->elapsedTime += deltaTime;
	}
}