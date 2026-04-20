#include "CollisionFanc.h"
#include "CollisionHelper.h"
#include "../MathUtility.h"

namespace NoEngine {
namespace Math {

CapsuleAABBCollision TestCapsuleAABB(const Transform* capsuleTransform, const Math::CapsuleCollider* capsule,
    const Transform* aabbTransform, const Math::AABBCollider* aabb) {
    CapsuleAABBCollision result;

    // ワールド座標でのそれぞれのコライダーを取得
    CapsuleWorld cap = GetWorldCapsule(capsuleTransform, capsule);
    AABBWorld box = GetWorldAABB(aabbTransform, aabb);

    // カプセルの中心軸を表すベクトル
    Vector3 segment = cap.p1 - cap.p0;
    // AABBの中心
    Vector3 boxCenter = 0.5f * (box.max + box.min);

    // AABBの中心からカプセル下端へのベクトル
    Vector3 p0ToCenter = boxCenter - cap.p0;

    float segmentLengthSquared = segment.LengthSquared();

    // 線分への射影でtを求める
    float t = 0.0f;
    if (segmentLengthSquared > 0.0f) {
        t = p0ToCenter.Dot(segment) / segmentLengthSquared;
    }
    // 線分範囲へclamp
    t = std::clamp(t, 0.0f, 1.0f);

    // 線分上の最近接点を求める
    Vector3 pointOnSegment = cap.p0 + segment * t;
    // AABBにclampして最近接点を求める
    Vector3 closestOnBox = Clamp(pointOnSegment, box.min, box.max);

    // 差分ベクトル最近接点間の距離。
    Vector3 diff = pointOnSegment - closestOnBox;

    // 0除算を避けるために1e-8fでclamp
    float dist = std::max(diff.Length(),1e-8f);
    float r = cap.radius;

    // 衝突判定(最近接点間の距離が半径より小さいか)
    if (dist < r) {
        // 衝突している
        result.hit = true;
        // 貫通量
        result.penetration = r - dist;
        // 法線ベクトル
        result.normal = diff / dist;

        result.closestOnBox = closestOnBox;
        result.closestOnCapsule = pointOnSegment;
    }

    return result;
}

}
}