#include "VausControlSystem.h"
#include "../../tag.h"
#include "../../Component/PhysicsComponent.h"
#include "../../Component/RingAnimationComponent.h"
#include "../../Component/VausStateComponent.h"
#include "../../Component/BallStateComponent.h"

#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Math/Easing.h"

using namespace NoEngine;

namespace
{
	constexpr float kRingInitialRadius = 4.85f;
	float sRingRadius = kRingInitialRadius;
}

void VausControlSystem::Update(No::Registry& registry, float deltaTime)
{
	auto vausView = registry.View<
		No::TransformComponent,
		No::MaterialComponent,
		VausStateComponent>();

	auto ballView = registry.View<
		BallTag,
		No::TransformComponent,
		No::MaterialComponent>();

	auto ringView = registry.View<
		RingTag,
		No::TransformComponent,
		No::MaterialComponent,
		RingAnimationComponent>();

	float angle = CalculateMouseAngle();
	static float currentCharge = 0.0f;
	isPress_ = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	static float chargeTime = 0.0f;
	static float power = 0.0f;
	for (auto entity : ringView)
	{
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* ringAnimation = registry.GetComponent<RingAnimationComponent>(entity);
		if (isPress_)
		{
			if (!wasPress_)
			{
				power = 0.0f;
			}
			ringAnimation->releaseTime = 0.0f;
			ringAnimation->pressedTime += deltaTime * 2.0f;
			ringAnimation->pressedTime = std::clamp(ringAnimation->pressedTime, 0.0f, 1.0f);
			chargeTime = Easing::Lerp(0.0f, 1.0f, ringAnimation->pressedTime);
			ringAnimation->tTemp = chargeTime;
			if (chargeTime >= 1.0f)
			{
				transform->translate.x += static_cast<float>( rand() % 3 - 1) * deltaTime;
				transform->translate.y += static_cast<float>(rand() % 3 - 1) * deltaTime;
			}
		}
		else
		{
			transform->translate = Vector3::ZERO;
			if (wasPress_ && !isPress_)
			{
				power = ringAnimation->tTemp;
			}
			ringAnimation->pressedTime = 0.0f;
			ringAnimation->releaseTime += deltaTime;
			ringAnimation->releaseTime = std::clamp(ringAnimation->releaseTime, 0.0f, 1.0f);
			chargeTime = Easing::EaseOutElastic(ringAnimation->tTemp, 0.0f, ringAnimation->releaseTime);
		}
		transform->scale = Vector3::UNIT_SCALE * (1.0f + ringAnimation->kChargeScale * chargeTime);
		sRingRadius = kRingInitialRadius + chargeTime;
	}
	if (!wasPress_ && isPress_)
	{
		for (auto entity : ballView)
		{
			auto* ballState = registry.GetComponent<BallStateComponent>(entity);
			auto* ballPhysics = registry.GetComponent<PhysicsComponent>(entity);
			if (ballState)
			{
				ballState->landed = false;
			}
			if (ballPhysics)
			{
				ballPhysics->useGravity = true;
			}
		}
	}
	for (auto entity : vausView)
	{
		auto* vausTransform = registry.GetComponent<No::TransformComponent>(entity);
		auto* vausState = registry.GetComponent<VausStateComponent>(entity);
		vausState->currentRingRadius = sRingRadius;
		vausState->chargePower = power;

		Vector3 ringPos{ sRingRadius * std::cos(angle), sRingRadius * std::sin(angle), -0.25f };
		vausTransform->translate = ringPos;
		vausTransform->rotation = MathCalculations::MakeRotateAxisAngleQuaternion(Vector3::FORWARD, angle + PI * 0.5f);
		vausTransform->scale = Vector3::UNIT_SCALE * (1.0f + 0.2f * chargeTime);
		vausState->theta = angle;

		if (wasPress_ && !isPress_)
		{
			vausState->hasReleasedMovement = true;
		}
		if (vausState->hasReleasedMovement)
		{
			vausState->releaseTime += deltaTime;
			vausState->releaseVelocityAccum +=
				(vausTransform->translate - vausState->prevPosition);
		}

		vausState->prevPosition = vausTransform->translate;

	}

	wasPress_ = isPress_;
}

float VausControlSystem::CalculateMouseAngle()
{

	POINT pt;
	if (GetCursorPos(&pt))
	{
		ScreenToClient(GraphicsCore::gWindowManager.GetMainWindow()->GetWindowHandle(), &pt);
	}

	//クライアント座標でウィンドウの中心を取得
	RECT rc;
	auto* main = GraphicsCore::gWindowManager.GetMainWindow();
	int cx = 0, cy = 0;
	if (main)
	{
		GetClientRect(main->GetWindowHandle(), &rc);
		cx = (rc.right - rc.left) / 2;
		cy = (rc.bottom - rc.top) / 2;
	}

	//画面空間で中心からマウスまでのベクトルを計算
	float mx = static_cast<float>(pt.x - cx);
	float my = static_cast<float>(-(pt.y - cy));

	return std::atan2f(my, mx);
}