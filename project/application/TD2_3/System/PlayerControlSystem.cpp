#include "PlayerControlSystem.h"
#include "../Component/ColliderComponent.h"
#include "../tag.h"

void PlayerControlSystem::Update(No::Registry& registry, float deltaTime)
{
	auto view = registry.View<
		No::TransformComponent,
		No::MaterialComponent,
		SphereColliderComponent,
		PlayerTag>();

	for (auto entity : view)
	{
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* material = registry.GetComponent<No::MaterialComponent>(entity);
		auto* collider = registry.GetComponent<SphereColliderComponent>(entity);

		if (No::Keyboard::IsPress('A'))transform->translate.x -= 8.0f * deltaTime;
		if (No::Keyboard::IsPress('D'))transform->translate.x += 8.0f * deltaTime;
		if (No::Keyboard::IsPress('W'))transform->translate.y += 8.0f * deltaTime;
		if (No::Keyboard::IsPress('S'))transform->translate.y -= 8.0f * deltaTime;
		if (No::Keyboard::IsPress('Q'))transform->translate.z -= 8.0f * deltaTime;
		if (No::Keyboard::IsPress('E'))transform->translate.z += 8.0f * deltaTime;

		if (collider->isCollied)
		{
			material->color = NoEngine::Color(1.0f,0.0f,0.0f,1.0f);
		}
		else
		{
            material->color = NoEngine::Color(1.0f,1.0f,1.0f,1.0f);
		}
	}
}
