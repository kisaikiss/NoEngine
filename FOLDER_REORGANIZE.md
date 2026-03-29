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

### Phase 6-A: 保存失敗の検出と確認手段の追加

**問題:** `SaveRailToJson` / `SaveEventsToJson` の戻り値を `EditorManager` が無視しており、
保存に失敗しても気づけない。実行時の作業ディレクトリが期待値と異なる可能性もある。

**確認手順（コード変更なし・目視確認）:**

1. Visual Studio の「デバッグ → プロパティ → 作業ディレクトリ」を確認し、
   `resources/game/td_3105/...` の相対パスが正しく解決されるか確認する
2. `resources/game/td_3105/Data/StageData/Stage_01/EventData/` フォルダが存在するかエクスプローラーで確認する
   （存在しない場合、`SaveEventsToJson` は無音で失敗している）
3. `resources/game/td_3105/Data/StageData/Stage_01/RailData/` フォルダの存在確認

**コード対応（保存失敗の可視化）:**

- [ ] `EditorManager::DrawRailTab()` の「JSONへ保存」ボタン後に `SaveRailToJson` の戻り値をチェックし、
  失敗時は `ImGui::TextColored(ImVec4(1,0,0,1), "保存失敗")` を表示
- [ ] `EditorManager::DrawEventTab()` の「JSONへ保存」ボタンで同様の対応
- [ ] `EditorManager::SaveAll()` でも両関数の戻り値をチェックし、失敗時は表示
- [ ] **ビルド確認・実行確認**（エラー表示が出る場合は Phase 6-B へ）

---

### Phase 6-B: 保存先ディレクトリの自動作成

**問題:** `EventData/` フォルダが存在しない場合 `std::ofstream` が失敗して JSON が保存されない。
`SaveRailToJson` も同様（`RailData/` フォルダ不在の場合の保険）。

**変更対象:** `Data/RailDataIO.cpp`

| 関数 | 追加内容 |
|------|---------|
| `SaveEventsToJson()` | `ofstream` の前に `std::filesystem::create_directories(parentPath)` を追加 |
| `SaveRailToJson()` | 同上 |

- [ ] `RailDataIO.cpp` に `#include <filesystem>` 追加
- [ ] `SaveEventsToJson` の `ofstream` 開く直前にディレクトリ作成処理を追加
  （`std::filesystem::path(filePath).parent_path()` を `create_directories`）
- [ ] `SaveRailToJson` に同様追加
- [ ] **ビルド確認**
- [ ] 実行して「JSONへ保存」ボタン後にファイルが生成されるか確認
- [ ] 生成されたJSONの内容が ImGui で設定した値と一致するか確認

---

### Phase 6-C: フィールドオブジェクト消滅問題の修正

**根本原因:** `LoadFieldPlacements()` の冒頭で `DestroyAllFieldObjects()` を実行した後、
ファイルが存在しない・読めない場合に `return` してしまう。
→ フィールドオブジェクトが消えたまま何も生成されない。

```
【現状の処理順】
DestroyAllFieldObjects(registry);  ← 先に全削除
ifstream ifs(filePath);
if (!ifs) { return; }              ← 失敗でそのままreturn → 消えたまま

【修正後の処理順】
ifstream ifs(filePath);
if (!ifs) { return; }              ← ファイルが読めなければ削除もしない
DestroyAllFieldObjects(registry);  ← 読めた場合だけ削除して再生成
```

**変更対象:** `Editor/FieldObjectEditor.cpp` の `LoadFieldPlacements()` 関数

- [ ] `LoadFieldPlacements()` の `DestroyAllFieldObjects()` 呼び出しをファイルオープン成功後に移動
  （`ifstream ifs` → `ifs >> json` → パース確認 → `DestroyAllFieldObjects` → エンティティ生成の順に変更）
- [ ] 動作確認: 存在しないステージ名でフィールド読込をしても既存オブジェクトが消えないことを確認
- [ ] 動作確認: 「フィールドオブジェクト追加 → フィールド保存 → フィールド読込」でオブジェクトが再生成されることを確認
- [ ] 動作確認: 「すべて保存 → すべて読込」でフィールドオブジェクトが正常に再表示されることを確認
- [ ] **ビルド確認・動作確認**

**補足確認:**
- [ ] `DrawRailTab()` / `DrawEventTab()` の個別「JSONから再読み込み」ボタンが
  `fieldObjectEditor_.ForceReload()` を呼んでいないことをコードレビューで確認する
  （呼んでいなければフィールドには影響しないため OK）

---

### Phase 6-D: 敵弾コライダーのデバッグ表示追加

**現状:** `CollisionDebugRenderSystem` は敵本体 (`CBRailEnemyTag`) のコライダーのみ表示。
敵弾 (`CBEnemyBulletTag`) は `showEnemyBulletCollider` フラグも描画処理も未実装。

**変更対象:**

| ファイル | 変更内容 |
|---------|---------|
| `Collision/Component/CollisionDebugConfigComponent.h` | `showEnemyBulletCollider = true` フィールド追加 |
| `Collision/System/CollisionDebugRenderSystem.h` | `DrawEnemyBulletColliderDebug()` 宣言追加 |
| `Collision/System/CollisionDebugRenderSystem.cpp` | 実装追加・チェックボックス追加・`Update()` から呼び出し |

**`DrawEnemyBulletColliderDebug()` の実装概要:**
```cpp
// CBEnemyBulletTag + Collider3DComponent をView
// shapeType が Sphere なら DrawSphere(worldPosition, worldRadius, color)
// Box なら DrawCube(worldPosition, worldBoxSize, color) （将来対応用）
```

- [ ] `CollisionDebugConfigComponent.h` に `bool showEnemyBulletCollider = true;` 追加
- [ ] `CollisionDebugRenderSystem.h` に `DrawEnemyBulletColliderDebug()` 宣言追加
- [ ] `CollisionDebugRenderSystem.cpp` の `DrawConfigImGui()` に `ImGui::Checkbox("敵弾コライダー", &config.showEnemyBulletCollider)` 追加（`enableCollisionDebug` が true のブロック内）
- [ ] `DrawEnemyBulletColliderDebug()` の実装追加（`CBEnemyBulletTag + Collider3DComponent` のView + Sphere描画）
- [ ] `Update()` から `DrawEnemyBulletColliderDebug(registry, *config)` を呼ぶ行を追加
- [ ] **ビルド確認**
- [ ] 実行して「敵弾コライダー」チェックONで飛んでいる弾のコライダーが表示されるか確認

---

### Phase 6-E: 敵弾コライダーのサイズ/オフセット ImGui 編集 + JSON 保存対応

**現状の問題:**

| 場所 | 現状 | 問題 |
|-----|------|-----|
| `EnemyConfig.bulletCollider` | 定義済み | - |
| `EnemyDataIO::Save/Load` | `bulletCollider` の読み書き実装済み | - |
| `EnemyShootSystem::SpawnEnemyBullet()` | `radiusMultiplier = 0.5f` ハードコード | `preset.bulletCollider` を参照していない |
| `EditorManager::DrawEnemyTab()` | `bulletCollider` の ImGui 編集がない | 値を変更できない |
| `EditorManager::ApplyEnemyPresetsToAliveEnemies()` | 弾コライダー反映がない | 生きている弾に設定が反映されない |

**変更対象:**

**① `Component/Enemy/EnemyShooterComponent.h`**
弾コライダー設定値をコンポーネントに持たせる:
```cpp
float   bulletColliderRadiusMultiplier = 0.5f;
No::Vector3 bulletColliderLocalOffset  = { 0.f, 0.f, 0.f };
```

**② `System/Enemy/EnemySpawnSystem.cpp`** の `SpawnRailEnemies()`
`shooter` コンポーネントへ preset の bulletCollider 値を反映:
```cpp
shooter->bulletColliderRadiusMultiplier = preset.bulletCollider.radiusMultiplier;
shooter->bulletColliderLocalOffset      = preset.bulletCollider.localOffset3D;
```

**③ `System/Enemy/EnemyShootSystem.cpp`** の `SpawnEnemyBullet()`
引数に `float radiusMultiplier, No::Vector3 localOffset` を追加し、コライダーに適用:
```cpp
collider->radiusMultiplier = radiusMultiplier;
collider->localOffset      = localOffset;
```
呼び出し元で `shooter->bulletColliderRadiusMultiplier`, `shooter->bulletColliderLocalOffset` を渡す。

**④ `Editor/EditorManager.cpp`** の `drawType` ラムダ
`ImGui::SeparatorText("弾コライダー")` + 以下の ImGui 編集を追加:
```cpp
changed |= ImGui::DragFloat("弾コライダー半径倍率", &cfg.bulletCollider.radiusMultiplier, 0.01f, 0.01f, 5.0f);
changed |= ImGui::DragFloat3("弾コライダーオフセット", &cfg.bulletCollider.localOffset3D.x, 0.01f);
```

**⑤ `Editor/EditorManager.cpp`** の `ApplyEnemyPresetsToAliveEnemies()`
shooter コンポーネントへ bulletCollider 値を反映する行を追加:
```cpp
shooter->bulletColliderRadiusMultiplier = cfg.bulletCollider.radiusMultiplier;
shooter->bulletColliderLocalOffset      = cfg.bulletCollider.localOffset3D;
```
（生きている弾自体への反映は不要。次弾から有効になればよい）

**チェックリスト:**

- [ ] `EnemyShooterComponent.h` に `bulletColliderRadiusMultiplier` / `bulletColliderLocalOffset` 追加
- [ ] `EnemySpawnSystem.cpp` の `SpawnRailEnemies()` で shooter に bulletCollider 設定を反映
- [ ] `EnemyShootSystem.cpp` の `SpawnEnemyBullet()` 関数シグネチャに radiusMultiplier / localOffset 引数追加
- [ ] `SpawnEnemyBullet()` 内の `collider->radiusMultiplier` をハードコード値から引数に変更
- [ ] `collider->localOffset` に引数の offset を反映（現在未設定）
- [ ] 呼び出し元2箇所（通常敵・ボス）で `shooter->bulletCollider...` を渡すよう更新
- [ ] `EditorManager::DrawEnemyTab()` の `drawType` ラムダに bulletCollider の ImGui 編集追加
- [ ] `EditorManager::ApplyEnemyPresetsToAliveEnemies()` の shooter ブロックに bulletCollider 反映追加
- [ ] **ビルド確認**
- [ ] 実行してエディタで値を変更 → JSON 保存 → 再起動後に値が反映されるか確認
- [ ] Phase 6-D のデバッグ表示で弾のコライダーサイズが変わるか確認

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
