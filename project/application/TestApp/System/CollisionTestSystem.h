#pragma once
#include "engine/NoEngine.h"

namespace TestApp {

	/// <summary>
	/// 衝突判定テストシステム
	/// 3Dオブジェクトと2Dスプライト間の衝突判定を管理する
	/// コライダーの更新（座標・サイズの同期）、3D→2D投影処理、衝突判定の実行
	/// </summary>
	class CollisionTestSystem : public No::ISystem {
	public:
		/// <summary>
		/// システムの更新処理
		/// 毎フレーム呼ばれ、全ての衝突判定を実行する
		/// </summary>
		void Update(No::Registry& registry, float deltaTime) override;

	private:
		// ========================================
		// コライダー更新
		// ========================================

		/// <summary>
		/// 3Dコライダーのワールド座標と半径を更新
		/// TransformComponentから情報を取得し、コライダーに反映
		/// </summary>
		void UpdateCollider3D(No::Registry& registry);

		/// <summary>
		/// 2Dコライダーのスクリーン座標とサイズを更新
		/// Transform2DComponentから情報を取得し、コライダーに反映
		/// </summary>
		void UpdateCollider2D(No::Registry& registry);

		// ========================================
		// 投影処理
		// ========================================

		/// <summary>
		/// 3Dコライダーを2Dスクリーン座標に投影
		/// カメラ情報を使用して透視投影を適用し、ProjectedColliderComponentを更新
		/// </summary>
		void ProjectColliders(No::Registry& registry);

		// ========================================
		// 衝突判定
		// ========================================

		/// <summary>
		/// 投影された3Dコライダーと2Dコライダーの衝突判定
		/// レイヤーマスクによるフィルタリングを適用
		/// </summary>
		void CheckProjectedVs2D(No::Registry& registry);

		/// <summary>
		/// 2Dコライダー同士の衝突判定
		/// 現在は使用していない拡張用
		/// </summary>
		void Check2DVs2D(No::Registry& registry);

		// ========================================
		// ヘルパー関数
		// ========================================

		/// <summary>
		/// アクティブなカメラコンポーネントを取得
		/// </summary>
		/// <returns>カメラコンポーネントへのポインタ、見つからない場合nullptr</returns>
		No::CameraComponent* GetActiveCamera(No::Registry& registry);
	};

}
