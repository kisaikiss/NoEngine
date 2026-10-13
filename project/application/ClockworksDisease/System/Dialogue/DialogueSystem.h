#pragma once
#include "engine/NoEngine.h"

class DialogueSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	bool initialized_ = false;
	bool active_ = false;
	float boxWidth_ = 0.0f;
	size_t lineIndex_ = 0;

	No::Entity boxEntity_ = No::INVALID_ENTITY;
	No::Entity textEntity_ = No::INVALID_ENTITY;
	No::Entity speaker_ = No::INVALID_ENTITY;
	bool Begin(No::Registry& registry, No::Entity speaker);
	void Advance(No::Registry& registry);
	void ShowLine(No::Registry& registry);
	void End(No::Registry& registry);
	void CreateTextBox(No::Registry& registry);
};