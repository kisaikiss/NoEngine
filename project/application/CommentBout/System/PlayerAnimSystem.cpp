#include "stdafx.h"
#include "PlayerAnimSystem.h"
#include "application/CommentBout/Component/PlayerAnimStateComponent.h"
#include "application/CommentBout/Component/InvincibleComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/DamageFlashComponent.h"
#include "application/CommentBout/Data/PlayerConfig.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/InputHelper.h"
#include "engine/Functions/ECS/Component/Animator2DComponent.h"
#include "engine/Functions/ECS/Component/SpriteComponent.h"
#include <algorithm>
#include <cmath>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

void PlayerAnimSystem::Update(No::Registry& registry, float deltaTime)
{
    const PlayerConfig* config = nullptr;
    for (auto e : registry.View<CBPlayerConfigTag, PlayerConfig>()) {
        config = registry.GetComponent<PlayerConfig>(e);
        break;
    }
    if (!config) {
        return;
    }

    auto playerView = registry.View<CBPlayerTag, PlayerAnimStateComponent,
        No::Animator2DComponent, No::SpriteComponent,
        InvincibleComponent, HealthComponent>();

    for (auto entity : playerView) {
        auto* animState  = registry.GetComponent<PlayerAnimStateComponent>(entity);
        auto* animator   = registry.GetComponent<No::Animator2DComponent>(entity);
        auto* sprite     = registry.GetComponent<No::SpriteComponent>(entity);
        auto* invincible = registry.GetComponent<InvincibleComponent>(entity);
        auto* health     = registry.GetComponent<HealthComponent>(entity);
        if (!animState || !animator || !sprite || !invincible || !health) {
            continue;
        }

        if (health->isDead) {
            continue;
        }

        // ── アニメーション状態の決定 ────────────────────────────────
        const No::Vector2 inputDir = InputHelper::GetMoveInput();
        const bool moving = (inputDir.x != 0.f || inputDir.y != 0.f);

        PlayerAnimState newState;
        if (animState->isAttacking) {
            newState = PlayerAnimState::Attack;
        } else if (moving) {
            newState = PlayerAnimState::Move;
        } else {
            newState = PlayerAnimState::Idle;
        }

        // flipX: 左移動でフリップ (x入力なし時は前の状態を維持)
        if (inputDir.x < -0.1f) {
            sprite->flipX = true;
        } else if (inputDir.x > 0.1f) {
            sprite->flipX = false;
        }

        // ── 状態変化時に Animator をリセット ────────────────────────
        if (newState != animState->state) {
            animState->prevState = animState->state;
            animState->state     = newState;
            animator->timer      = 0.f;
            sprite->uv.x         = 0.f;

            switch (newState) {
            case PlayerAnimState::Idle:
                animator->currentAnimation = 0;
                animator->framesNum        = static_cast<uint32_t>(std::max(1, config->animIdleFrameCount));
                animator->frameByFrameTime = config->animIdleFrameTime;
                break;
            case PlayerAnimState::Move:
                animator->currentAnimation = 1;
                animator->framesNum        = static_cast<uint32_t>(std::max(1, config->animMoveFrameCount));
                animator->frameByFrameTime = config->animMoveFrameTime;
                break;
            case PlayerAnimState::Attack:
                animator->currentAnimation = 2;
                animator->framesNum        = static_cast<uint32_t>(std::max(1, config->animAttackFrameCount));
                animator->frameByFrameTime = config->animAttackFrameTime;
                // 攻撃アニメーション長 = フレーム数 × 1コマ時間
                animState->attackAnimTimer = static_cast<float>(std::max(1, config->animAttackFrameCount))
                    * std::max(0.01f, config->animAttackFrameTime);
                break;
            }
        }

        // ── 攻撃タイマーのカウントダウン (state設定・初期化の後に行う) ──
        if (animState->isAttacking) {
            animState->attackAnimTimer -= deltaTime;
            if (animState->attackAnimTimer <= 0.f) {
                animState->isAttacking    = false;
                animState->attackAnimTimer = 0.f;
            }
        }

        // ── 無敵中の透明点滅 ────────────────────────────────────────
        // DamageFlashComponent がアクティブな間は干渉しない (赤色点滅を優先)
        const bool flashActive = [&]() -> bool {
            const auto* flash = registry.GetComponent<DamageFlashComponent>(entity);
            return flash && flash->timer > 0.f;
        }();

        if (!flashActive) {
            if (invincible->time > 0.f) {
                const float total = (invincible->duration > 0.f) ? invincible->duration : 0.35f;
                const float remainRatio = std::min(1.f, invincible->time / total);
                const float hz = config->invincibleBlinkBaseHz
                    + (1.f - remainRatio) * (config->invincibleBlinkMaxHz - config->invincibleBlinkBaseHz);
                const float t = 0.5f + 0.5f * std::sin(invincible->time * hz * 2.f * 3.14159265f);
                sprite->color.a = config->invincibleBlinkMinAlpha
                    + (1.f - config->invincibleBlinkMinAlpha) * t;
            } else {
                sprite->color.a = 1.f;
            }
        }
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
