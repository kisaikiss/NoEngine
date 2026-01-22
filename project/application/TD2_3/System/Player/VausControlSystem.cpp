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
	bool isPress = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	static bool wasPress = false;
	static float chargeTime = 0.0f;
	static float chargeTemp = 0.0f;
	for (auto entity : ringView)
	{
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* ringAnimation = registry.GetComponent<RingAnimationComponent>(entity);
		if (isPress)
		{
			ringAnimation->releaseTime = 0.0f;
			ringAnimation->pressedTime += deltaTime * 2.0f;
			ringAnimation->pressedTime = std::clamp(ringAnimation->pressedTime, 0.0f, 1.0f);
			chargeTime = chargeTemp = Easing::Lerp(0.0f, 1.0f, ringAnimation->pressedTime);
			ringAnimation->tTemp = chargeTime;
		}
		else
		{
			ringAnimation->pressedTime = 0.0f;
			ringAnimation->releaseTime += deltaTime;
			ringAnimation->releaseTime = std::clamp(ringAnimation->releaseTime, 0.0f, 1.0f);
			chargeTime = Easing::EaseOutElastic(ringAnimation->tTemp, 0.0f, ringAnimation->releaseTime);
		}
		transform->scale = Vector3::UNIT_SCALE * (1.0f + ringAnimation->kChargeScale * chargeTime);
		sRingRadius = kRingInitialRadius + chargeTime;
	}
	if (!wasPress && isPress)
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
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* state = registry.GetComponent<VausStateComponent>(entity);
		state->currentRingRadius = sRingRadius;
		state->chargePower = chargeTime;
		Vector3 ringPos{ sRingRadius * std::cos(angle), sRingRadius * std::sin(angle), -0.25f };
		transform->translate = ringPos;
		transform->rotation = MathCalculations::MakeRotateAxisAngleQuaternion(Vector3::FORWARD, angle + PI * 0.5f);
		transform->scale = Vector3::UNIT_SCALE * (1.0f + 0.2f * chargeTime);
		state->theta = angle;
	}

	wasPress = isPress;
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