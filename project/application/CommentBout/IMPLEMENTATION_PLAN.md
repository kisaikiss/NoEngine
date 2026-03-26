# CommentBout 実装予定タスクリスト

## 目的
実装し残しを防ぐため、課題を依存順で分解し、完了条件を明確化する。

---

## Phase 1: 敵プリセット運用の一本化

### 1-1. `EnemyTypePreset` を実運用に統合
- [x] `EnemySpawnSystem` で `EnemyTypePreset.json` を必須適用
- [x] 種類別ハードコード値（HP/scale/弾速/間隔）を削除
- [x] `Preset -> Event上書き` 順で最終パラメータを決定
- [x] `hp` はEventから排斥し、Preset固定で運用する

### 1-2. プリセット編集UIを追加
- [x] `EnemyTypePresetEditor`（ImGui）を追加
- [x] `modelScale/minHp/shootInterval/bulletSpeed/bulletDamage/bulletLifetime/targetDepth` を編集可能にする
- [x] 種類ごとの `baseColliderBox(x,y,z)`（scale=(1,1,1)基準AABBサイズ）を編集可能にする
- [x] JSON保存/読込ボタンを追加

### 1-3. Event Editorからプリセット項目を排斥
- [x] `GameEventEditor` で `enemyType` コンボは残す
- [x] `hp` 等のプリセット由来項目を非表示化（Override未採用）
- [x] `Boss` で必要な軌道/行動パラメータのみ編集可にする

### 1-4. `baseColliderBox` 適用処理を統合
- [x] Spawn時に `Collider3DComponent` へ `baseColliderBox` を反映
- [x] `boxSizeMultiplier = baseColliderBox` で運用し、`transform.scale` 変更時にも判定が崩れないことを確認

## 実装ログ（進捗記録）
- [x] Phase 1 完了
- [ ] Phase 2 完了
- [ ] Phase 3 完了
- [ ] Phase 4 完了
- [ ] 最終ビルド成功

---

## Phase 2: HPバー系の安定化と汎用化

### 2-1. `HpBarComponent` / `HpBarViewSystem` への移行完了
- [x] Boss/Playerとも `HpBarComponent` に統一
- [x] 旧 `BossHpBarComponent` 依存を削除
- [x] 参照先 (`EnemyRewardToBossSystem`, `EnemyVisualSystem`) を統一

### 2-2. ボス未出現時ゲージ減少バグ修正
- [x] target未設定時の表示ポリシーを固定（非表示 or 0固定）
- [x] `delayedRatio` 初期化と再取得時初期化を修正

### 2-3. プレイヤーHPバー表示修正
- [x] `CBPlayerHpBarTag` エンティティ生成を確認
- [x] `targetEntity=player` の設定を保証
- [x] layer/order/anchor/pivot を見える設定に固定

### 2-4. HPバー編集UI + JSON保存
- [x] Boss/Playerそれぞれの `anchor/size/layer/order/color/delayedSpeed` 編集UI
- [x] `Stage_01_hpbar.json` へ保存/読込

### 2-5. シェイク適用範囲修正
- [x] 外枠・赤遅延・緑ゲージを同一オフセットで移動
- [x] 親子構造で揺らす方式に整理（可能なら）

## 実装ログ（進捗記録）
- [x] Phase 1 完了
- [x] Phase 2 完了
- [ ] Phase 3 完了
- [ ] Phase 4 完了
- [ ] 最終ビルド成功

---

## Phase 3: 被ダメ演出と当たり判定（再設計）

### 3-1. 判定責務の分離（照準点と被弾判定）
- [x] 自機前方点は「敵弾の照準計算専用」であることを明文化
- [x] 被弾判定は照準点を参照しない
- [x] 被弾判定は `EnemyContactDamageSystem` と同一思想（投影 + ゲート）に統一

### 3-2. 敵弾判定を投影ベースへ統一
- [x] `EnemyBulletHitSystem` の判定を 3D hitbox 依存から撤去
- [x] `ProjectedCollider`（敵弾） vs `PlayerSpriteAABB` + `CameraGate` で命中判定
- [x] 判定ヘルパーは接触判定と同等ロジックを使う（重複最小）

### 3-3. 敵弾消滅ポリシーの固定
- [x] 敵弾は「時間経過（Lifetime）」または「衝突時」で消滅
- [x] プレイヤー命中時のみ `DamageRequest` 発行
- [x] プレイヤー非命中の衝突ではダメージは発行しない

### 3-4. 被ダメフラッシュ共通化
- [x] `DamageFlashComponent` + `DamageFlashSystem` を唯一のフラッシュ経路にする
- [x] 敵/自機とも `DamageApplySystem` 経由で対象限定フラッシュ
- [x] 命中した対象以外はフラッシュしない

### 3-5. 不要依存の整理
- [x] 弾被弾判定用途の `PlayerHitboxSyncSystem` を撤去
- [x] 弾判定で不要になった `PlayerHitboxComponent` 3D同期項目を整理

### 3-6. 報酬オーブダメージ対象の固定
- [x] オーブ到達時ダメージは `CBBossTag` 保持Entityのみに適用
- [x] ボス不在時はダメージを発行しない

## 実装ログ（進捗記録）
- [x] Phase 1 完了
- [x] Phase 2 完了
- [x] Phase 3 完了
- [ ] Phase 4 完了
- [ ] 最終ビルド成功

---

## Phase 4: コメントと保守性整備

### 4-1. 全コンポーネントコメント統一
- [x] 役割
- [x] 主更新System
- [x] ランタイム/保存対象

### 4-2. System依存コメントの整備
- [x] `GameScene` の実行順コメントを最新化
- [x] 依存（前段/後段）を明記

### 4-3. Playerヘルス系の統合
- [x] `HealthComponent + InvincibleComponent` を主系に統一
- [x] `PlayerComponent` へプレイヤー固有設定（例: invincibleDurationDefault）を集約
- [x] `PlayerHealthComponent` を段階廃止（互換同期コードを削除）

## 実装ログ（進捗記録）
- [x] Phase 1 完了
- [x] Phase 2 完了
- [x] Phase 3 完了
- [x] Phase 4 完了
- [x] 最終ビルド成功

---

## 回帰確認チェックリスト
- [ ] `SpawnEnemy` / `RailStop` / `RailResume` が従来通り動作
- [ ] `MoveOnly` / `MoveAndShoot` / `Boss` が正しく生成
- [ ] ボス八の字→停止→3連射ループが継続
- [ ] 敵撃破オーブがHPバーへ到達しボスにダメージ
- [ ] Boss/Player HPバーの編集保存が有効
- [ ] 敵弾・接触ともプレイヤーダメージが入る

---

## 実装ログ（進捗記録）
- [x] Phase 1 完了
- [x] Phase 2 完了
- [x] Phase 3 完了
- [x] Phase 4 完了
- [x] 最終ビルド成功
