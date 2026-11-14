#pragma once
#include "engine/Math/Types/Quaternion.h"

namespace NoEngine::Component {
enum class BillboardFacing {
    ScreenAligned,
    CameraPosition,
};

enum class BillboardConstraint {
    Free,
    AxisLocked,
};

enum class BillboardCameraSource {
    ActiveCamera,
    RenderCamera,
};

struct BillboardComponent {
    bool enabled = true;
    BillboardFacing facing = BillboardFacing::CameraPosition;
    BillboardConstraint constraint = BillboardConstraint::Free;
    BillboardCameraSource cameraSource = BillboardCameraSource::ActiveCamera;
    // ワールド空間の固定軸。自由回転時は上方向の基準として使う。
    Math::Vector3 axis = { 0.0f, 1.0f, 0.0f };
    // ローカル+Zを正面とし、素材の向きや板面内の回転を補正する。
    Math::Quaternion rotationOffset = Math::Quaternion::IDENTITY;
};
}
