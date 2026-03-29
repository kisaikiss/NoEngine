# NoEngine / CommentBout 計画書

> 作成日: 2026-03-29
> ブランチ: `Collision_Update`
> 目的: Phase 7〜10 の機能実装

---

## AI 行動規約

**Claude はこの作業において以下を厳守する：**

- `git push` は一切行わない（ローカル作業のみ）
- `git commit` はユーザーが明示的に指示した場合のみ行う
- リモートリポジトリへの操作（push / force-push / PR作成 など）は禁止
- 外部サービスへのデータ送信は行わない
- 破壊的操作（ファイル大量削除・ブランチ削除など）は事前確認する

---

## 方針

- Phase 単位で実装→ビルド→確認を繰り返す
- 各 Phase 完了時にチェックリストを埋めて記録する
- 実装前に「前提確認」がある場合は必ず grep で現状を確認してからコードを書く

---

## Phase 7 — DebugShortcutSystem

> IMPLEMENTATION_PLAN.md Phase 2 に対応。
> 目的: デバッグショートカットキーを専用システム・コンポーネントに集約し、
> PlayerControlSystem / GameResultSystem / GameScene に散在する入力処理を排除する。

---

### 設計概要

```
DebugShortcutStateComponent（ゲーム側コンポーネント、専用エンティティに配置）
    ↑ 書き込み
DebugShortcutSystem（SetStopInPause(false)）
    ← キー入力を読み、フラグを更新

EditorManager / PlayerControlSystem / GameResultSystem
    → DebugShortcutStateComponent を読み取るだけ
```

EditorManager は System ではないため、System 間の直接参照を避けるために
共有コンポーネント方式を採用する。

---

### 新規作成ファイル

#### `Component/Editor/DebugShortcutStateComponent.h`

```cpp
#pragma once

struct DebugShortcutStateComponent {
    bool useDebugCamera     = false;  // false=RailCamera, true=DebugCamera
    bool debugDisplayAll    = false;  // true=デバッグ表示強制全開, false=全閉
    bool debugInvincible    = false;  // 自機無敵
    bool debugDisableResult = false;  // クリア/オーバー判定を両方無効化
};
struct DebugShortcutStateTag {};
```

#### `System/Editor/DebugShortcutSystem.h`

```cpp
#pragma once
#include "engine/NoEngine.h"

class DebugShortcutSystem : public No::ISystem {
public:
    DebugShortcutSystem() { SetStopInPause(false); }
    void Update(No::ECSRegistry& registry, float deltaTime) override;
};
```

#### `System/Editor/DebugShortcutSystem.cpp`

担当ショートカット:

| キー | フラグ | 処理 |
|-----|--------|------|
| `LeftShift + 1` | `useDebugCamera` | トグル |
| `LeftShift + 2` | `debugDisplayAll` | トグル |
| `LeftCtrl + I` | `debugInvincible` | トグル |
| `LeftCtrl + R` | `debugDisableResult` | トグル |

> ※旧実装（左右Shift同時押し）は PlayerControlSystem / GameResultSystem から削除する。
> 重複を避けるため新ショートカットに変更。

---

### 変更ファイル一覧

#### `GameScene.cpp`

- Setup() で `DebugShortcutStateTag` エンティティを生成し `DebugShortcutStateComponent` を追加
- `DebugShortcutSystem` を `InputHelperSystem` の直後に登録
- `CameraImGui()` のカメラ切り替えロジックを削除（ImGui ウィンドウ自体は残してよい）

#### `System/Player/PlayerControlSystem.cpp`

- 無敵トグルの入力処理（左右 Shift 同時押し部分）を削除
- `debugInvincible` の参照先を `PlayerComponent` → `DebugShortcutStateComponent` に変更

#### `System/OutGame/GameResultSystem.cpp`

- `DebugToggleClearOver()` 関数を削除
- `debugDisableOver` / `debugDisableClear` の2変数を廃止
- 代わりに `DebugShortcutStateComponent::debugDisableResult` を参照（1フラグでOver/Clear両方を無効化）

#### `Editor/EditorManager.cpp`

CollapsingHeader("デバッグ表示") 内の処理を以下の3セクションに整理:

```
1. スポーンデバッグ（既存チェックボックス群）
2. レールカメラ描画チェックボックス群（既存）
3. コリジョンデバッグ設定（CollisionDebugRenderSystem から移植）
   ← CollisionDebugRenderSystem::DrawConfigImGui() の中身をここに移す
```

`debugDisplayAll` の参照:
```cpp
// 毎フレーム、DebugShortcutStateComponent を取得して開閉を制御
bool forceOpen  = shortcut && shortcut->debugDisplayAll;
bool forceClose = shortcut && !shortcut->debugDisplayAll;
ImGui::SetNextItemOpen(forceOpen,  ImGuiCond_Always);  // true なら強制展開
// false 時は SetNextItemOpen(false) で強制折り畳み
```

`useDebugCamera` の参照:
```cpp
// カメラ切り替えロジックを CameraImGui() から移植
// activeCameraEntity_ を debugCameraEntity_ ↔ railCameraEntity_ で切り替え
```

#### `Collision/System/CollisionDebugRenderSystem.cpp`

- `DrawConfigImGui()` の呼び出しを `Update()` から削除
- 描画ロジック本体（DrawEnemyColliderDebug 等）はそのまま維持

---

### チェックリスト

- [x] `Component/Editor/DebugShortcutStateComponent.h` 作成
- [x] `System/Editor/DebugShortcutSystem.h/.cpp` 作成
- [x] `GameScene.cpp`: DebugShortcutStateTag エンティティ生成 + DebugShortcutSystem 登録
- [x] `PlayerControlSystem.cpp`: 無敵入力削除、DebugShortcutStateComponent 参照に変更
- [x] `GameResultSystem.cpp`: DebugToggleClearOver() 削除、debugDisableResult 参照に変更
- [x] `EditorManager.cpp`: コリジョンデバッグUIをデバッグ表示セクションに移植
- [x] `EditorManager.cpp`: debugDisplayAll でヘッダー強制開閉、useDebugCamera でカメラ切替
- [x] `CollisionDebugRenderSystem.cpp`: DrawConfigImGui() 呼び出し削除
- [x] vcxproj / vcxproj.filters 更新（新規ファイル追加）
- [ ] **ビルド確認**
- [ ] 動作確認: 4種ショートカットが正常動作すること
- [ ] 動作確認: LeftShift+2 でデバッグ表示が一括開閉されること

---

## Phase 8 — 敵弾パラメータ拡張

> IMPLEMENTATION_PLAN.md Phase 3 に対応（Phase 6-E/F の未実装部分を含む）。
> 目的: 敵弾のモデルスケール・コライダーサイズ・コライダーオフセットを
> ImGui/JSON で編集・保存できるようにする。

---

### 前提確認（実装前に必ず grep で確認してからコードを書く）

以下の項目は過去の設計で実装済みの可能性がある。grep で存在を確認し、
**済みならスキップ、なければ追加する**。

| 確認箇所 | 確認コマンド例 |
|---------|--------------|
| `CollisionDebugConfigComponent.h` に `showEnemyBulletCollider` があるか | grep showEnemyBulletCollider |
| `CollisionDebugRenderSystem.cpp` に `DrawEnemyBulletColliderDebug` があるか | grep DrawEnemyBulletColliderDebug |
| `EnemyShooterComponent.h` に `bulletColliderRadiusMultiplier` があるか | grep bulletColliderRadiusMultiplier |
| `EnemySpawnSystem.cpp` で shooter に bulletCollider をコピーしているか | grep bulletCollider EnemySpawnSystem.cpp |

---

### Step 8-A: 敵弾コライダーデバッグ表示

**変更対象:**
- `Collision/Component/CollisionDebugConfigComponent.h`
- `Collision/System/CollisionDebugRenderSystem.h`
- `Collision/System/CollisionDebugRenderSystem.cpp`

```cpp
// CollisionDebugConfigComponent.h に追加（未実装なら）
bool showEnemyBulletCollider = true;
```

```cpp
// CollisionDebugRenderSystem.h に追加（未実装なら）
void DrawEnemyBulletColliderDebug(No::ECSRegistry& registry);
```

```cpp
// DrawEnemyBulletColliderDebug() の実装（DrawEnemyColliderDebug と同構造）
// CBEnemyBulletTag + Collider3DComponent を View して DrawSphere で描画
// 衝突中: 赤 (1,0.2,0.2,1), 衝突なし: 緑 (0.2,1,0.2,1)
```

チェックボックスUIは Phase 7 で EditorManager に統合済みのため、ここでは描画実装のみ。

---

### Step 8-B: 敵弾コライダーサイズ/オフセット

**変更対象:**
- `Component/Enemy/EnemyShooterComponent.h`
- `System/Enemy/EnemySpawnSystem.cpp`
- `System/Enemy/EnemyShootSystem.cpp`
- `Editor/EditorManager.cpp`

```cpp
// EnemyShooterComponent.h に追加（未実装なら）
float       bulletColliderRadiusMultiplier = 0.5f;
No::Vector3 bulletColliderLocalOffset      = { 0.0f, 0.0f, 0.0f };
```

```cpp
// EnemySpawnSystem.cpp の SpawnRailEnemies() に追加（未実装なら）
shooter->bulletColliderRadiusMultiplier = preset.bulletCollider.radiusMultiplier;
shooter->bulletColliderLocalOffset      = preset.bulletCollider.localOffset3D;
```

```cpp
// EnemyShootSystem.cpp の SpawnEnemyBullet() を変更（未実装なら）
// 引数追加: float bulletRadiusMultiplier, const No::Vector3& bulletLocalOffset
collider->radiusMultiplier = bulletRadiusMultiplier;
collider->localOffset      = bulletLocalOffset;
// 呼び出し元2箇所（通常敵・ボス）に shooter-> の値を渡す
```

```cpp
// EditorManager::DrawEnemyTab() の drawType ラムダ内に追加
ImGui::SeparatorText("弾コライダー");
changed |= ImGui::DragFloat ("弾コライダー半径倍率",   &cfg.bulletCollider.radiusMultiplier, 0.01f, 0.01f, 5.0f);
changed |= ImGui::DragFloat3("弾コライダーオフセット", &cfg.bulletCollider.localOffset3D.x,  0.01f, -5.0f, 5.0f);
```

`ApplyEnemyPresetsToAliveEnemies()` の shooter ブロックにも同じ反映処理を追加。

---

### Step 8-C: 弾モデルスケール（新規追加）

現状: `EnemyShootSystem::SpawnEnemyBullet()` 内で `t->scale = { 0.25f, 0.25f, 0.25f };` がハードコード。

**変更対象:**
- `Data/EnemyConfig.h`
- `Data/EnemyDataIO.cpp`
- `Component/Enemy/EnemyShooterComponent.h`
- `System/Enemy/EnemySpawnSystem.cpp`
- `System/Enemy/EnemyShootSystem.cpp`
- `Editor/EditorManager.cpp`

```cpp
// EnemyConfig.h に追加
float bulletModelScale = 0.25f;  // 既存ハードコード値に合わせた初期値
```

```cpp
// EnemyDataIO.cpp の Save に追加
j["bulletModelScale"] = cfg.bulletModelScale;
// Load に追加
if (j.contains("bulletModelScale")) cfg.bulletModelScale = j["bulletModelScale"].get<float>();
```

```cpp
// EnemyShooterComponent.h に追加
float bulletModelScale = 0.25f;
```

```cpp
// EnemySpawnSystem.cpp の SpawnRailEnemies() に追加
shooter->bulletModelScale = preset.bulletModelScale;
```

```cpp
// EnemyShootSystem.cpp の SpawnEnemyBullet() を変更
float s = shooter->bulletModelScale;
t->scale = { s, s, s };  // 旧: { 0.25f, 0.25f, 0.25f }
```

```cpp
// EditorManager::DrawEnemyTab() の drawType ラムダ内に追加
ImGui::SeparatorText("弾モデル");
changed |= ImGui::DragFloat("弾モデルスケール", &cfg.bulletModelScale, 0.001f, 0.01f, 2.0f);
```

`ApplyEnemyPresetsToAliveEnemies()` の shooter ブロックにも `bulletModelScale` の反映を追加。

> **UI表示方針:** 弾設定UIは全 EnemyType に表示。
> 射撃しない敵は shooter が存在しないため実際には参照されず無害。

---

### チェックリスト

**Step 8-A:**
- [x] `showEnemyBulletCollider` 存在確認・追加（実装済み）
- [x] `DrawEnemyBulletColliderDebug()` 存在確認・追加（実装済み）
- [x] `Update()` からの呼び出し確認（実装済み）

**Step 8-B:**
- [x] EnemyShooterComponent 2フィールド確認・追加（実装済み）
- [x] EnemySpawnSystem コピー処理確認・追加（実装済み）
- [x] EnemyShootSystem 引数追加・コライダー反映（実装済み）
- [x] 呼び出し元2箇所（通常敵・ボス）更新（実装済み）
- [x] EditorManager 弾コライダーUI追加（実装済み）
- [x] ApplyEnemyPresetsToAliveEnemies 更新（実装済み）

**Step 8-C:**
- [x] EnemyConfig に `bulletModelScale` 追加
- [x] EnemyDataIO Save/Load に追加
- [x] EnemyShooterComponent に `bulletModelScale` 追加
- [x] EnemySpawnSystem でコピー
- [x] EnemyShootSystem で使用（ハードコード 0.25f を削除）
- [x] EditorManager UI追加（弾モデルスケール DragFloat）
- [x] ApplyEnemyPresetsToAliveEnemies 更新

- [x] **ビルド確認**
- [ ] JSON保存 → 再起動後に値が保持されること確認
- [ ] Phase 8-A のデバッグ表示で弾コライダーサイズが変化することを確認

---

## Phase 9 — 吹き出し停止演出

> 目的: 敵を倒したときの吹き出しが、出現地点でバウンスアニメーション後に
> 一定時間停止してから、ボスHPバーへ移動する。

---

### フェーズ設計

```
[現在]
スポーン → ベジェ移動開始 → 到達でボスダメージ

[変更後]
スポーン（縮小スケール）
  → Appearing: バウンスイージングで最大サイズまで拡大
  → Stopping:  最大サイズのまま停止（stopDuration 秒）
  → Moving:    ベジェ曲線でボスHPバーへ移動 → 到達でボスダメージ
```

---

### データ拡張: `Data/SpeechBubbleConfig.h`

`SpeechBubbleSizeConfig` に追加:
```cpp
float appearDuration = 0.35f;  // 出現バウンスのイージング時間（大中小別）
float stopDuration   = 0.60f;  // 出現後の停止時間（大中小別）
// 既存の spriteSize はそのまま「最大サイズ」として扱う
```

`SpeechBubbleConfig` に追加:
```cpp
float initialScalePercent = 0.3f;  // 出現開始時スケール比率（大中小共通）
```

`to_json` / `from_json` に追記（JSON serialization）。

---

### コンポーネント拡張: `Component/Enemy/SpeechBubbleComponent.h`

```cpp
enum class SpeechBubblePhase { Appearing, Stopping, Moving };

// 追加フィールド
SpeechBubblePhase phase = SpeechBubblePhase::Appearing;
float appearElapsed = 0.f;
float stopElapsed   = 0.f;
// 既存の elapsed は Moving フェーズ専用として継続使用
```

---

### スポーン変更: `System/Enemy/EnemyVisualSystem.cpp`

吹き出しエンティティ生成時のスケール初期値を変更:
```cpp
// 変更前
t2d->scale = sizeCfg->spriteSize;

// 変更後
t2d->scale = sizeCfg->spriteSize * cfg.initialScalePercent;
// （cfg は GameResourceComponent 経由で取得した SpeechBubbleConfig）
```

`comp->phase` は `Appearing` のまま（デフォルト値）で OK。

---

### システム更新: `System/Enemy/SpeechBubbleToBossSystem.cpp`

**バウンスイージング関数（ファイル内に static 定義）:**
```cpp
static float EaseBounceOut(float t) {
    constexpr float n1 = 7.5625f, d1 = 2.75f;
    if      (t < 1.0f / d1) { return n1 * t * t; }
    else if (t < 2.0f / d1) { t -= 1.500f / d1; return n1 * t * t + 0.7500f; }
    else if (t < 2.5f / d1) { t -= 2.250f / d1; return n1 * t * t + 0.9375f; }
    else                    { t -= 2.625f / d1; return n1 * t * t + 0.984375f; }
}
```

**Update() のフェーズ分岐:**

```
SpeechBubbleConfig を GameResourceComponent から取得
sizeCategory で sizeLarge/sizeMedium/sizeSmall を選択

----- Appearing フェーズ -----
comp->appearElapsed += dt;
float rate = comp->appearElapsed / sizeCfg->appearDuration;
rate = clamp(rate, 0, 1);
float easedRate = EaseBounceOut(rate);
No::Vector2 minScale = cfg->initialScalePercent * sizeCfg->spriteSize;
No::Vector2 maxScale = sizeCfg->spriteSize;
t2d->scale = Lerp(minScale, maxScale, easedRate);
if (rate >= 1.0f) {
    t2d->scale = maxScale;
    comp->phase = SpeechBubblePhase::Stopping;
}

----- Stopping フェーズ -----
comp->stopElapsed += dt;
if (comp->stopElapsed >= sizeCfg->stopDuration) {
    comp->phase = SpeechBubblePhase::Moving;
}

----- Moving フェーズ -----
// 既存のベジェ曲線処理（comp->elapsed / comp->duration）
// rate >= 1.0 → DamageRequest 送信 → エンティティ削除（既存のまま）
```

`No::Vector2 Lerp(a, b, t)` はファイル内または Utility に定義。

---

### EditorManager — 吹き出し設定タブUI追加

```cpp
// SpeechBubbleConfig 全体の設定
ImGui::DragFloat("出現開始スケール比率（共通）", &cfg.initialScalePercent, 0.01f, 0.05f, 1.0f);

// 各サイズ設定（Large/Medium/Small）内に追加
ImGui::DragFloat("出現イージング時間", &sizeCfg.appearDuration, 0.01f, 0.05f, 3.0f);
ImGui::DragFloat("停止時間",          &sizeCfg.stopDuration,   0.01f, 0.0f,  5.0f);
// 既存の spriteSize（最大サイズ）は継続表示
```

---

### チェックリスト

- [x] `SpeechBubbleSizeConfig` に `appearDuration` / `stopDuration` 追加
- [x] `SpeechBubbleConfig` に `initialScalePercent` 追加
- [x] `SpeechBubbleSizeConfig` / `SpeechBubbleConfig` の `to_json` / `from_json` 更新
- [x] `SpeechBubbleComponent.h` に `SpeechBubblePhase` enum + 3フィールド追加
- [x] `EnemyVisualSystem.cpp`: 初期スケールを `initialScalePercent * spriteSize` に変更
- [x] `SpeechBubbleToBossSystem.cpp`: `EaseBounceOut` 追加 + 3フェーズ処理に更新
- [x] `EditorManager.cpp`: 吹き出しタブに `appearDuration / stopDuration / initialScalePercent` UI追加
- [x] **ビルド確認**
- [ ] 動作確認: バウンス出現 → 停止 → 移動 → ダメージの流れを確認
- [ ] 動作確認: ImGui で `stopDuration=0` にするとすぐ移動開始することを確認

---

## Phase 10 — オーバー/クリア演出段階化

> 目的: HP0 / ボス撃破時にカットシーン専用ポーズを導入する。
> 演出中はポーズメニューを開けないようにする。

---

### フェーズ設計

```
[オーバー]
HP=0
  → GameResultSystem: PlayerAnimState = Dead にセット
  → PlayerDeathSystem（SetStopInPause=false）が検知
      → inCutscene=true, isPause=true でゲームをポーズ
      → 自機スプライトを画面スペースで毎フレーム Y+ 方向に移動（落下）
      → transform2D.y > screenHeight + offscreenOffset になったら
          → ClearOverStateComponent を Over で起動

[クリア]
ボス死亡
  → GameResultSystem: BossDefeatSequenceComponent を起動（既存）
  → BossDefeatSystem（SetStopInPause=false）の演出開始直後
      → inCutscene=true, isPause=true でゲームをポーズ
      → 演出終了後 ClearOverStateComponent を Clear で起動（既存フロー）
```

---

### inCutscene とポーズの関係

```
isPause=true  → SetStopInPause(false) を持たないシステムが全て停止
               （PlayerControlSystem, EnemySystem 等）
inCutscene=true → PauseSystem がポーズ入力を無視する
               （ポーズメニューが開かない）

SetStopInPause(false) が必要なシステム（演出中も動き続けるべきもの）:
  - InputHelperSystem（既存）
  - CollisionDebugRenderSystem（既存）
  - ClearOverSystem（確認・追加）
  - ClearOverViewSystem（確認・追加）
  - BossDefeatSystem（追加）
  - PlayerDeathSystem（新規、最初から設定）
```

---

### Step 10-A: `inCutscene` フラグ追加

**変更対象:** `Component/GameResourceComponent.h`

```cpp
bool inCutscene = false;  // カットシーン中フラグ（ポーズメニュー禁止）
```

---

### Step 10-B: PauseSystem の変更

**変更対象:** `System/OutGame/PauseSystem.cpp`

ポーズ入力処理（`IsPauseTrigger()` を呼んでいる箇所）の先頭に追加:
```cpp
// カットシーン中はポーズメニューを開かない
auto* res = /* GameResourceComponent を取得 */;
if (res && res->inCutscene) return;
```

---

### Step 10-C: `PlayerAnimState::Dead` 追加

**変更対象:** `Component/Player/PlayerAnimStateComponent.h`

```cpp
// 変更前
enum class PlayerAnimState { Idle, Move, Attack };

// 変更後
enum class PlayerAnimState { Idle, Move, Attack, Dead };
```

**変更対象:** `System/Player/PlayerAnimSystem.cpp`

- `Dead` ステートに入ったら他ステートへ遷移しないようにガードを追加
- Dead 中のアニメーション: 最終フレームで停止（または最後のポーズを維持）でよい

---

### Step 10-D: GameResultSystem の変更

**変更対象:** `System/OutGame/GameResultSystem.cpp`

**Over 判定部分を変更:**
```cpp
// 変更前: HP≤0 → ClearOverStateComponent を Over で起動
// 変更後: HP≤0 → PlayerAnimState = Dead をセット
//         （ClearOverState の起動は PlayerDeathSystem が担当）
auto* animState = registry.GetComponent<PlayerAnimStateComponent>(playerEntity);
if (animState) animState->state = PlayerAnimState::Dead;
```

**Clear 判定部分:** 変更なし（BossDefeatSequenceComponent 起動はそのまま）

**debugDisableResult の対応:**
- 旧: `debugDisableOver` / `debugDisableClear` の2変数
- 新: Phase 7 の `DebugShortcutStateComponent::debugDisableResult` を参照（1フラグで両方無効化）

---

### Step 10-E: BossDefeatSystem の変更

**変更対象:** `System/Enemy/BossDefeatSystem.cpp` / `.h`

```cpp
// コンストラクタに追加
BossDefeatSystem() { SetStopInPause(false); }
```

演出開始時（BossDefeatSequenceComponent を初めて検出した直後）に追加:
```cpp
auto* res   = /* GameResourceComponent を取得 */;
auto* pause = /* PauseComponent を取得 */;
if (res && pause) {
    res->inCutscene  = true;
    pause->isPause   = true;
}
```

---

### Step 10-F: PlayerDeathSystem 新規作成

**新規:** `System/Player/PlayerDeathSystem.h`

```cpp
#pragma once
#include "engine/NoEngine.h"

class PlayerDeathSystem : public No::ISystem {
public:
    PlayerDeathSystem() { SetStopInPause(false); }
    void Update(No::ECSRegistry& registry, float deltaTime) override;
private:
    bool cutsceneStarted_ = false;
};
```

**新規:** `System/Player/PlayerDeathSystem.cpp`

```
Update() の処理:

1. PlayerAnimStateComponent::Dead を持つプレイヤーエンティティを検索
   → 存在しなければ早期 return

2. cutsceneStarted_ が false なら:
   → GameResourceComponent::inCutscene = true
   → PauseComponent::isPause = true
   → cutsceneStarted_ = true

3. cutsceneStarted_ が true なら:
   → プレイヤーの Transform2DComponent::translate.y += fallSpeed * deltaTime
   → translate.y > (screenHeight + offscreenOffset) なら:
       → ClearOverStateComponent を Over で起動
       → cutsceneStarted_ をリセット（シーン再利用時のため）
```

---

### Step 10-G: PlayerDeathConfig 新規作成

**新規:** `Data/PlayerDeathConfig.h`

```cpp
#pragma once
#include "externals/nlohmann/json.hpp"

struct PlayerDeathConfig {
    float fallSpeed       = 200.f;  // スクリーン座標/秒
    float offscreenOffset = 100.f;  // 画面高さに加算するオフセット（スクリーン座標）
};

inline void to_json(nlohmann::json& j, const PlayerDeathConfig& c) {
    j["fallSpeed"]       = c.fallSpeed;
    j["offscreenOffset"] = c.offscreenOffset;
}
inline void from_json(const nlohmann::json& j, PlayerDeathConfig& c) {
    if (j.contains("fallSpeed"))       c.fallSpeed       = j["fallSpeed"].get<float>();
    if (j.contains("offscreenOffset")) c.offscreenOffset = j["offscreenOffset"].get<float>();
}
```

保存先: `resources/game/td_3105/Data/Config/PlayerDeathConfig.json`

ImGui 編集は `GameScene::NotSystemUpdate()` に追加:
```cpp
void GameScene::PlayerDeathConfigImGui() {
    // PlayerDeathConfig の fallSpeed / offscreenOffset を DragFloat で編集
    // Save/Load ボタンで JSON に保存・読込
}
```

---

### Step 10-H: ClearOverSystem / ClearOverViewSystem の確認

**確認:** 以下が `SetStopInPause(false)` を持っているか grep で確認し、なければ追加。

| システム | 理由 |
|---------|------|
| `ClearOverSystem` | inCutscene=true（isPause=true）でも動作が必要 |
| `ClearOverViewSystem` | UI描画を継続するため |

---

### GameScene.cpp の変更まとめ

- Setup() に `PlayerDeathSystem` を登録（`BossDefeatSystem` の前後のどちらかに追加）
- Setup() で `PlayerDeathConfig` の JSON 読み込みを追加
- `NotSystemUpdate()` に `PlayerDeathConfigImGui()` を追加

---

### チェックリスト

**準備:**
- [ ] `ClearOverSystem` / `ClearOverViewSystem` の SetStopInPause(false) 確認・追加

**Step 10-A〜C:**
- [ ] `GameResourceComponent.h` に `inCutscene` 追加
- [ ] `PauseSystem.cpp`: inCutscene 中はポーズ入力を無視
- [ ] `PlayerAnimStateComponent.h`: `Dead` ステート追加
- [ ] `PlayerAnimSystem.cpp`: Dead ステートのハンドリング追加

**Step 10-D〜E:**
- [ ] `GameResultSystem.cpp`: Over 判定を `PlayerAnimState=Dead` セットに変更
- [ ] `GameResultSystem.cpp`: debugDisableResult（Phase 7）対応
- [ ] `BossDefeatSystem.cpp`: `SetStopInPause(false)` 追加 + inCutscene/isPause セット

**Step 10-F〜G:**
- [ ] `System/Player/PlayerDeathSystem.h/.cpp` 新規作成
- [ ] `Data/PlayerDeathConfig.h` 新規作成（struct + JSON シリアライズ）
- [ ] `GameScene.cpp`: PlayerDeathSystem 登録 + PlayerDeathConfig 読み込み追加
- [ ] `GameScene.cpp`: PlayerDeathConfigImGui() 追加

**vcxproj:**
- [ ] vcxproj / vcxproj.filters 更新（PlayerDeathSystem.h/.cpp、PlayerDeathConfig.h）

**ビルド・動作確認:**
- [ ] **ビルド確認**
- [ ] 動作確認（Over）: HP0 → 落下アニメーション → 画面外 → オーバー画面表示
- [ ] 動作確認（Clear）: ボス死亡 → 演出中にポーズメニューが開かない → クリア画面表示
- [ ] 動作確認: TAB キーを演出中に押してもポーズメニューが開かないこと
- [ ] 動作確認: クリア/オーバー後のリスタートで cutsceneStarted_ がリセットされること
