#include "TutorialControlSystem.h"
#include"engine/Math/Easing.h"
#include "application/TD2_3/tag.h"
void TutorialControlSystem::Update(No::Registry& registry, float deltaTime)
{
    timer_ += deltaTime;
    (void)registry;

    auto view = registry.View<No::Transform2DComponent, No::SpriteComponent, TutorialSpriteTag>();

    for (auto entity : view) {

        


    }

}
