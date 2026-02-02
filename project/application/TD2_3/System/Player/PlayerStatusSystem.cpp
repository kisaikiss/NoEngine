#include "PlayerStatusSystem.h"
#include "../../Component/PlayerstatusComponent.h"
#include "../../tag.h"
#include "engine/Functions/ECS/Component/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"

#include "externals/imgui/imgui.h"

void PlayerStatusSystem::Update(No::Registry& registry, float deltaTime)
{
	(void)deltaTime;
	auto playerStatusView = registry.View<PlayerStatusComponent>();

	for (auto entity : playerStatusView)
	{
		auto* status = registry.GetComponent<PlayerStatusComponent>(entity);
		
		// 経験値が閾値を超えたらレベルアップを開始（UI 表示のため pendingUpgrade を使う）
		const int requiredExp = 3;
		if (!status->pendingUpgrade && status->exp >= requiredExp)
		{
			status->exp = 0;
			status->level++;
			// UI 表示開始フラグ
			status->pendingUpgrade = true;
		}

		// レベルアップの選択 UI を表示中なら ImGui で三択を表示
#ifdef USE_IMGUI
		ImGui::Begin("Debug Player Status");
		ImGui::Text("Score: %d", status->score);
		ImGui::Text("Level: %d", status->level);
		ImGui::Text("EXP: %d ", status->exp);
		ImGui::Text("HP: %d / %d", status->hp, status->hpMax);
		ImGui::End();

		ImGui::Begin("UpgradeSprite");

		auto spriteView = registry.View<No::SpriteComponent, No::Transform2DComponent,StatusSpriteTag>();

		int index = 0;
		for (auto spriteEntity : spriteView)
		{
			auto* sprite = registry.GetComponent<No::SpriteComponent>(spriteEntity);
			auto* transform2D = registry.GetComponent<No::Transform2DComponent>(spriteEntity);
			if (ImGui::CollapsingHeader(sprite->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushID(index);
				if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::DragFloat2("Position##A", &transform2D->translate.x);
					ImGui::DragFloat("Rotation##A", &transform2D->rotation);
					ImGui::DragFloat2("Scale##A", &transform2D->scale.x);
					ImGui::TreePop();
				}
				if (ImGui::TreeNodeEx("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::DragFloat2("Pivot##A", &sprite->pivot.x);
					ImGui::Checkbox("FlipX##A", &sprite->flipX);
					ImGui::Checkbox("FlipY##A", &sprite->flipY);
					ImGui::DragFloat4("UV##A", &sprite->uv.x);
					ImGui::DragInt("Layer##A", (int*)&sprite->layer);
					ImGui::DragInt("OrderInLayer##A", (int*)&sprite->orderInLayer);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			index++;
		}
		ImGui::End();
#endif // USE_IMGUI
	}
}