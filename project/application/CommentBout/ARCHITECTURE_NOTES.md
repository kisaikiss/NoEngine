# CommentBout Architecture Notes

## 目的
最小限の責務整理を明文化し、機能追加時の影響範囲を把握しやすくする。

## Core Flow
1. `RailCameraSystem`
- レール進行とイベント発火（`SpawnEnemyRequestComponent` 発行）

2. `EnemySpawnSystem`
- 敵実体の生成
- `enemyType` による初期コンポーネント付与（MoveOnly / MoveAndShoot / Boss）

3. `BossBehaviorSystem` / `EnemyMoveSystem` / `EnemyShootSystem`
- ボス挙動（カメラ相対原点、八の字、Stop/Shootフェーズ）
- 通常敵移動
- 敵弾生成

4. `CollisionSystem` + `EnemyBulletHitSystem` + `PlayerAttackResolveSystem`
- 3D/2D衝突更新
- 敵弾ヒット時の `DamageRequest`
- プレイヤー攻撃ヒット時の `DamageRequest`

5. `EnemyRewardToBossSystem` + `DamageApplySystem`
- 報酬オーブ到達ダメージを `DamageRequest` で反映
- 共通HP反映・死亡処理

6. `BossHpBarViewSystem` / `EnemyVisualSystem`
- ボスHPバー描画状態更新
- 敵の見た目更新と撃破時報酬オーブ生成

## Component Roles
- `HealthComponent`: 共通HPの主系
- `InvincibleComponent`: 共通無敵時間
- `PlayerHealthComponent`: 互換用（段階移行中）
- `EnemyShooterComponent`: 敵射撃設定
- `BossComponent`: ボス挙動パラメータ＋ランタイム状態
- `EnemyBulletComponent`: 敵弾速度/ダメージ
- `EnemyRewardOrbComponent`: 報酬オーブの曲線移動状態
- `BossHpBarComponent`: ボスバー表示状態

## Rule
- ダメージ反映は必ず `DamageRequestComponent` 経由で行う。
- 見た目更新Systemで直接HPを書き換えない。
- Boss移動は `BossBehaviorSystem` に限定する。
