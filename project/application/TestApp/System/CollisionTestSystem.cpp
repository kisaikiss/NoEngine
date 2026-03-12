#include "CollisionTestSystem.h"
#include "application/TestApp/Component/Collider3DComponent.h"
#include "application/TestApp/Component/Collider2DComponent.h"
#include "application/TestApp/Component/ProjectedColliderComponent.h"
#include "application/TestApp/Utility/CoordinateConverter.h"
#include "application/TestApp/Utility/CollisionAlgorithms.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>

namespace TestApp {

	void CollisionTestSystem::Update(No::Registry& registry, float deltaTime) {
		// 未使用の警告を抑制
		static_cast<void>(deltaTime);

		//コライダーの更新
		// TransformやTransform2Dの変更を各コライダーに反映
		UpdateCollider3D(registry);
		UpdateCollider2D(registry);

		// 3Dを2D投影処理
		// カメラ情報を使用して3Dコライダーをスクリーン座標に投影
		ProjectColliders(registry);

		// 衝突判定
		// 投影された3Dコライダーと2Dコライダーの衝突をチェック
		CheckProjectedVs2D(registry);

		// 2D同士の衝突判定（未実装だけど一応奥ならここ）
		// Check2DVs2D(registry);
	}


	void CollisionTestSystem::UpdateCollider3D(No::Registry& registry) {

		// 全ての3Dコライダーを取得
		auto view = registry.View<Collider3DComponent, No::TransformComponent>();

		for (auto entity : view) {
			auto* collider = registry.GetComponent<Collider3DComponent>(entity);
			auto* transform = registry.GetComponent<No::TransformComponent>(entity);

			if (!collider || !transform) continue;
			// ワールド座標を更新
			collider->worldPosition = transform->GetWorldPosition();

			// ワールド空間での半径を計算
			if (collider->useScaleAsRadius) {
				// Transformのscaleを半径として使用
				// 非等方スケールの場合は最大値を使用
				float maxScale = std::max({
					std::abs(transform->scale.x),
					std::abs(transform->scale.y),
					std::abs(transform->scale.z)
					});
				collider->worldRadius = maxScale * collider->radiusMultiplier;
			} else {
				// 設定された半径を直接使用
				collider->worldRadius = collider->radius * collider->radiusMultiplier;
			}

			// 衝突フラグをリセット
			collider->isColliding = false;
		}
	}


	void CollisionTestSystem::UpdateCollider2D(No::Registry& registry) {

		// 全ての2Dコライダーを取得
		auto view = registry.View<Collider2DComponent, No::Transform2DComponent>();
		for (auto entity : view) {
			auto* collider = registry.GetComponent<Collider2DComponent>(entity);
			auto* transform2D = registry.GetComponent<No::Transform2DComponent>(entity);
			if (!collider || !transform2D) continue;
			// スクリーン座標を更新
			collider->screenPosition = transform2D->translate;
			// スクリーン空間でのサイズを計算
			if (collider->useTransformAsSize) {
				// Transform2Dのscaleをサイズとして使用
				collider->worldSize.x = transform2D->scale.x * collider->sizeMultiplier.x;
				collider->worldSize.y = transform2D->scale.y * collider->sizeMultiplier.y;
			} else {
				// 設定されたサイズを直接使用
				collider->worldSize.x = collider->size.x * collider->sizeMultiplier.x;
				collider->worldSize.y = collider->size.y * collider->sizeMultiplier.y;
			}

			// 衝突フラグをリセット
			collider->isColliding = false;
		}
	}



	void CollisionTestSystem::ProjectColliders(No::Registry& registry) {
		// カメラ情報を取得
		No::CameraComponent* camera = GetActiveCamera(registry);
		if (!camera) {
			return; // カメラが見つからない場合は投影処理をスキップ
		}
		// ウィンドウサイズを取得
		NoEngine::WindowSize windowSize =
			NoEngine::GraphicsCore::gWindowManager.GetMainWindow()->GetWindowSize();
		// 全ての3Dコライダー + ProjectedColliderを投影
		/// ProjectedColliderComponentは事前に追加されていないとダメな形になってる(TODO::動的にここで追加するとvector範囲外参照)
		auto view = registry.View<Collider3DComponent, ProjectedColliderComponent>();

		for (auto entity : view) {
			auto* collider3D = registry.GetComponent<Collider3DComponent>(entity);
			auto* projected = registry.GetComponent<ProjectedColliderComponent>(entity);
			if (!collider3D || !projected) continue;
			// ワールド座標をスクリーン座標に投影
			projected->screenPosition = CoordinateConverter::WorldToScreen(
				collider3D->worldPosition,
				camera->forGPU.viewProjection,
				windowSize
			);
			// ワールド半径をスクリーン半径に投影
			// 透視投影による距離補正が適用される
			projected->screenRadius = CoordinateConverter::WorldRadiusToScreen(
				collider3D->worldPosition,
				collider3D->worldRadius,
				camera->forGPU.viewProjection,
				windowSize
			);

			// 可視性を判定
			projected->isVisible = CoordinateConverter::IsOnScreen(
				projected->screenPosition,
				windowSize
			) && projected->screenRadius > 0.0f;
			// 衝突レイヤー情報をコピー
			projected->collisionLayer = collider3D->collisionLayer;
			projected->collisionMask = collider3D->collisionMask;
			// 衝突フラグをリセット
			projected->isColliding = false;
		}
	}


	void CollisionTestSystem::CheckProjectedVs2D(No::Registry& registry) {

		// 投影された3Dコライダーを取得
		auto projectedView = registry.View<ProjectedColliderComponent>();
		auto collider2DView = registry.View<Collider2DComponent>();


		// 総当たりで衝突判定
		for (auto projectedEntity : projectedView) {
			auto* projected = registry.GetComponent<ProjectedColliderComponent>(projectedEntity);
			if (!projected) continue;

			// 画面外のオブジェクトはスキップ
			if (!projected->isVisible) continue;

			// 全ての2Dコライダーとチェック
			for (auto collider2DEntity : collider2DView) {
				auto* collider2D = registry.GetComponent<Collider2DComponent>(collider2DEntity);
				if (!collider2D) continue;

				// レイヤーマスクで双方向マスクチェック（両方がtrueの場合のみ衝突判定）
				bool layer1CanCollideWith2 = (projected->collisionLayer & collider2D->collisionMask) != 0;
				bool layer2CanCollideWith1 = (collider2D->collisionLayer & projected->collisionMask) != 0;
				if (!layer1CanCollideWith2 || !layer2CanCollideWith1) {
					continue; // レイヤーが一致しないのでスキップ
				}

				// 円とAABBの衝突判定
				// 投影されたコライダー円
				// 2DコライダーAABB
				bool isColliding = CollisionAlgorithms::CheckCircleAABB(
					projected->screenPosition,
					projected->screenRadius,
					collider2D->screenPosition,
					collider2D->worldSize
				);

				// 衝突結果を設定
				if (isColliding) {
					// 投影コライダーの衝突情報を更新
					projected->isColliding = true;
					projected->collidedEntity = collider2DEntity;

					// 2Dコライダーの衝突情報を更新
					collider2D->isColliding = true;
					collider2D->collidedEntity = projectedEntity;

					// 元の3Dコライダーの衝突情報も更新
					auto* collider3D = registry.GetComponent<Collider3DComponent>(projectedEntity);
					if (collider3D) {
						collider3D->isColliding = true;
						collider3D->collidedEntity = collider2DEntity;
					}
				}
			}
		}
	}


	void CollisionTestSystem::Check2DVs2D(No::Registry& registry) {

		// 2Dコライダー同士の衝突判定
		// 今回は使用しないが、将来の拡張用に残しておく
		auto view = registry.View<Collider2DComponent>();

		for (auto entity1 : view) {
			auto* collider1 = registry.GetComponent<Collider2DComponent>(entity1);
			if (!collider1) continue;

			for (auto entity2 : view) {
				// 自分自身との衝突判定はスキップ
				if (entity1 == entity2) continue;
				auto* collider2 = registry.GetComponent<Collider2DComponent>(entity2);
				if (!collider2) continue;
				// レイヤーマスクチェック
				bool layer1CanCollideWith2 = (collider1->collisionLayer & collider2->collisionMask) != 0;
				bool layer2CanCollideWith1 = (collider2->collisionLayer & collider1->collisionMask) != 0;
				if (!layer1CanCollideWith2 || !layer2CanCollideWith1) {
					continue;
				}
				// AABBとAABBの衝突判定
				bool isColliding = CollisionAlgorithms::CheckAABBAABB(
					collider1->screenPosition,
					collider1->worldSize,
					collider2->screenPosition,
					collider2->worldSize
				);

				if (isColliding) {
					collider1->isColliding = true;
					collider1->collidedEntity = entity2;

					collider2->isColliding = true;
					collider2->collidedEntity = entity1;
				}
			}
		}
	}


	No::CameraComponent* CollisionTestSystem::GetActiveCamera(No::Registry& registry) {

		// ActiveCameraTagを持つカメラを検索
		auto view = registry.View<No::CameraComponent, No::ActiveCameraTag>();

		for (auto entity : view) {
			auto* camera = registry.GetComponent<No::CameraComponent>(entity);
			if (camera) {
				return camera;
			}
		}
		return nullptr;
	}

}
