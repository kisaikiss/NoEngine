#pragma once
#include "engine/NoEngine.h"
#include "../../Component/TutorialSpriteComponent.h"
#include"engine/Math/Types/Vector2.h"
class TutorialControlSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float timer_ = 0.0f;

	const float kEndPosX_ = 0.0f;
	const float kMiddlePosX_ = 640.0f;
	const float kStartPosX_ = 1280.0f;

	const float kFirstStartTime = 1.0f;
	const float kFirstShowTime = 1.0f;
	const float kFirstEndTime = 1.0f;

	void MoveSprite(No::Transform2DComponent* transform, TutorialSpriteComponent* tutorial,const NoEngine::Vector2& offset,float deltaTime);
};
