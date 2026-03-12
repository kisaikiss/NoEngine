#include "TestScene3.h"
#include "application/TestApp/System/TestSystem.h"
#include "application/TestApp/System/CollisionTestSystem.h"
#include "application/TestApp/Component/Collider3DComponent.h"
#include "application/TestApp/Component/Collider2DComponent.h"
#include "application/TestApp/Component/ProjectedColliderComponent.h"
#include "application/TestApp/Utility/CollisionMask.h"

using namespace TestApp;


void TestScene3::Setup() {

	// CollisionTestSystemは当たり判定処理の先に実行される必要がある
	AddSystem(std::make_unique<TestApp::CollisionTestSystem>());
	AddSystem(std::make_unique<TestSystem>());
	AddSystem(std::make_unique<No::AnimationSystem>());
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());

	No::Registry& registry = *GetRegistry();

	// ========================================
	// Ball 1 (3D球体)
	// ========================================

	No::Entity ball1Entity = registry.GenerateEntity();
	auto* ball1Transform = registry.AddComponent<No::TransformComponent>(ball1Entity);
	ball1Transform->translate = { 0.f, 0.f, 5.f };
	ball1Transform->scale = { 0.5f, 0.5f, 0.5f };
	auto* ball1Tag = registry.AddComponent<No::EditTag>(ball1Entity);
	ball1Tag->name = "Ball1";

	// メッシュとマテリアル
	auto* ball1Mesh = registry.AddComponent<No::MeshComponent>(ball1Entity);
	auto* ball1Material = registry.AddComponent<No::MaterialComponent>(ball1Entity);
	NoEngine::Asset::ModelLoader::LoadModel(
		"ball",
		"resources/game/td_3105/Model/Particle/Particle.obj",
		ball1Mesh
	);
	ball1Material->materials = NoEngine::Asset::ModelLoader::GetMaterial("ball");
	ball1Material->color = { 1.0f, 1.0f, 0.0f, 1.0f }; // 黄色
	ball1Material->psoName = L"Renderer : Default PSO";
	ball1Material->psoId = NoEngine::Render::GetPSOID(ball1Material->psoName);
	ball1Material->rootSigId = NoEngine::Render::GetRootSignatureID(ball1Material->psoName);

	// コライダー設定
	// scale = 0.5, radius = 0.5 * 0.5 = 0.25
	auto* ball1Collider = registry.AddComponent<TestApp::Collider3DComponent>(ball1Entity);
	ball1Collider->radius = 0.5f;				// Ballの直径
	ball1Collider->radiusMultiplier = 0.5f;		// 半径なので1/2
	ball1Collider->useScaleAsRadius = false;	// radiusを直接使用
	ball1Collider->collisionLayer = CollisionType::Player;
	ball1Collider->collisionMask = CollisionMask::Player;

	// スクリーン判定Componentを事前に追加(動的追加はvector範囲外参照する？)
	auto* ball1Projected = registry.AddComponent<TestApp::ProjectedColliderComponent>(ball1Entity);
	ball1Projected->source3DEntity = ball1Entity;

	// ========================================
	// Ball 2 (3D球体) 
	// ========================================

	No::Entity ball2Entity = registry.GenerateEntity();
	auto* ball2Transform = registry.AddComponent<No::TransformComponent>(ball2Entity);
	ball2Transform->translate = { 2.f, 0.f, 5.f };
	ball2Transform->scale = { 0.3f, 0.3f, 0.3f };
	auto* ball2Tag = registry.AddComponent<No::EditTag>(ball2Entity);
	ball2Tag->name = "Ball2";

	// メッシュとマテリアル
	auto* ball2Mesh = registry.AddComponent<No::MeshComponent>(ball2Entity);
	auto* ball2Material = registry.AddComponent<No::MaterialComponent>(ball2Entity);

	// 既に読み込まれているモデルを取得
	NoEngine::Asset::ModelLoader::GetModel("ball", ball2Mesh);
	ball2Material->materials = NoEngine::Asset::ModelLoader::GetMaterial("ball");
	ball2Material->color = { 0.0f, 1.0f, 1.0f, 1.0f }; // シアン
	ball2Material->psoName = L"Renderer : Default PSO";
	ball2Material->psoId = NoEngine::Render::GetPSOID(ball2Material->psoName);
	ball2Material->rootSigId = NoEngine::Render::GetRootSignatureID(ball2Material->psoName);

	// コライダー設定（Transformのscaleを使用する方法で動作確認）
	auto* ball2Collider = registry.AddComponent<TestApp::Collider3DComponent>(ball2Entity);
	ball2Collider->useScaleAsRadius = true;		// Transformのscaleを使用
	ball2Collider->radiusMultiplier = 0.5f;		// スケールの半分を半径にする
	ball2Collider->collisionLayer = CollisionType::PlayerBullet;
	ball2Collider->collisionMask = CollisionMask::PlayerBullet;

	// スクリーン判定Componentを事前に追加(動的追加はvector範囲外参照する？)
	auto* ball2Projected = registry.AddComponent<TestApp::ProjectedColliderComponent>(ball2Entity);
	ball2Projected->source3DEntity = ball2Entity;

	// ========================================
	// Sprite 1
	// ========================================

	No::Entity sprite1Entity = registry.GenerateEntity();
	auto* sprite1Transform2D = registry.AddComponent<No::Transform2DComponent>(sprite1Entity);
	sprite1Transform2D->translate = { 640.f, 360.f };
	sprite1Transform2D->scale = { 100.f, 100.f };
	auto* sprite1Sprite = registry.AddComponent<No::SpriteComponent>(sprite1Entity);
	sprite1Sprite->layer = 10;
	sprite1Sprite->color = { 0.f, 0.f, 1.f, 1.f }; // 初期色: 青
	sprite1Sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
	auto* sprite1Tag = registry.AddComponent<No::EditTag>(sprite1Entity);
	sprite1Tag->name = "Sprite1";

	// コライダー設定
	// scale = 100, size = 100 * 1.0 = 100
	auto* sprite1Collider = registry.AddComponent<TestApp::Collider2DComponent>(sprite1Entity);
	sprite1Collider->useTransformAsSize = true;			// Transformのscaleを使用
	sprite1Collider->sizeMultiplier = { 1.0f, 1.0f };	// スケールをそのまま使用
	sprite1Collider->collisionLayer = CollisionType::Enemy;
	sprite1Collider->collisionMask = CollisionMask::Enemy;

	// ========================================
	// Sprite 2(若干小さいコライダーの判定)
	// ========================================

	No::Entity sprite2Entity = registry.GenerateEntity();
	auto* sprite2Transform2D = registry.AddComponent<No::Transform2DComponent>(sprite2Entity);
	sprite2Transform2D->translate = { 400.f, 300.f };
	sprite2Transform2D->scale = { 80.f, 80.f };
	auto* sprite2Sprite = registry.AddComponent<No::SpriteComponent>(sprite2Entity);
	sprite2Sprite->layer = 10;
	sprite2Sprite->color = { 0.f, 0.f, 1.f, 1.f }; // 初期色: 青
	sprite2Sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
	auto* sprite2Tag = registry.AddComponent<No::EditTag>(sprite2Entity);
	sprite2Tag->name = "Sprite2";

	// コライダー設定
	auto* sprite2Collider = registry.AddComponent<TestApp::Collider2DComponent>(sprite2Entity);
	sprite2Collider->useTransformAsSize = true;
	sprite2Collider->sizeMultiplier = { 0.9f, 0.9f };	// 見た目の90%のコライダー
	sprite2Collider->collisionLayer = CollisionType::Enemy;
	sprite2Collider->collisionMask = CollisionMask::Enemy;



	// ========================================
	// Cube Block（3D AABB地形）
	// ========================================

	No::Entity cubeEntity = registry.GenerateEntity();
	auto* cubeTransform = registry.AddComponent<No::TransformComponent>(cubeEntity);
	cubeTransform->translate = { 0.f, 0.f, 7.f };	// Ball1の少し奥
	cubeTransform->scale = { 1.f, 1.f, 1.f };		// 1x1x1
	auto* cubeTag = registry.AddComponent<No::EditTag>(cubeEntity);
	cubeTag->name = "CubeBlock";

	// メッシュとマテリアル
	auto* cubeMesh = registry.AddComponent<No::MeshComponent>(cubeEntity);
	auto* cubeMaterial = registry.AddComponent<No::MaterialComponent>(cubeEntity);
	NoEngine::Asset::ModelLoader::LoadModel(
		"cube",
		"resources/game/td_3105/Model/cube/cube.obj",
		cubeMesh
	);
	cubeMaterial->materials = NoEngine::Asset::ModelLoader::GetMaterial("cube");
	cubeMaterial->color = { 0.5f, 0.5f, 0.5f, 1.0f }; // 初期色: グレー
	cubeMaterial->psoName = L"Renderer : Default PSO";
	cubeMaterial->psoId = NoEngine::Render::GetPSOID(cubeMaterial->psoName);
	cubeMaterial->rootSigId = NoEngine::Render::GetRootSignatureID(cubeMaterial->psoName);

	// コライダー設定（AABB Box）
	// scale = (1,1,1) をそのままボックスサイズに使用
	auto* cubeCollider = registry.AddComponent<TestApp::Collider3DComponent>(cubeEntity);
	cubeCollider->shapeType = TestApp::ShapeType3D::Box;
	cubeCollider->useScaleAsBox = true;				// Transformのscaleをそのまま使用
	cubeCollider->boxSizeMultiplier = { 1.f, 1.f, 1.f };
	cubeCollider->collisionLayer = CollisionType::Block;
	cubeCollider->collisionMask = CollisionMask::Block;

	// スクリーン投影用（デバッグ描画のため）
	auto* cubeProjected = registry.AddComponent<TestApp::ProjectedColliderComponent>(cubeEntity);
	cubeProjected->source3DEntity = cubeEntity;


	// ========================================
	// 既存のものも動作するか確認
	// ========================================

	//girlのアニメーションモデル
	No::Entity entity = registry.GenerateEntity();
	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	auto* t = registry.AddComponent<No::TransformComponent>(entity);
	auto* imguiName = registry.AddComponent<No::EditTag>(entity);
	imguiName->name = "girl";
	t->rotation.FromAxisAngle(No::Vector3(0.f, 1.f, 0.f), PI);
	t->translate = { 0.f,-6.5f,4.f };
	auto* m = registry.AddComponent<No::MaterialComponent>(entity);
	auto* a = registry.AddComponent<No::AnimatorComponent>(entity);
	No::ModelLoader::LoadModel("magiclash", "resources/engine/Model/test/TD_girl/test7.gltf");
	No::ModelLoader::GetModel("magiclash", model, a);
	m->materials = No::ModelLoader::GetMaterial("magiclash");
	m->drawOutline = true;
	m->enableSkinning = true;
	m->psoName = L"Renderer : DefaultSkinned PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);

	// 既存のSprite
	auto* t2d = registry.AddComponent<No::Transform2DComponent>(entity);
	t2d->translate = { 100.f, 200.f };
	auto* sprite = registry.AddComponent<No::SpriteComponent>(entity);
	sprite->layer = 1;
	t2d->scale = { 100.f, 100.f };
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");


	// ライト
	auto light2 = registry.GenerateEntity();
	auto* dir2 = registry.AddComponent<No::DirectionalLightComponent>(light2);
	dir2->color = { 1.f,1.f,1.f,1.f };
	dir2->direction = { 0.f,-1.f,0.f };
	dir2->intensity = 1.f;


	// カメラ
	auto camera = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(camera);
	registry.AddComponent<No::CameraComponent>(camera);
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;
}

void TestScene3::NotSystemUpdate() {

#ifdef USE_IMGUI
	ImGui::Begin("ChangeScene");
	if (ImGui::Button("SceneChange")) {
		No::SceneChangeEvent event;
		event.nextScene = "TestScene";
		GetRegistry()->EmitEvent(event);
	}
	ImGui::End();


	// カメラ手動調整ウィンドウ
	ImGui::Begin("camera");
	No::Registry& registry = *GetRegistry();
	auto cameraView = registry.View<No::ActiveCameraTag, No::TransformComponent>();
	auto camIt = cameraView.begin();
	if (camIt != cameraView.end()) {
		auto* cameraTransform = registry.GetComponent<No::TransformComponent>(*camIt);
		ImGui::DragFloat3("pos", &cameraTransform->translate.x, 0.1f);
		ImGui::DragFloat3("rot", &cameraTransform->rotation.x, 0.1f);
	}
	ImGui::End();


	DrawCollisionImGui();
#endif // USE_IMGUI
}

void TestScene3::DrawCollisionImGui()
{

	No::Registry& registry = *GetRegistry();

	// 衝突結果を可視化（3Dモデルの色変更）
	auto meshView = registry.View<TestApp::Collider3DComponent, No::MaterialComponent>();
	for (auto entity : meshView) {
		auto* collider3D = registry.GetComponent<TestApp::Collider3DComponent>(entity);
		auto* material = registry.GetComponent<No::MaterialComponent>(entity);
		if (!collider3D || !material) continue;

		// 衝突している場合は赤、していない場合は白
		if (collider3D->isColliding) {
			material->color = { 1.0f, 0.0f, 0.0f, 1.0f }; // 赤
		} else {
			material->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白
		}
	}

	// 衝突結果を可視化（2Dスプライトの色変更）

	auto spriteView = registry.View<TestApp::Collider2DComponent, No::SpriteComponent>();
	for (auto entity : spriteView) {
		auto* collider = registry.GetComponent<TestApp::Collider2DComponent>(entity);
		auto* sprite = registry.GetComponent<No::SpriteComponent>(entity);

		if (!collider || !sprite) continue;

		// 衝突している場合は赤、していない場合は青
		if (collider->isColliding) {
			sprite->color = { 1.0f, 0.0f, 0.0f, 1.0f }; // 赤
		} else {
			sprite->color = { 0.0f, 0.0f, 1.0f, 1.0f }; // 青
		}
	}


	// デバッグ情報の表示
#ifdef USE_IMGUI
	ImGui::Begin("Collision Debug");

	// ----------------------------------------
	// 3Dボールの情報
	// ----------------------------------------

	ImGui::Text("3D Objects (Projected)");

	auto ball3DView = registry.View<TestApp::Collider3DComponent, TestApp::ProjectedColliderComponent, No::EditTag>();
	for (auto entity : ball3DView) {
		auto* collider3D = registry.GetComponent<TestApp::Collider3DComponent>(entity);
		auto* projected = registry.GetComponent<TestApp::ProjectedColliderComponent>(entity);
		auto* tag = registry.GetComponent<No::EditTag>(entity);

		if (!collider3D || !projected || !tag) continue;

		ImGui::Separator();
		ImGui::Text("Name: %s", tag->name.c_str());
		ImGui::Text("  World Pos: (%.2f, %.2f, %.2f)",
			collider3D->worldPosition.x,
			collider3D->worldPosition.y,
			collider3D->worldPosition.z);
		ImGui::Text("  World Radius: %.2f", collider3D->worldRadius);
		ImGui::Text("  Screen Pos: (%.2f, %.2f)",
			projected->screenPosition.x,
			projected->screenPosition.y);
		ImGui::Text("  Screen Radius: %.2f", projected->screenRadius);
		ImGui::Text("  Visible: %s", projected->isVisible ? "YES" : "NO");
		ImGui::Text("  Colliding: %s", collider3D->isColliding ? "YES" : "NO");

		// デバッグ描画（緑の球体）
		if (projected->isVisible) {
			NoEngine::Primitive::DrawSphere(
				collider3D->worldPosition,
				collider3D->worldRadius,
				{ 0.0f, 1.0f, 0.0f, 0.5f }
			);
		}
	}

	// ----------------------------------------
	// 2Dスプライトの情報
	// ----------------------------------------

	ImGui::Text("");
	ImGui::Text("2D Sprites");

	auto sprite2DView = registry.View<TestApp::Collider2DComponent, No::EditTag>();
	for (auto entity : sprite2DView) {
		auto* collider2D = registry.GetComponent<TestApp::Collider2DComponent>(entity);
		auto* tag = registry.GetComponent<No::EditTag>(entity);

		if (!collider2D || !tag) continue;

		ImGui::Separator();
		ImGui::Text("Name: %s", tag->name.c_str());
		ImGui::Text("  Screen Pos: (%.2f, %.2f)",
			collider2D->screenPosition.x,
			collider2D->screenPosition.y);
		ImGui::Text("  Size: (%.2f, %.2f)",
			collider2D->worldSize.x,
			collider2D->worldSize.y);
		ImGui::Text("  Colliding: %s", collider2D->isColliding ? "YES" : "NO");
	}

	ImGui::End();
#endif
}