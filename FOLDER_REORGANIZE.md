# NoEngine / CommentBout フォルダ再整理計画書

> 作成日: 2026-03-29
> ブランチ: `Collision_Update`
> 目的: Phase 9 のフォルダ整理を見直し、実装可能な範囲で精査・実施する

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

| # | 方針 | 理由 |
|---|------|------|
| 1 | Enemy 系・Player 系は明確に分けられるものを分類する | ファイル名から判断可能 |
| 2 | UI 系（HpBar・RailProgressBar）はサブフォルダ不要 | 数が少なく分けても効果薄 |
| 3 | どこにも属さない汎用 System は `System/Common/` へ | RailCamera・Lifetime・InputHelper |
| 4 | ダメージ受け/当たり効果系は将来 `Collision/System/` へ移したいが、他機能と混在しているため今回は除外 | 後の独立 Phase で対応 |
| 5 | 1 ステップずつビルド確認しながら進める | include 漏れの早期検出 |

---

## 現在のフォルダ状態（本計画開始時点）

Phase 9 のうち以下は **実施済み** (ビルド確認済み):

```
Component/FieldObject/    ✅  FieldPlacementComponent.h
Component/OutGame/        ✅  ClearOver系 + GameResultComponent
Editor/                   ✅  FieldObjectEditor（FieldObjectから移動）
System/Editor/            ✅  RailCameraEditorSystem 他 3 本
System/FieldObject/       ✅  FieldEditorSystem
System/OutGame/           ✅  GameResult + ClearOver系（既存の Pause/Title 系と統合）
Spawner/OutGame/          ✅  全 3 スポナー
FieldObject/ ディレクトリ  ✅  削除済み
```

残っている課題（本計画の対象）:

```
System/ 直下にまだある:
  Enemy系  × 10  BossBehavior, BossDefeat, EnemyBulletHit, EnemyContactDamage,
                  EnemyMove, EnemyShoot, EnemySpawn, EnemySystem, EnemyVisual,
                  SpeechBubbleToBoss
  Player系 ×  3  PlayerAnim, PlayerAttackResolve, PlayerControl
  汎用     ×  3  RailCameraSystem, LifetimeSystem, InputHelperSystem
  除外対象 ×  4  DamageApply, DamageFlash, HpBarView, RailProgressBar（現状維持）

Component/ 直下にまだある:
  Enemy系  ×  8  EnemyBullet, EnemyComponent, EnemyRewardSource, EnemyShooter,
                  SpawnEnemyRequest, SpeechBubble, BossComponent, BossDefeatSequence
  Player系 ×  5  PlayerComponent, PlayerAnimState, PlayerAttackComponent,
                  PlayerHitbox, InvincibleComponent
  除外対象 ×  9  AttackDamage, DamageFlash, DamageRequest, Health,
                  HpBar, RailProgressBar, RailCamera, GameResource, Lifetime（現状維持）
```

---

## Phase 1 — System/Enemy/ の作成

### 移動対象

| 移動前 (System/) | 移動後 |
|-----------------|--------|
| `BossBehaviorSystem.cpp, .h` | `System/Enemy/` |
| `BossDefeatSystem.cpp, .h` | `System/Enemy/` |
| `EnemyBulletHitSystem.cpp, .h` | `System/Enemy/` |
| `EnemyContactDamageSystem.cpp, .h` | `System/Enemy/` |
| `EnemyMoveSystem.cpp, .h` | `System/Enemy/` |
| `EnemyShootSystem.cpp, .h` | `System/Enemy/` |
| `EnemySpawnSystem.cpp, .h` | `System/Enemy/` |
| `EnemySystem.cpp, .h` | `System/Enemy/` |
| `EnemyVisualSystem.cpp, .h` | `System/Enemy/` |
| `SpeechBubbleToBossSystem.cpp, .h` | `System/Enemy/` |

### include パスを更新する必要があるファイル

- `Scene/GameScene.cpp`（上記 System を全て AddSystem している）

### チェックリスト

- [ ] `System/Enemy/` ディレクトリ作成、全 10 ペアのファイルを新パスに作成
- [ ] `GameScene.cpp` の include パスを `System/Enemy/Xxx` に更新
- [ ] `vcxproj` / `vcxproj.filters` 更新
- [ ] **ビルド確認**

---

## Phase 2 — System/Player/ の作成

### 移動対象

| 移動前 (System/) | 移動後 |
|-----------------|--------|
| `PlayerAnimSystem.cpp, .h` | `System/Player/` |
| `PlayerAttackResolveSystem.cpp, .h` | `System/Player/` |
| `PlayerControlSystem.cpp, .h` | `System/Player/` |

### include パスを更新する必要があるファイル

- `Scene/GameScene.cpp`

### チェックリスト

- [ ] `System/Player/` ディレクトリ作成、3 ペアを移動
- [ ] `GameScene.cpp` の include パスを更新
- [ ] `vcxproj` / `vcxproj.filters` 更新
- [ ] **ビルド確認**

---

## Phase 3 — System/Common/ の作成

「どこにも属さない汎用 System」をまとめる場所。

### 移動対象

| 移動前 (System/) | 移動後 | 理由 |
|-----------------|--------|------|
| `RailCameraSystem.cpp, .h` | `System/Common/` | レール専用だが Enemy でも Player でもない |
| `LifetimeSystem.cpp, .h` | `System/Common/` | 汎用ライフタイム管理 |
| `InputHelperSystem.cpp, .h` | `System/Common/` | 入力正規化ヘルパー（シーン共通） |

### include パスを更新する必要があるファイル

- `Scene/GameScene.cpp`
- `Scene/TitleScene.cpp`（InputHelperSystem を使用）

### チェックリスト

- [ ] `System/Common/` ディレクトリ作成、3 ペアを移動
- [ ] `GameScene.cpp` の include パスを更新
- [ ] `TitleScene.cpp` の include パスを更新
- [ ] `vcxproj` / `vcxproj.filters` 更新
- [ ] **ビルド確認**

---

## Phase 4 — Component/Enemy/ の作成

### 移動対象

| 移動前 (Component/) | 移動後 |
|--------------------|--------|
| `BossComponent.h` | `Component/Enemy/` |
| `BossDefeatSequenceComponent.h` | `Component/Enemy/` |
| `EnemyBulletComponent.h` | `Component/Enemy/` |
| `EnemyComponent.h` | `Component/Enemy/` |
| `EnemyRewardSourceComponent.h` | `Component/Enemy/` |
| `EnemyShooterComponent.h` | `Component/Enemy/` |
| `SpawnEnemyRequestComponent.h` | `Component/Enemy/` |
| `SpeechBubbleComponent.h` | `Component/Enemy/` |

### include パスを更新する必要があるファイル

Component/Enemy/ へ移動するファイルをインクルードしているファイル一覧:

- `Scene/GameScene.cpp`
- `Editor/EditorManager.cpp`
- `Editor/RailCameraEditor.cpp`
- `System/BossBehaviorSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/BossDefeatSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/DamageApplySystem.cpp`
- `System/Editor/EnemyConfigEditorSystem.cpp`
- `System/EnemyBulletHitSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/EnemyContactDamageSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/EnemyMoveSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/EnemyShootSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/EnemySpawnSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/EnemyVisualSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/OutGame/GameResultSystem.cpp`
- `System/PlayerAttackResolveSystem.cpp`（移動後は `System/Player/` 内）
- `System/RailCameraSystem.cpp`（移動後は `System/Common/` 内）
- `System/SpeechBubbleToBossSystem.cpp`（移動後は `System/Enemy/` 内）
- `Collision/System/CollisionDebugRenderSystem.cpp`（EnemyShooterComponent のみ）

### チェックリスト

- [ ] `Component/Enemy/` ディレクトリ作成、8 ヘッダを新パスに作成
- [ ] 上記 include ファイル全ての include パスを `Component/Enemy/Xxx` に更新
- [ ] `vcxproj` / `vcxproj.filters` 更新（ClInclude のみ、.h のみなので ClCompile は不要）
- [ ] **ビルド確認**

---

## Phase 5 — Component/Player/ の作成

### 移動対象

| 移動前 (Component/) | 移動後 |
|--------------------|--------|
| `PlayerComponent.h` | `Component/Player/` |
| `PlayerAnimStateComponent.h` | `Component/Player/` |
| `PlayerAttackComponent.h` | `Component/Player/` |
| `PlayerAttackComponent.cpp` | `Component/Player/` |
| `PlayerHitboxComponent.h` | `Component/Player/` |
| `InvincibleComponent.h` | `Component/Player/` |

### include パスを更新する必要があるファイル

- `Scene/GameScene.cpp`
- `Editor/EditorManager.cpp`
- `System/BossBehaviorSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/BossDefeatSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/DamageApplySystem.cpp`
- `System/DamageFlashSystem.cpp`
- `System/Editor/EnemyConfigEditorSystem.cpp`
- `System/Editor/PlayerInfoDebugSystem.cpp`
- `System/EnemyBulletHitSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/EnemyContactDamageSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/EnemyMoveSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/EnemyShootSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/EnemySpawnSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/EnemyVisualSystem.cpp`（移動後は `System/Enemy/` 内）
- `System/HpBarViewSystem.cpp`
- `System/OutGame/GameResultSystem.cpp`
- `System/PlayerAnimSystem.cpp`（移動後は `System/Player/` 内）
- `System/PlayerAttackResolveSystem.cpp`（移動後は `System/Player/` 内）
- `System/PlayerControlSystem.cpp`（移動後は `System/Player/` 内）
- `System/SpeechBubbleToBossSystem.cpp`（移動後は `System/Enemy/` 内）
- `Collision/System/CollisionDebugRenderSystem.cpp`

### チェックリスト

- [ ] `Component/Player/` ディレクトリ作成、5 ヘッダ + 1 cpp を新パスに作成
- [ ] 上記 include ファイル全ての include パスを `Component/Player/Xxx` に更新
- [ ] `vcxproj` / `vcxproj.filters` 更新（PlayerAttackComponent.cpp は ClCompile も更新）
- [ ] **ビルド確認**

---

## 現状維持（今回は動かさない）

### System/ 直下に残すもの

| ファイル | 理由 |
|---------|------|
| `DamageApplySystem` | 将来 Collision/System/ へ移したいが他機能と混在、要別 Phase |
| `DamageFlashSystem` | 同上 |
| `HpBarViewSystem` | UI 系、サブフォルダ不要 |
| `RailProgressBarSystem` | UI 系、サブフォルダ不要 |

### Component/ 直下に残すもの

| ファイル | 理由 |
|---------|------|
| `AttackDamageComponent.h` | 吹き出し攻撃（敵）にも使われる、ダメージ系で別 Phase |
| `DamageFlashComponent.h` | ダメージ系、別 Phase |
| `DamageRequestComponent.h` | ダメージ系、別 Phase |
| `HealthComponent.h` | Player / Enemy 両方が持つ共有コンポーネント |
| `HpBarComponent.h` | UI 系 |
| `RailProgressBarComponent.h` | UI 系 |
| `RailCameraComponent.h` | RailCameraSystem と対になるが単体で問題ない |
| `GameResourceComponent.h` | グローバルリソース |
| `LifetimeComponent.h` | 汎用 |

---

## 完了確認

- [ ] ビルドエラーなし、警告なし
- [ ] `System/Enemy/`、`System/Player/`、`System/Common/` が存在する
- [ ] `Component/Enemy/`、`Component/Player/` が存在する
- [ ] System/ 直下に Enemy/Player 系ファイルが残っていない
- [ ] Component/ 直下に Enemy/Player 系ファイルが残っていない（除外対象除く）
- [ ] `vcxproj` / `vcxproj.filters` が全移動に追従している
- [ ] ゲームが正常動作する

---

## Phase 6 — 保存/読込バグ修正・敵弾コライダー対応

> 対象ブランチ: `Collision_Update`
> フォルダ整理（Phase 1〜5）とは独立した機能バグ修正フェーズ。

---

### 根本原因（判明済み）

`EditorManager` の ImGui に `InputText` でステージ名を直接入力できる箇所があり、
**誤操作で意図しない文字が混入したまま保存/読込を実行すると存在しないパスにアクセスして無音で失敗**していた。

現状は `stageNameBuffer_`（ImGui 入力バッファ）が唯一のステージ名ソースであり、
「入力中の値 = ファイルアクセスに使う値」になってしまっている。

---

### Phase 6-A: ステージ名の二重変数化（根本修正）

**設計方針:**

| 変数 | 役割 | 変更タイミング |
|-----|------|--------------|
| `stageNameBuffer_[64]`（既存） | ImGui InputText の編集バッファ。表示・入力のみ | ユーザーがキー入力するたび |
| `activeStageName_[64]`（新規追加） | **ファイルアクセスに使う確定済み名前** | 「ステージ名確定」ボタン押下時のみ |

`LoadAll` / `SaveAll` / 各タブの個別 IO は全て `activeStageName_` を参照する。
`stageNameBuffer_` の値が壊れていても、確定ボタンを押すまでファイル操作には影響しない。

**UI レイアウト変更（`DrawImGui()` のトップ部分）:**

```
[CollapsingHeader "ステージ変更"  ← デフォルト閉じた状態]
  InputText("StageName", stageNameBuffer_, ...)
  Button("ステージ名確定")
    → activeStageName_ = stageNameBuffer_
    → rail->stageName  = activeStageName_

[常時表示]
  Text("使用中: %s", activeStageName_)
  Button("すべて読込")  Button("すべて保存")
  ... (ポーズ・デバッグ等は従来どおり)
```

**変更対象:**

| ファイル | 変更内容 |
|---------|---------|
| `Editor/EditorManager.h` | `char activeStageName_[64] = "";` メンバ追加 |
| `Editor/EditorManager.cpp` | `Initialize()` で両変数を初期化。`DrawImGui()` トップUIを変更。`LoadAll`/`SaveAll`/`DrawRailTab`/`DrawEventTab` が `activeStageName_` を参照するよう変更 |

**チェックリスト:**

- [x] `EditorManager.h` に `char activeStageName_[64] = "";` 追加
- [x] `EditorManager::Initialize()` で `activeStageName_` を `rail->stageName` で初期化
- [x] `DrawImGui()` のトップ UI を `CollapsingHeader("ステージ変更")` ＋ 確定ボタンに変更
- [x] 確定ボタン押下時: `activeStageName_` へコピー、`rail->stageName` 更新
- [x] 常時表示エリアに `Text("使用中: %s", activeStageName_)` ＋「すべて読込/保存」ボタン
- [x] `LoadAll()` / `SaveAll()` / `DrawRailTab()` / `DrawEventTab()` で `activeStageName_` を参照
- [ ] **ビルド確認**
- [ ] 実行確認: バッファに誤字を入れても「確定」しなければ読込/保存に影響しないことを確認

---

### Phase 6-B: ファイル構成変更とラッパー JSON 導入

**現在のパス構成:**

```
StageData/
  Stage_01/
    RailData/Stage_01_rail.json
    EventData/Stage_01_events.json
    FieldObjectData/Stage_01_field_objects.json
```

**新しいパス構成:**

```
StageData/
  Stage_01.json                        ← ラッパーJSON（新規）
  RailData/Stage_01_rail.json
  EventData/Stage_01_events.json
  FieldObjectData/Stage_01_field_objects.json
```

ステージが増えても `Stage_02.json`, `RailData/Stage_02_rail.json` ... の形で同じフォルダに並ぶ。

**ラッパー JSON `Stage_01.json` の構造:**

```json
{
  "stageName": "Stage_01",
  "railDataPath":        "resources/game/td_3105/Data/StageData/RailData/Stage_01_rail.json",
  "eventDataPath":       "resources/game/td_3105/Data/StageData/EventData/Stage_01_events.json",
  "fieldObjectDataPath": "resources/game/td_3105/Data/StageData/FieldObjectData/Stage_01_field_objects.json"
}
```

**パス生成関数の変更:**

| 関数 | 現在 | 新規 |
|-----|------|------|
| `MakeRailFilePath(name)` | `StageData/Stage_01/RailData/Stage_01_rail.json` | `StageData/RailData/Stage_01_rail.json` |
| `MakeEventFilePath(name)` | `StageData/Stage_01/EventData/Stage_01_events.json` | `StageData/EventData/Stage_01_events.json` |
| `MakeFieldPlacementPath(name)` | `StageData/Stage_01/FieldObjectData/...` | `StageData/FieldObjectData/Stage_01_field_objects.json` |
| `MakeStageWrapperPath(name)`（新規） | — | `StageData/Stage_01.json` |

**新規追加する関数（`RailDataIO.h/.cpp`）:**

```cpp
std::string MakeStageWrapperPath(const std::string& stageName);
bool SaveStageWrapper(const std::string& stageName);   // SaveAll 時に呼ぶ
bool LoadStageWrapper(const std::string& stageName);   // 確定ボタン押下時・存在確認に使う
```

`LoadStageWrapper` はステージの存在確認のみ行い、パスは `stageName` から導出する
（将来、ラッパーから個別パスをオーバーライドできる拡張余地を残す）。

**既存ファイルの移行（手動作業）:**

```
StageData/Stage_01/RailData/Stage_01_rail.json
  → StageData/RailData/Stage_01_rail.json

StageData/Stage_01/EventData/Stage_01_events.json
  → StageData/EventData/Stage_01_events.json

StageData/Stage_01/FieldObjectData/Stage_01_field_objects.json
  → StageData/FieldObjectData/Stage_01_field_objects.json

新規: StageData/Stage_01.json（初回 SaveAll で自動生成）
```

**チェックリスト:**

- [x] `RailDataIO.h` に `MakeStageWrapperPath` / `SaveStageWrapper` / `LoadStageWrapper` 宣言追加
- [x] `RailDataIO.cpp` の `MakeRailFilePath` を新パスに変更
- [x] `RailDataIO.cpp` の `MakeEventFilePath` を新パスに変更
- [x] `RailDataIO.cpp` に `MakeStageWrapperPath` / `SaveStageWrapper` / `LoadStageWrapper` を実装
- [x] `FieldObjectEditor.cpp` の `MakeFieldPlacementPath` を新パスに変更
- [x] `EditorManager.cpp` の確定ボタン処理に `LoadStageWrapper` 呼び出し追加
- [x] `EditorManager.cpp` の `SaveAll` に `SaveStageWrapper` 呼び出し追加
- [ ] エクスプローラーで既存ファイルを新フォルダに移動
- [ ] **ビルド確認**
- [ ] 実行確認: 「ステージ名確定 → すべて読込」でデータが正常に読めるか確認
- [ ] 実行確認: 「すべて保存」でラッパー JSON と各サブ JSON が正しい場所に生成されるか確認

---

### Phase 6-C: フィールドオブジェクト消滅問題の修正

**根本原因:** `LoadFieldPlacements()` の冒頭で `DestroyAllFieldObjects()` を実行した後、
ファイルが存在しない・読めない場合に `return` してしまう。
→ フィールドオブジェクトが消えたまま何も生成されない。

```
【修正前】
DestroyAllFieldObjects(registry);  ← 先に全削除
ifstream ifs(filePath);
if (!ifs) { return; }              ← 失敗 → 消えたまま

【修正後】
ifstream ifs(filePath);
if (!ifs) { return; }              ← 読めなければ何もしない
nlohmann::json json;
ifs >> json;
if (パース失敗) { return; }
DestroyAllFieldObjects(registry);  ← 読めた場合だけ削除して再生成
```

- [x] `LoadFieldPlacements()` の `DestroyAllFieldObjects()` をファイル・パース確認の後に移動
- [ ] **ビルド確認**
- [ ] 動作確認: 存在しないパスで読込してもオブジェクトが消えないことを確認
- [ ] 動作確認: 「すべて保存 → すべて読込」でフィールドオブジェクトが正常再表示されることを確認

---

### Phase 6-D: 保存先ディレクトリの自動作成

**問題:** `RailData/` / `EventData/` 等のフォルダが存在しない環境では
`std::ofstream` が無音で失敗する。

各 Save 関数の `std::ofstream` の直前に追加:
```cpp
std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
```

- [x] `RailDataIO.cpp` に `#include <filesystem>` 追加
- [x] `SaveRailToJson()` にディレクトリ作成処理追加
- [x] `SaveEventsToJson()` に同様追加
- [x] `SaveStageWrapper()` に同様追加
- [x] `FieldObjectEditor.cpp` の `SaveFieldPlacements()` に同様追加
- [ ] **ビルド確認**
- [ ] 実行確認: フォルダを削除した状態で「すべて保存」してもフォルダが自動生成されるか確認

---

### Phase 6-E: 敵弾コライダーのデバッグ表示追加

**現状:** `CollisionDebugRenderSystem` は敵本体のコライダーのみ表示。
敵弾（`CBEnemyBulletTag`）のコライダーは描画フラグも描画処理も未実装。

**変更対象:**

| ファイル | 変更内容 |
|---------|---------|
| `Collision/Component/CollisionDebugConfigComponent.h` | `bool showEnemyBulletCollider = true;` 追加 |
| `Collision/System/CollisionDebugRenderSystem.h` | `DrawEnemyBulletColliderDebug()` 宣言追加 |
| `Collision/System/CollisionDebugRenderSystem.cpp` | チェックボックス追加・実装追加・`Update()` 呼び出し |

`DrawEnemyBulletColliderDebug()` は `DrawEnemyColliderDebug()` と同構造で
`CBEnemyBulletTag` を View する。弾は Sphere なので `DrawSphere` で描画。

- [ ] `CollisionDebugConfigComponent.h` に `bool showEnemyBulletCollider = true;` 追加
- [ ] `CollisionDebugRenderSystem.h` に `DrawEnemyBulletColliderDebug()` 宣言追加
- [ ] `DrawConfigImGui()` の `enableCollisionDebug` ブロック内に `ImGui::Checkbox("敵弾コライダー", ...)` 追加
- [ ] `DrawEnemyBulletColliderDebug()` 実装追加（`CBEnemyBulletTag + Collider3DComponent` を View + `DrawSphere`）
- [ ] `Update()` から呼び出し追加
- [ ] **ビルド確認**
- [ ] 実行確認: チェックONで飛んでいる弾のコライダーが表示されるか確認

---

### Phase 6-F: 敵弾コライダーのサイズ/オフセット ImGui 編集 + JSON 保存対応

**現状の問題の整理:**

| 場所 | 現状 | 問題 |
|-----|------|-----|
| `EnemyConfig.bulletCollider` | 定義済み | — |
| `EnemyDataIO::Save/Load` | 読み書き実装済み | — |
| `EnemyShootSystem::SpawnEnemyBullet()` | `radiusMultiplier = 0.5f` ハードコード | `preset.bulletCollider` を参照していない |
| `EditorManager::DrawEnemyTab()` | `bulletCollider` の ImGui 編集なし | 値を変更できない |
| `ApplyEnemyPresetsToAliveEnemies()` | 弾コライダー反映なし | 次弾に反映されない |

**変更設計:**

① `EnemyShooterComponent.h` に弾コライダー設定値を追加:
```cpp
float       bulletColliderRadiusMultiplier = 0.5f;
No::Vector3 bulletColliderLocalOffset      = { 0.0f, 0.0f, 0.0f };
```

② `EnemySpawnSystem.cpp` の `SpawnRailEnemies()` で preset → shooter へコピー:
```cpp
shooter->bulletColliderRadiusMultiplier = preset.bulletCollider.radiusMultiplier;
shooter->bulletColliderLocalOffset      = preset.bulletCollider.localOffset3D;
```

③ `EnemyShootSystem.cpp` の `SpawnEnemyBullet()` に引数追加・コライダーに反映:
```cpp
// 引数追加: float bulletRadiusMultiplier, const No::Vector3& bulletLocalOffset
collider->radiusMultiplier = bulletRadiusMultiplier;
collider->localOffset      = bulletLocalOffset;
```
呼び出し元2箇所（通常敵・ボス）に `shooter->bulletCollider...` を渡す。

④ `EditorManager::DrawEnemyTab()` の `drawType` ラムダに追加:
```cpp
ImGui::SeparatorText("弾コライダー");
changed |= ImGui::DragFloat("弾コライダー半径倍率", &cfg.bulletCollider.radiusMultiplier, 0.01f, 0.01f, 5.0f);
changed |= ImGui::DragFloat3("弾コライダーオフセット", &cfg.bulletCollider.localOffset3D.x, 0.01f, -5.0f, 5.0f);
```

⑤ `ApplyEnemyPresetsToAliveEnemies()` の shooter ブロックに追加:
```cpp
shooter->bulletColliderRadiusMultiplier = cfg.bulletCollider.radiusMultiplier;
shooter->bulletColliderLocalOffset      = cfg.bulletCollider.localOffset3D;
```

**チェックリスト:**

- [ ] `EnemyShooterComponent.h` に `bulletColliderRadiusMultiplier` / `bulletColliderLocalOffset` 追加
- [ ] `EnemySpawnSystem.cpp` の `SpawnRailEnemies()` で shooter に bulletCollider をコピー
- [ ] `EnemyShootSystem.cpp` の `SpawnEnemyBullet()` 引数追加・コライダー反映
- [ ] 呼び出し元2箇所（通常敵・ボス）を更新
- [ ] `EditorManager::DrawEnemyTab()` の `drawType` ラムダに弾コライダー編集追加
- [ ] `ApplyEnemyPresetsToAliveEnemies()` の shooter ブロックに反映追加
- [ ] **ビルド確認**
- [ ] 実行確認: エディタで値変更 → JSON保存 → 再起動後に反映されるか確認
- [ ] Phase 6-E のデバッグ表示で弾コライダーサイズが変わるか確認

---

## ダメージ/コリジョン系の将来計画（除外理由の記録）

将来的に `Collision/System/` に移したいファイル群：

| ファイル | 理由 |
|---------|------|
| `DamageApplySystem` | 当たり判定結果を受けてダメージを適用するロジック |
| `DamageFlashSystem` | ダメージ受け時の視覚エフェクト |
| `EnemyContactDamageSystem` | 敵との接触ダメージ → 当たった効果 |
| `EnemyBulletHitSystem` | 弾の命中判定結果処理 → 当たった効果 |

これらは現在 `HealthComponent`・`PlayerHitboxComponent`・`AttackDamageComponent` 等を
複合的に参照しており、Collision/ 系への移動には Component の依存整理が前提となる。
別 Phase として独立させること。
