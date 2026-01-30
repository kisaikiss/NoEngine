#include "HpSpriteControlSystem.h"
#include"../SpriteConfigManager/SpriteConfigManager.h"

void HpSpriteControlSystem::Update(No::Registry& registry, float deltaTime)
{
    auto spriteView = registry.View<No::Transform2DComponent, No::SpriteComponent>();

    (void)deltaTime;

//    for (auto entity : spriteView) {
//
//        auto* a = registry.GetComponent<No::Transform2DComponent>(entity);
//        auto* sp = registry.GetComponent<No::SpriteComponent>(entity);
//
//        // ImGui 編集
//#ifdef USE_IMGUI
//        std::string imGuiName = sp->name.empty()
//            ? "Sprite" + std::to_string(entity)
//            : sp->name;
//
//        ImGui::Begin(imGuiName.c_str());
//        ImGui::DragFloat2("translate", &a->translate.x, 1.f);
//        ImGui::DragFloat2("scale", &a->scale.x, 0.1f);
//        ImGui::DragFloat("rotate", &a->rotation, 0.04f);
//
//        ImGui::DragFloat2("pivot", &sp->pivot.x, 0.05f, 0.f, 1.f);
//        ImGui::Checkbox("flipX", &sp->flipX);
//        ImGui::Checkbox("flipY", &sp->flipY);
//        ImGui::DragFloat4("uv", &sp->uv.x, 0.05f);
//        ImGui::End();
//#endif
//    }
}
