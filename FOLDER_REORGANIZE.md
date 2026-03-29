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
