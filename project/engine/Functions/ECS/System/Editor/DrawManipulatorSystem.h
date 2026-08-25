#pragma once
#include "../ISystem.h"
#include "engine/Math/Types/Vector4.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
namespace NoEngine {
namespace ECS {

class DrawManipulatorSystem :
	public ISystem {
public:
	DrawManipulatorSystem() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(Registry& registry, float deltaTime) override;

	static bool TriggerManipulateButton();
	static void SetSelectWaypointIndex(int index);
	static void SetSelectWaypointIndex2D(int index);

private:
	bool isActive_ = false;
	bool isActivePreFrame_ = false;

	// --- TransformRoutineComponent の waypoint 編集用 ---
	bool routineEditMode_ = false;      // trueの間は選択エンティティ自体ではなくwaypointを編集する
	bool waypointIsActive_ = false;
	bool waypointIsActivePreFrame_ = false;

	// --- TransformRoutineComponent2D の waypoint 編集用 ---
	bool routineEditMode2D_ = false;
	bool waypointIsActive2D_ = false;
	bool waypointIsActivePreFrame2D_ = false;

	void Manipulate3D(Registry& registry, const Math::Vector4& sceneRect);
	void Manipulate2D(Registry& registry, const Math::Vector4& sceneRect);
	void ManipulateRoutineWaypoints(Registry& registry, const Math::Vector4& sceneRect);
	void ManipulateRoutineWaypoints2D(Registry& registry, const Math::Vector4& sceneRect);

	// Manipulate3D/ManipulateRoutineWaypointsで共用するカメラ取得処理
	bool GetActiveCamera3D(Registry& registry, Math::Matrix4x4& outView, Math::Matrix4x4& outProjection);

	// TransformRoutineComponentのkeyframeが基準とすべき空間（親のワールド行列。親がなければ単位行列）
	Math::Matrix4x4 GetParentWorld3D(Registry& registry, Component::TransformComponent* t);
	Math::Matrix4x4 GetParentWorld2D(Registry& registry, Component::Transform2DComponent* t);

	// ワールド座標をシーンウィンドウ内のスクリーン座標へ変換する（waypointマーカー描画用）
	// viewProjection は view * projection（行ベクトル規約: MakeAffineMatrix4x4等と同じ掛け順）を渡すこと
	bool WorldToScreen(const Math::Vector3& worldPos, const Math::Matrix4x4& viewProjection,
		const Math::Vector4& sceneRect, Math::Vector2& outScreenPos);
};

}
}