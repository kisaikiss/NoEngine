#pragma once
#include "engine/NoEngine.h"
#include "application/CommentBout/Utility/CBSpriteLayer.h"
#include "application/CommentBout/Collision/Component/ColliderConfig.h"

/// <summary>
/// プレイヤーの設定データ構造体。
/// PlayerConfigComponent.h から移行・リファクタリングした新しい名前。
/// </summary>
struct PlayerConfig {
    // PlayerComponent
    float moveSpeed = 480.0f;
    float acceleration = 2400.0f;
    float deceleration = 3000.0f;
    float maxSpeed = 480.0f;
    float invincibleDurationDefault = 0.35f;

    // PlayerAttackComponent
    No::Vector2 attackSpawnOffset = { 0.0f, -80.0f };
    No::Vector2 attackSize = { 140.0f, 140.0f };
    float attackVisibleTime = 0.35f;
    int attackLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::Effect));
    int attackPower = 10;

    // Health / Invincible
    int playerMaxHp = 10;
    float invincibleDuration = 0.35f;

    // DamageFlash (player)
    float flashDuration = 0.16f;
    bool flashBlinkEnabled = true;
    float flashBlinkHz = 14.0f;
    float flashMinAlpha = 0.35f;
    float flashMaxAlpha = 0.75f;
    No::Vector3 flashColorRGB = { 1.0f, 0.35f, 0.35f };

    // PlayerHitboxComponent
    bool useCameraGateForPlayerHit = true;
    float cameraGateNear = 0.0f;
    float cameraGateDepth = 0.5f;
    float cameraGateHalfWidth = 0.6f;
    float cameraGateHalfHeight = 0.32f;

    // StartTransform2DComponent
    No::Vector2 startPosition = { 640.0f, 600.0f };
    No::Vector2 startScale = { 128.0f, 200.0f };
    float startRotation = 0.0f;

    // コライダー設定
    ColliderConfig playerCollider2D;  // 自機の2Dコライダー設定
    ColliderConfig attackCollider;    // 攻撃の2Dコライダー設定
};
