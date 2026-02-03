#include "SceneManager.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Functions/ECS/Component/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Math/Easing.h"
#include <algorithm>

namespace NoEngine
{
	//元のコード
	//void Scene::SceneManager::ChangeScene(const std::string& name)
	//{
	//	auto it = factories_.find(name);
	//	if (it == factories_.end()) return;
	//
	//	if (currentScene_)
	//	{
	//		currentScene_->OnExit();
	//	}
	//
	//	currentScene_ = it->second();
	//	currentScene_->Setup();
	//	currentScene_->OnEnter();
	//}

	void Scene::SceneManager::ChangeScene(const std::string& name, bool immediate)
	{
		auto it = factories_.find(name);
		if (it == factories_.end()) return;

		if (immediate || !currentScene_)
		{
			if (currentScene_)
			{
				currentScene_->OnExit();
			}

			currentScene_ = it->second();
			currentScene_->Setup();
			currentScene_->OnEnter();

			// クリア遷移状態
			isTransitioning_ = false;
			transitionPhase_ = TransitionPhase::None;
			overlayEntity_ = 0;
			return;
		}

		// フェード遷移を開始
		pendingName_ = name;
		isTransitioning_ = true;
		transitionPhase_ = TransitionPhase::FadingOut;
		transitionTimer_ = 0.0f;

		// オーバーレイを現在のシーンの Registry に作成（黒、alpha=0）
		auto registry = currentScene_->GetRegistry();
		if (registry)
		{
			auto winSize = GraphicsCore::gWindowManager.GetMainWindow()->GetWindowSize();
			overlayEntity_ = registry->GenerateEntity();
			auto* t2d = registry->AddComponent<Component::Transform2DComponent>(overlayEntity_);
			t2d->scale = { static_cast<float>(winSize.clientWidth), static_cast<float>(winSize.clientHeight) };
			t2d->translate = { static_cast<float>(winSize.clientWidth) * 0.5f, static_cast<float>(winSize.clientHeight) * 0.5f };

			auto* sprite = registry->AddComponent<Component::SpriteComponent>(overlayEntity_);

			sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
			sprite->name = "SceneFadeOverlay";
			sprite->layer = 100;
			sprite->color = { 0.f, 0.f, 0.f, 0.f };
		}
	}

	void Scene::SceneManager::Update(float deltaTime)
	{
		// フェード遷移中の挙動を優先して処理
		if (isTransitioning_)
		{
			float half = transitionDuration_ * 0.5f;
			if (deltaTime > 0.0f && deltaTime < 0.1f)
				transitionTimer_ += deltaTime;

			//現在の overlayEntity_ の alpha を更新
			auto updateOverlayAlpha = [&](float a)
				{
					if (currentScene_)
					{
						auto registry = currentScene_->GetRegistry();
						if (registry && overlayEntity_ != 0 && registry->Has<Component::SpriteComponent>(overlayEntity_))
						{
							auto* sp = registry->GetComponent<Component::SpriteComponent>(overlayEntity_);
							sp->color.a = std::clamp(a, 0.0f, 1.0f);
						}
					}
				};

			if (transitionPhase_ == TransitionPhase::FadingOut)
			{
				// 線形ではなくイージングで alpha を補間
				float t = std::clamp(transitionTimer_ / half, 0.0f, 1.0f);
				float eased = NoEngine::Easing::EaseInOutSine(0.0f, 1.0f, t);
				updateOverlayAlpha(eased);

				// フェードアウト完了したらロード移行
				if (transitionTimer_ >= half)
				{
					if (currentScene_) currentScene_->OnExit();

					auto it = factories_.find(pendingName_);
					if (it != factories_.end())
					{
						currentScene_ = it->second();
						// 次シーンの Setup を呼ぶ（ここでモデル／テクスチャ等を読み込む）
						currentScene_->Setup();
						currentScene_->OnEnter();
					}

					// 新シーンの Registry にオーバーレイ作成
					if (currentScene_)
					{
						auto registry = currentScene_->GetRegistry();
						if (registry)
						{
							auto winSize = GraphicsCore::gWindowManager.GetMainWindow()->GetWindowSize();
							overlayEntity_ = registry->GenerateEntity();
							auto* t2d = registry->AddComponent<Component::Transform2DComponent>(overlayEntity_);
							t2d->scale = { static_cast<float>(winSize.clientWidth), static_cast<float>(winSize.clientHeight) };
							t2d->translate = { static_cast<float>(winSize.clientWidth) * 0.5f, static_cast<float>(winSize.clientHeight) * 0.5f };

							auto* sprite = registry->AddComponent<Component::SpriteComponent>(overlayEntity_);
							sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
							sprite->name = "SceneFadeOverlay";
							sprite->layer = 10000;
							sprite->color = { 0.f, 0.f, 0.f, 1.f };
						}
					}

					// 次フェーズへ
					transitionPhase_ = TransitionPhase::FadingIn;
					transitionTimer_ = 0.0f;
				}
			}
			else if (transitionPhase_ == TransitionPhase::FadingIn)
			{
				// 逆向きもイージングで
				float t = std::clamp(transitionTimer_ / half, 0.0f, 1.0f);
				float eased = NoEngine::Easing::EaseInOutSine(0.0f, 1.0f, t);
				updateOverlayAlpha(1.0f - eased);

				if (transitionTimer_ >= half)
				{
					// フェード終了。オーバーレイを削除して終了
					if (currentScene_)
					{
						auto registry = currentScene_->GetRegistry();
						if (registry && overlayEntity_ != 0 && registry->Has<Component::SpriteComponent>(overlayEntity_))
						{
							registry->DestroyEntity(overlayEntity_);
						}
					}
					overlayEntity_ = 0;
					isTransitioning_ = false;
					transitionPhase_ = TransitionPhase::None;
					transitionTimer_ = 0.0f;
				}
			}
			if (currentScene_)
			{
				currentScene_->Update(deltaTime);
			}
			return;
		}

		// 通常更新
		if (currentScene_) currentScene_->Update(deltaTime);
	}
}
