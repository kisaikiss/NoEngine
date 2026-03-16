#include "TitleSystem.h"
#include "engine/Functions/ECS/Component/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Functions/Input/Input.h"
#include "engine/Math/Easing.h"
#include "externals/imgui/imgui.h"
#include "../Component/PhaseComponent.h"
#include "../Component/PlayerstatusComponent.h"

void TitleSystem::Update(No::Registry& registry, float deltaTime)
{
	time_ += deltaTime;
	if ((No::Keyboard::IsTrigger(VK_RETURN) ||
		No::Pad::IsTrigger(No::GamepadButton::A))/* && !isChangeScene_*/)
	{
		//一旦ここでSE再生 ヨシダ
		No::SoundEffectPlay("select", 0.5f);
		registry.EmitEvent(No::SceneChangeEvent("GameScene"));
		isChangeScene_ = true;
	}
	for (auto entity : registry.View<No::SpriteComponent, No::Transform2DComponent>())
	{
		auto* sprite = registry.GetComponent<No::SpriteComponent>(entity);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(entity);

		if (sprite->name == "Title")
		{
			transform->scale.x += 0.1f * std::sin(time_ * 2.0f);
			transform->scale.y += 0.1f * std::sin(time_ * 2.25f);
			transform->rotation = 0.05f * std::sin(time_ * 1.75f);
		}
		else if (sprite->name == "StartButton")
		{
			transform->translate.y += 0.03f * std::sin(time_ * 2.0f);
			transform->rotation = 0.05f * std::sin(time_ * 2.0f);
		}
		else if (sprite->name == "AButton")
		{
			transform->scale.x += 0.1f * std::sin(time_ * 3.0f);
			transform->scale.y += 0.1f * std::sin(time_ * 3.0f);
		}
#ifdef USE_IMGUI
		ImGui::Begin("Title System");
		if (ImGui::CollapsingHeader(sprite->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(static_cast<int>(entity));
			if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat2("Position", &transform->translate.x);
				ImGui::DragFloat("Rotation", &transform->rotation);
				ImGui::DragFloat2("Scale", &transform->scale.x);
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::ColorEdit4("Color", &sprite->color.r);
				ImGui::DragFloat("Fill", &sprite->fill, 0.01f, 0.0f, 1.0f);
				ImGui::Checkbox("FlipX", &sprite->flipX);
				ImGui::Checkbox("FlipY", &sprite->flipY);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::End();
#endif // USE_IMGUI

	}
}
