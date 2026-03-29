#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// デバッグ用ショートカットキーを管理するシステム。
/// SetStopInPause(false) でポーズ中も動作する。
///
/// ショートカット一覧:
///   LeftShift + 1  : デバッグカメラ / レールカメラ切り替え (useDebugCamera)
///   LeftShift + 2  : デバッグ表示項目の全ON / 全OFFを一括適用 (debugDisplayAll)
///   LeftCtrl  + 3  : 自機無敵トグル (debugInvincible)
///   LeftCtrl  + 4  : クリア/オーバー無効トグル (debugDisableResult)
/// </summary>
class DebugShortcutSystem : public No::ISystem {
public:
    DebugShortcutSystem() { SetStopInPause(false); }
    void Update(No::Registry& registry, float deltaTime) override;
};
