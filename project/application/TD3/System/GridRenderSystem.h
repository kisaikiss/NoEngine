#pragma once
#include "engine/NoEngine.h"

// グリッド描画システム
// グリッドの線を赤色で描画
class GridRenderSystem : public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
    
private:
    // グリッド座標をワールド座標に変換
    No::Vector3 GridToWorld(int x, int y);
};
