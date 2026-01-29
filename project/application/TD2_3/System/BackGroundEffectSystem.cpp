#include "BackGroundEffectSystem.h"
#include "../Component/BackGroundComponent.h"
#include "externals/imgui/imgui.h"
#include "engine/Math/Easing.h"

using namespace NoEngine;

void BackGroundEffectSystem::Update(No::Registry& registry, float deltaTime)
{
	auto view = registry.View<BackGroundComponent>();

	for (auto entity : view)
	{
		auto* backGround = registry.GetComponent<BackGroundComponent>(entity);
		backGround->time += deltaTime;

#ifdef USE_IMGUI
		ImGui::Begin("BackGround");
		ImGui::DragFloat("timeScale", &backGround->timeScale, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat("powerFactor", &backGround->powerFactor, 0.01f, 0.0f, 1.0f);
		ImGui::End();
#endif // USE_IMGUI

	}
}
