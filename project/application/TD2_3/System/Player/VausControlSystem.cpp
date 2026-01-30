#include "VausControlSystem.h"
#include "../../tag.h"
#include "../../Component/PhysicsComponent.h"
#include "../../Component/RingAnimationComponent.h"
#include "../../Component/VausStateComponent.h"
#include "../../Component/BallStateComponent.h"
#include "../../Component/BackGroundComponent.h"

#include "engine/Math/Types/Calculations/QuaternionCalculations.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Math/Easing.h"
#include "engine/Functions/Input/Input.h"

using namespace NoEngine;

namespace
{
	constexpr float kRingInitialRadius = 4.85f;
	constexpr float kStickDeadZone = 0.2f;
	constexpr float kMouseMoveAngleThreshold = 0.05f;
	float sRingRadius = kRingInitialRadius;
}

static float NormalizeAngleLocal(float a)
{
	while (a > PI) a -= 2 * PI;
	while (a < -PI) a += 2 * PI;
	return a;
}

void VausControlSystem::Update(No::Registry& registry, float deltaTime)
{
	auto vausView = registry.View<
		VausTag,
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
	
	auto backGroundView = registry.View<BackGroundComponent>();

	// 現在のマウス角度（常に取得）
	float mouseAngle = CalculateMouseAngle();

	// スティックの情報を取得
	auto stick = Input::Pad::GetStick();
	float lmag2 = stick.leftStickX * stick.leftStickX + stick.leftStickY * stick.leftStickY;
	float rmag2 = stick.rightStickX * stick.rightStickX + stick.rightStickY * stick.rightStickY;
	float dead2 = kStickDeadZone * kStickDeadZone;
	bool stickActive = (lmag2 > dead2) || (rmag2 > dead2);
	float stickAngle = CalculateStickAngle();

	if (stickActive)
	{
		lastInput_ = LastInput::Stick;
	}
	else
	{
		// マウス移動を検出
		float d = std::fabs(NormalizeAngleLocal(mouseAngle - prevMouseAngle_));
		bool mouseMoved = d > kMouseMoveAngleThreshold;
		bool mousePressedNow = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		if (mouseMoved || mousePressedNow)
		{
			if (lastInput_ == LastInput::Stick)
			{
				isBlending_ = true;
				blendTime_ = 0.0f;
				blendFromAngle_ = currentAngle_;
			}
			lastInput_ = LastInput::Mouse;
		}
	}

	float angle = currentAngle_;

	if (lastInput_ == LastInput::Stick && stickActive)
	{
		currentAngle_ = stickAngle;
	}
	else if (lastInput_ == LastInput::Mouse)
	{
		float target = mouseAngle;

		if (isBlending_)
		{
			blendTime_ += deltaTime;
			float t = std::clamp(blendTime_ / blendDuration_, 0.0f, 1.0f);

			currentAngle_ = Easing::EaseOutCubic(
				blendFromAngle_,
				target,
				t
			);

			if (t >= 1.0f)
				isBlending_ = false;
		}
		else
		{
			currentAngle_ = target;
		}
	}


	currentAngle_ = NormalizeAngleLocal(currentAngle_);
	angle = currentAngle_;


	// ボタン/押下状態：マウス左ボタンまたはパッドの A ボタ ン
	isPress_ = ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) || Input::Pad::IsPress(Input::GamepadButton::A);

	
	for (auto entity : ringView)
	{
		auto* ringTrans = registry.GetComponent<No::TransformComponent>(entity);
		auto* ringAnimation = registry.GetComponent<RingAnimationComponent>(entity);
		if (isPress_)
		{
			if (!wasPress_)
			{
				No::SoundEffectPlay("ballPong2", 0.5f);
				power_ = 0.0f;
			}
			ringAnimation->releaseTime = 0.0f;
			ringAnimation->pressedTime += deltaTime * 2.0f;
			ringAnimation->pressedTime = std::clamp(ringAnimation->pressedTime, 0.0f, 1.0f);
			chargeTime_ = Easing::Lerp(0.0f, 1.0f, ringAnimation->pressedTime);
			ringAnimation->tTemp = chargeTime_;
			if (chargeTime_ >= 1.0f)
			{
				ringTrans->translate.x += static_cast<float>( rand() % 3 - 1) * deltaTime;
				ringTrans->translate.y += static_cast<float>(rand() % 3 - 1) * deltaTime;
			}
		}
		else
		{
			ringTrans->translate = Vector3::ZERO;
			if (wasPress_ && !isPress_)
			{
				power_ = ringAnimation->tTemp;
				//ヨシダ追加しました
				float pitch = 1.0f + ringAnimation->pressedTime * 0.5f;
				No::SetPitch("chargeEnter", pitch);
				No::SoundPlay("chargeEnter", 0.5f, false);
			}
			ringAnimation->pressedTime = 0.0f;
			ringAnimation->releaseTime += deltaTime;
			ringAnimation->releaseTime = std::clamp(ringAnimation->releaseTime, 0.0f, 1.0f);
			chargeTime_ = Easing::EaseOutElastic(ringAnimation->tTemp, 0.0f, ringAnimation->releaseTime);
		}
		ringTrans->scale = Vector3::UNIT_SCALE * (1.0f + ringAnimation->kChargeScale * chargeTime_);
		sRingRadius = kRingInitialRadius + chargeTime_;
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
		Vector3 posBeforeUpdate = vausTransform->translate;
		vausState->currentRingRadius = sRingRadius;
		vausState->chargePower = power_;
		vausState->theta = angle;
		Vector3 ringPos{ sRingRadius * std::cos(angle), sRingRadius * std::sin(angle), -0.25f };
		vausTransform->translate = ringPos;
		vausTransform->rotation = MathCalculations::MakeRotateAxisAngleQuaternion(Vector3::FORWARD, angle + PI * 0.5f);
		vausTransform->scale = Vector3::UNIT_SCALE * (1.0f + 0.2f * chargeTime_);
		vausState->isReleasing = false;

		if (wasPress_ && !isPress_)
		{
			vausState->isReleasing = true;
		}
		if (!isPress_ && chargeTime_ > 0.001f)
		{
			vausState->isReleasing = true;
		}

		if (deltaTime > 0.0f)
		{
			vausState->currentVelocity = (vausTransform->translate - vausState->prevPosition) / deltaTime;
		}
		else
		{
			vausState->currentVelocity = Vector3::ZERO;
		}

		vausState->prevPosition = vausTransform->translate;
	}
	for (auto entity : backGroundView)
	{
		auto* backGround = registry.GetComponent<BackGroundComponent>(entity);
		backGround->powerFactor = Easing::Lerp(0.1f, 0.3f, chargeTime_);

	}
	wasPress_ = isPress_;
	prevMouseAngle_ = mouseAngle;
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

float VausControlSystem::CalculateStickAngle()
{	
	auto stick = Input::Pad::GetStick();
	float lmag2 = stick.leftStickX * stick.leftStickX + stick.leftStickY * stick.leftStickY;
	float rmag2 = stick.rightStickX * stick.rightStickX + stick.rightStickY * stick.rightStickY;
	float dead2 = kStickDeadZone * kStickDeadZone;
	if (lmag2 > rmag2 && lmag2 > dead2) return std::atan2f(stick.leftStickY, stick.leftStickX);
	if (rmag2 > dead2) return std::atan2f(stick.rightStickY, stick.rightStickX);
	return 0.0f;
}
