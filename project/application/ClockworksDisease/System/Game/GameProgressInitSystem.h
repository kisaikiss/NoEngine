#pragma once
#include "engine/NoEngine.h"

// シーンに配置された全CollectibleItemTagの数を数え、GameProgressComponent::totalItemCountへ設定する。
// EditSystemによるシーンJSONの読み込みはSetup()ではなく初回Update()で行われるため、
// Setup()時点ではまだアイテムが存在しない可能性がある。そのため単発カウントをSystem側で毎フレーム試行する。
class GameProgressInitSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};