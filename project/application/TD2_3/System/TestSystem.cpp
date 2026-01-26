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
}
