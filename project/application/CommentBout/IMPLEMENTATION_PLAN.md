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
- [ ] Boss/Playerとも `HpBarComponent` に統一
- [ ] 旧 `BossHpBarComponent` 依存を削除
- [ ] 参照先 (`EnemyRewardToBossSystem`, `EnemyVisualSystem`) を統一

**完了条件**
- コード上で `BossHpBarComponent` 参照が残っていない。

### 2-2. ボス未出現時ゲージ減少バグ修正
- [ ] target未設定時の表示ポリシーを固定（非表示 or 0固定）
- [ ] `delayedRatio` 初期化と再取得時初期化を修正

**完了条件**
- ボス出現前にゲージが減らない。

### 2-3. プレイヤーHPバー表示修正
- [ ] `CBPlayerHpBarTag` エンティティ生成を確認
- [ ] `targetEntity=player` の設定を保証
- [ ] layer/order/anchor/pivot を見える設定に固定

**完了条件**
- 左下（指定位置）に常時表示される。

### 2-4. HPバー編集UI + JSON保存
- [ ] Boss/Playerそれぞれの `anchor/size/layer/order/color/delayedSpeed` 編集UI
- [ ] `Stage_01_hpbar.json` へ保存/読込

**完了条件**
- 両バーの編集値が保存・復元される。

### 2-5. シェイク適用範囲修正
- [ ] 外枠・赤遅延・緑ゲージを同一オフセットで移動
- [ ] 親子構造で揺らす方式に整理（可能なら）

**完了条件**
- 被弾時にバー全体が揺れる。
- 

---

## Phase 3: 被ダメ演出と当たり判定

### 3-1. 自機被ダメリアクション
- [ ] 接触被弾・敵弾被弾の双方で同一フラッシュ制御
- [ ] 既存色復帰を保証

**完了条件**
- 被弾時のみ短時間赤くなり、復帰する。

### 3-2. 自機と敵弾の当たり判定修正
- [ ] `PlayerHitbox` 3D位置が自機2Dと同期しているか確認
- [ ] `CollisionLayer/Mask` の双方向一致を確認
- [ ] `collidedEntity` 上書き問題がある場合は複数ヒット対応に改善

**完了条件**
- 敵弾が自機に安定して命中し、`DamageRequest` が発行される。

### 3-3. 被ダメフラッシュ共通化
- [ ] `PlayerDamageFlashComponent` を廃止方向にし、`DamageFlashComponent` を追加
- [ ] `timer/duration/flashColor/baseColor/affectSprite/affectMaterial` を持たせる
- [ ] 敵/自機とも同じ被ダメ経路（DamageRequest反映後）で制御
- [ ] 既存 `EnemyComponent::damageFlashTimer` は段階移行で最終削除

**完了条件**
- 敵と自機が同一仕組みでフラッシュする。

---

## Phase 4: コメントと保守性整備

### 4-1. 全コンポーネントコメント統一
- [ ] 役割
- [ ] 主更新System
- [ ] ランタイム/保存対象

**完了条件**
- `application/CommentBout/Component` / `FieldObject/Component` / `Collision/Component` の全 `.h` に責務コメントがある。

### 4-2. System依存コメントの整備
- [ ] `GameScene` の実行順コメントを最新化
- [ ] 依存（前段/後段）を明記

**完了条件**
- 更新順の意図がコメントのみで追える。

### 4-3. Playerヘルス系の統合
- [ ] `HealthComponent + InvincibleComponent` を主系に統一
- [ ] `PlayerComponent` へプレイヤー固有設定（例: invincibleDurationDefault）を集約
- [ ] `PlayerHealthComponent` を段階廃止（互換同期コードを削除）

**完了条件**
- `PlayerHealthComponent` 参照が主処理から消え、互換コードが撤去される。

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
- [ ] Phase 2 完了
- [ ] Phase 3 完了
- [ ] Phase 4 完了
- [ ] 最終ビルド成功
