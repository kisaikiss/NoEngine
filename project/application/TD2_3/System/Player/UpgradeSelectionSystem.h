#pragma once
#include "engine/NoEngine.h"

struct UpgradeChooseComponent;
class UpgradeSelectionSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	void HideChooseUI(No::Registry& registry, UpgradeChooseComponent* choose, No::Entity chooseEntity);
	void ApplyUpgradeChoice(No::Registry& registry, No::Entity statusEntity, const std::string& choiceName, UpgradeChooseComponent* choose);
};