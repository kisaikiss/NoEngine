#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// デバッグ用ショートカットキーを管理するシステム。
/// SetStopInPause(false) でポーズ中も動作する。
///
/// ショートカット一覧:
///   LeftShift + 1  : デバッグカメラ / レールカメラ切り替え (useDebugCamera)
///   LeftShift + 2  : デバッグ表示 全展開 / 全折り畳みトグル (debugDisplayAll)
///   LeftCtrl  + I  : 自機無敵トグル (debugInvincible)
///   LeftCtrl  + R  : クリア/オーバー無効トグル (debugDisableResult)
/// </summary>
class DebugShortcutSystem : public No::ISystem {
public:
    DebugShortcutSystem() { SetStopInPause(false); }
    void Update(No::Registry& registry, float deltaTime) override;
};
