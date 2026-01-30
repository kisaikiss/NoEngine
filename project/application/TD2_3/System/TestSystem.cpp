#include "TestSystem.h"

void TestSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<No::TransformComponent, No::MaterialComponent>();
	
	for (auto entity : view) {
		auto* a = registry.GetComponent<No::TransformComponent>(entity);
		if (No::Pad::IsPress(No::GamepadButton::Up)) a->translate.y += 1.f * deltaTime;
		if (No::Pad::IsPress(No::GamepadButton::Down)) a->translate.y -= 1.f * deltaTime;
		if (No::Pad::IsPress(No::GamepadButton::Right)) a->translate.x += 1.f * deltaTime;
		if (No::Pad::IsPress(No::GamepadButton::Left)) a->translate.x -= 1.f * deltaTime;
		auto p = No::Pad::GetStick();
		a->translate.x += p.leftStickX * deltaTime;
		a->translate.y += p.leftStickY * deltaTime;
#ifdef USE_IMGUI
		//angle_ += 1.f * deltaTime;
		//a->rotation.FromAxisAngle(NoEngine::Vector3(0.f, 1.f, 0.f), angle_);

		std::string imGuiName = "model" + std::to_string(entity);
		ImGui::Begin(imGuiName.c_str());
		ImGui::DragFloat3("translate", &a->translate.x, 0.05f);
		ImGui::DragFloat3("scale", &a->scale.x, 0.05f);
		ImGui::DragFloat4("rotate", &a->rotation.x, 0.04f);
		ImGui::End();	
#endif // USE_IMGUI
	}



#ifdef USE_IMGUI
	auto spriteView = registry.View < No::Transform2DComponent, No::SpriteComponent>();

	for (auto entity : spriteView) {
		auto* a = registry.GetComponent<No::Transform2DComponent>(entity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(entity);
		std::string imGuiName = "sprite" + std::to_string(entity);
		ImGui::Begin(imGuiName.c_str());
		ImGui::DragFloat2("translate", &a->translate.x, 1.f);
		ImGui::DragFloat2("scale", &a->scale.x, 0.1f);
		ImGui::DragFloat("rotate", &a->rotation, 0.04f);
		ImGui::DragFloat2("pivot", &sp->pivot.x, 0.05f, 0.f, 1.f);
		ImGui::Checkbox("flipX", &sp->flipX);
		ImGui::Checkbox("flipY", &sp->flipY);
		ImGui::DragFloat4("uv", &sp->uv.x, 0.05f);
		ImGui::End();
	}

	auto lightView = registry.View<No::DirectionalLightComponent>();

	for (auto entity : lightView) {
		auto* a = registry.GetComponent<No::DirectionalLightComponent>(entity);
		std::string imGuiName = "light" + std::to_string(entity);
		ImGui::Begin(imGuiName.c_str());
		ImGui::DragFloat4("color", &a->color.r, 0.1f);
		ImGui::DragFloat3("direction", &a->direction.x, 0.05f);
		ImGui::DragFloat("intensity", &a->intensity, 0.03f);
		a->direction = a->direction.Normalize();
		ImGui::End();
	}
#endif // USE_IMGUI
	

}
