#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// DamageRequest を集約して HP へ反映するSystem。
/// 既存コンポーネントとの互換も維持しながら段階移行できるようにする。
/// </summary>
class DamageApplySystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
