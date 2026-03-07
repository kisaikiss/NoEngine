#include "TestScene.h"
#include "application/TestApp/System/TestSystem.h"

void TestScene::Setup() {
	AddSystem(std::make_unique<TestSystem>());
	AddSystem(std::make_unique<No::AnimationSystem>());
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());

	No::Registry& registry = *GetRegistry();
	
	// Ball Entity (3D球体)
	No::Entity ballEntity = registry.GenerateEntity();
	auto* ballTransform = registry.AddComponent<No::TransformComponent>(ballEntity);
	ballTransform->translate = { 0.f, 0.f, 5.f };
	ballTransform->scale = { 0.5f, 0.5f, 0.5f };
	auto* ballTag = registry.AddComponent<No::EditTag>(ballEntity);
	ballTag->name = "Ball";

	auto* mesh = registry.AddComponent<No::MeshComponent>(ballEntity);
	auto* material = registry.AddComponent<No::MaterialComponent>(ballEntity);

	NoEngine::Asset::ModelLoader::LoadModel(
		"ball",
		"resources/game/td_3105/Model/Particle/Particle.obj",
		mesh
	);
	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("ball");
	material->color = { 1.0f, 1.0f, 0.0f, 1.0f };
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);


	// Girl Model (既存のモデル)
	No::Entity entity = registry.GenerateEntity();
	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	auto* t = registry.AddComponent<No::TransformComponent>(entity);
	auto* imguiName = registry.AddComponent<No::EditTag>(entity);
	imguiName->name = "girl";
	t->rotation.FromAxisAngle(No::Vector3(0.f, 1.f, 0.f), PI);
	t->translate = { 0.f,-1.5f,4.f };
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

	// Collision Test Sprite (新規追加 - 当たり判定用)
	No::Entity collisionSprite = registry.GenerateEntity();
	auto* collisionT2d = registry.AddComponent<No::Transform2DComponent>(collisionSprite);
	collisionT2d->translate = { 640.f, 360.f };
	collisionT2d->scale = { 100.f, 100.f };
	auto* collisionSpr = registry.AddComponent<No::SpriteComponent>(collisionSprite);
	collisionSpr->layer = 10;
	collisionSpr->color = { 0.f, 0.f, 1.f, 1.f }; // 初期色: 青
	collisionSpr->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
	auto* collisionTag = registry.AddComponent<No::EditTag>(collisionSprite);
	collisionTag->name = "CollisionSprite";

	// 既存のSprite (参考用に残しておく)
	auto* t2d = registry.AddComponent<No::Transform2DComponent>(entity);
	t2d->translate = { 100.f, 200.f };
	auto* sprite = registry.AddComponent<No::SpriteComponent>(entity);
	sprite->layer = 1;
	t2d->scale = { 100.f, 100.f };
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");

	auto light = registry.GenerateEntity();
	auto* dir = registry.AddComponent<No::DirectionalLightComponent>(light);
	dir->color = { 1.f,1.f,1.f,1.f };
	dir->direction = { 0.f,-1.f,0.f };
	dir->intensity = 1.f;

	auto* t2d2 = registry.AddComponent<No::Transform2DComponent>(light);
	t2d2->translate = { 100.f, 200.f };
	auto* sprite2 = registry.AddComponent<No::SpriteComponent>(light);
	sprite2->layer = 1;

	t2d2->scale = { 100.f, 100.f };
	sprite2->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");

	auto light2 = registry.GenerateEntity();
	auto* dir2 = registry.AddComponent<No::DirectionalLightComponent>(light2);
	dir2->color = { 1.f,1.f,1.f,1.f };
	dir2->direction = { 0.f,-1.f,0.f };
	dir2->intensity = 1.f;

	auto camera = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(camera);
	registry.AddComponent<No::CameraComponent>(camera);
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;
}

void TestScene::NotSystemUpdate() {
#ifdef USE_IMGUI
	ImGui::Begin("ChangeScene");
	if (ImGui::Button("SceneChange")) {
		No::SceneChangeEvent event;
		event.nextScene = "TestScene2";
		GetRegistry()->EmitEvent(event);
	}
	ImGui::End();


	// カメラ手動調整ウィンドウ（自動配置後の微調整用）
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


#endif // USE_IMGUI
}
