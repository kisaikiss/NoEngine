#pragma once
#include "../ISystem.h"
#include "../../Event/CollisionEvents.h"
#include <engine/Functions/ECS/Registry.h>
namespace NoEngine {
namespace ECS{
class NarrowPhase2DSystem :
    public ISystem {
public:
    enum class TestAxis {
        Horizontal,
        Vertical,
        All
    };

    NarrowPhase2DSystem(TestAxis axis = TestAxis::All) : axis_(axis) {}
    void Update(Registry& registry, float deltaTime) override;

private:
    struct BroadPhasePair {
        Entity a;
        Entity b;
    };

    Math::ContactPosition ClassifyContact(const Math::Vector2& normal);

    /// <summary>
    /// レジストリの情報を基にグリッド方式でブロードフェーズ判定を行い、潜在的な衝突ペアを収集して返します。
    /// </summary>
    /// <param name="registry">エンティティやコライダ情報を保持するレジストリへの参照。判定対象の情報を参照します。</param>
    /// <param name="cellSize">グリッドセルのサイズ（ワールド単位）。セルサイズに基づいてオブジェクトをバケット化します。</param>
    /// <returns>潜在的に接触する可能性のある BroadPhasePair の配列（std::vector）。</returns>
    std::vector<BroadPhasePair> BroadPhaseGrid(Registry& registry, float cellSize);
    TestAxis axis_;
};
}
}
