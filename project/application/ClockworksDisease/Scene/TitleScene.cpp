#include "stdafx.h"
#include "TitleScene.h"

void TitleScene::Setup() { AddSystems(); }

void TitleScene::AddSystems() {
	AddSystem(std::make_unique<No::EditSystem>());

	AddSystem(std::make_unique<No::ModelLoadSystem>());
	AddSystem(std::make_unique<No::SpriteLoadSystem>());
	AddSystem(std::make_unique<No::AnimationSystem>());

	AddSystem(std::make_unique<No::EffectEmitSystem>());
	AddSystem(std::make_unique<No::ParticleEmitterSystem>());
	AddSystem(std::make_unique<No::ParticleSystem>());

	AddSystem(std::make_unique<No::MovementSystem>());
	AddSystem(std::make_unique<No::TransformRoutineSystem>());

	AddSystem(std::make_unique<No::SpriteAnimationSystem>());

	AddSystem(std::make_unique<No::DrawManipulatorSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
	AddSystem(std::make_unique<No::DrawCameraFrustumSystem>());
}
