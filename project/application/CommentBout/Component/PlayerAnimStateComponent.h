#pragma once

enum class PlayerAnimState { Idle, Move, Attack };

/// <summary>
/// プレイヤーのアニメーション状態を保持するコンポーネント。
/// PlayerAnimSystem が読み書きし、PlayerControlSystem が攻撃開始時に書き込む。
/// </summary>
struct PlayerAnimStateComponent {
    PlayerAnimState state     = PlayerAnimState::Idle;
    PlayerAnimState prevState = PlayerAnimState::Idle;
    bool  isAttacking      = false;   // 攻撃モーション再生中
    float attackAnimTimer  = 0.f;     // 攻撃アニメーション残り時間
};
