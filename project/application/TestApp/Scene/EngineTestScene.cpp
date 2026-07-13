#include "EngineTestScene.h"
#include "application/TestApp/System/TestSystem.h"
#include "engine/Assets/Font/FontManager.h"

namespace {
No::Entity cameraE;
}

void EngineTestScene::Setup() {
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<No::DrawManipulatorSystem>());
	AddSystem(std::make_unique<TestSystem>());
	AddSystem(std::make_unique<No::ModelLoadSystem>());
	AddSystem(std::make_unique<No::AnimationSystem>());
	AddSystem(std::make_unique<No::SpriteAnimationSystem>());
	AddSystem(std::make_unique<No::ParticleEmitterSystem>());
	AddSystem(std::make_unique<No::EffectEmitSystem>());
	AddSystem(std::make_unique<No::ParticleSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
	AddSystem(std::make_unique<No::MovementSystem>());

	No::Registry& registry = *GetRegistry();
	No::Entity entity = registry.GenerateEntity();
	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	auto* t = registry.AddComponent<No::TransformComponent>(entity);
	auto* imguiName = registry.AddComponent<No::EditTag>(entity);
	imguiName->name = "girl";
	t->rotation.FromAxisAngle(No::Vector3(0.f, 1.f, 0.f), PI);
	t->translate = { 0.f,-1.5f,4.f };
	auto* m = registry.AddComponent<No::MaterialComponent>(entity);
	registry.AddComponent<No::AnimatorComponent>(entity)->enableSkinning = true;
	model->meshName = "resources/engine/Model/test/TD_girl/test7.gltf";
	m->drawOutline = true;
	auto* text = registry.AddComponent<No::TextComponent>(entity);
	text->fontHandle = NoEngine::FontManager::LoadFontFile("resources/engine/fonts/build/Isego.font");
	

	No::Entity background = registry.GenerateEntity();
	auto* bm = registry.AddComponent<No::MeshComponent>(background);
	bm->meshName = "resources/engine/Model/terrain/terrain.obj";
	registry.AddComponent<No::MaterialComponent>(background);
	registry.AddComponent<No::TransformComponent>(background);
	auto* backgroundTag = registry.AddComponent<No::EditTag>(background);
	backgroundTag->name = "background";

	auto light = registry.GenerateEntity();
	auto* dir = registry.AddComponent<No::DirectionalLightComponent>(light);
	dir->color = { 1.f,1.f,1.f,1.f };
	dir->direction = { 0.f,-1.f,0.f };
	dir->intensity = 0.1f;
	auto* lightTag = registry.AddComponent<No::EditTag>(light);
	lightTag->name = "directionalLight";

	auto pointLight = registry.GenerateEntity();
	auto* point = registry.AddComponent<No::PointLightComponent>(pointLight);
	point->color = { 1.f,1.f,1.f,1.f };
	point->radius = 1.f;
	point->intensity = 1.f;
	auto* pointTransform = registry.AddComponent<No::TransformComponent>(pointLight);
	pointTransform = {};
	auto* pointLightTag = registry.AddComponent<No::EditTag>(pointLight);
	pointLightTag->name = "pointLight";

	auto spotLight = registry.GenerateEntity();
	auto* spot = registry.AddComponent<No::SpotLightComponent>(spotLight);
	spot->color = No::Color::WHITE;
	auto* spotTransform = registry.AddComponent<No::TransformComponent>(spotLight);
	spotTransform = {};
	auto* spotTag = registry.AddComponent<No::EditTag>(spotLight);
	spotTag->name = "SpotLight";

	auto* t2d2 = registry.AddComponent<No::Transform2DComponent>(light);
	t2d2->translate = { 100.f, 200.f };
	auto* sprite2 = registry.AddComponent<No::SpriteComponent>(light);
	sprite2->layer = 1;

	t2d2->scale = { 100.f, 100.f };
	sprite2->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/uvChecker.png");

	auto camera = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(camera);
	registry.AddComponent<No::CameraComponent>(camera);
	registry.AddComponent<No::DebugCameraComponent>(camera);
	auto* cameraEditTag = registry.AddComponent<No::EditTag>(camera);
	cameraEditTag->name = "camera";
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;

	cameraE = registry.GenerateEntity();
	registry.AddComponent<No::CameraComponent>(cameraE);
	auto* cameraTransform2 = registry.AddComponent<No::TransformComponent>(cameraE);
	cameraTransform2->translate.z = -5.f;

	{
		auto camera2d = registry.GenerateEntity();
		registry.AddComponent<No::ActiveCamera2DTag>(camera2d);
		registry.AddComponent<No::Camera2DComponent>(camera2d);
		registry.AddComponent<No::Transform2DComponent>(camera2d);
		auto* camera2dTag = registry.AddComponent<No::EditTag>(camera2d);
		camera2dTag->name = "CAMERA2D";
	}

	auto emitterE = registry.GenerateEntity();
	auto* emitter = registry.AddComponent<No::ParticleEmitterComponent>(emitterE);
	emitter->texture = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Texture/gradationLine.png");
	emitter->count = 10;
	emitter->frequency = 0.1f;
	emitter->shape = No::ParticleShape::kCylinder;
	registry.AddComponent<No::TransformComponent>(emitterE);
	auto* emitterTag = registry.AddComponent<No::EditTag>(emitterE);
	emitterTag->name = "emitter";

	auto emitterE2 = registry.GenerateEntity();
	auto* emitter2 = registry.AddComponent<No::ParticleEmitterComponent>(emitterE2);
	emitter2->texture = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
	emitter2->count = 10;
	emitter2->frequency = 0.1f;
	auto* emitterT = registry.AddComponent<No::TransformComponent>(emitterE2);
	emitterT->translate.x = 4.f;
	auto* emitterTag2 = registry.AddComponent<No::EditTag>(emitterE2);
	emitterTag2->name = "emitter2";

	auto emitterE3 = registry.GenerateEntity();
	auto* emitter3 = registry.AddComponent<No::ParticleEmitterComponent>(emitterE3);
	emitter3->texture = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/test/TD_girl/face.png");
	emitter3->count = 10;
	emitter3->frequency = 0.1f;
	auto* emitterT2 = registry.AddComponent<No::TransformComponent>(emitterE3);
	emitterT2->translate.x = -4.f;
	auto* emitterTag3 = registry.AddComponent<No::EditTag>(emitterE3);
	emitterTag3->name = "emitter3";

	{
		auto e = registry.GenerateEntity();
		auto* effect = registry.AddComponent<No::EffectEmitterComponent>(e);
		effect->texture = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Texture/circle.png");
		effect->count = 3;
		effect->maxSpeed = 0;
		effect->minSpeed = 0;
		effect->maxScale = 10.f;
		effect->minScale = 1.f;
		registry.AddComponent<No::EditTag>(e)->name = "effectEmitter";
		registry.AddComponent<No::TransformComponent>(e);
	}

	{
		auto e = registry.GenerateEntity();
		auto* cubeMesh = registry.AddComponent<No::MeshComponent>(e);
		registry.AddComponent<No::MaterialComponent>(e);
		registry.AddComponent<No::AnimatorComponent>(e);
		cubeMesh->meshName = "resources/engine/Model/AnimatedCube/AnimatedCube.gltf";

		registry.AddComponent<No::EditTag>(e)->name = "AnimeCube";
		registry.AddComponent<No::TransformComponent>(e);
	}
}

void EngineTestScene::NotSystemUpdate() {
#ifdef USE_IMGUI
	

#endif // USE_IMGUI
}
