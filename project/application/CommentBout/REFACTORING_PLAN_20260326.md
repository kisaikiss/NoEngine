# CommentBout リファクタリング・追加実装手順（2026-03-26）

## 0. 事前決定（今回確定）
- [x] 計画書は新規ファイルで管理する
- [x] 進め方は `フェーズ + チェックリスト` 形式
- [x] リソース読込タイミングは `GameScene::Setup()` 一括ロード
- [x] リソース識別子は文字列キー（例: `PlayerSprite`）
- [x] 未確定パスは暫定パスで先行実装可
- [x] レール進行率は「レール全長に対する現在距離」
- [x] 進行バーUIは `SpriteComponent` ベース
- [x] 敵射撃条件に「画面内判定 + 距離閾値」を導入
- [x] 射撃距離閾値 / 後方削除距離は `EnemyTypePreset` に追加
- [x] ポーズ切替はエンジン `PauseComponent` 一本化
- [x] 自機慣性パラメータ追加 + 無敵トグル（左右Shift同時押し）
- [x] フィールドエディタは今回は実装せず、実装手順のみ作成

---

## 1. リソース一元化（モデル/テクスチャ）

### 方針（Q1への回答）
- [x] `GameResourceComponent` を拡張して実施する（既存参照を壊しにくく、段階移行が安全）
- [x] 将来的に名前を `GameResourceCatalogComponent` へ変更できる構成にする

### 補足（Q2への回答）
- [x] 同一ファイルを別キーで持つ運用は可能
- [x] 例: `PlayerSprite` と `PlayerAttackSprite` が同一パスでも、キーは分離可能
- [x] エンジンの `TextureManager` はパス単位キャッシュのため、同一実体を共有できる

### 実装ステップ
- [x] `GameResourceComponent` に `std::unordered_map<std::string, TextureRef>` / `std::unordered_map<std::string, ModelId>` 相当を追加
- [x] `GameScene::Setup()` に「リソース登録関数」を新設（読込場所を1か所化）
- [x] 既存の直接 `LoadCovertTexture` / `LoadModel` 呼び出しを、段階的にカタログ参照へ置換
- [x] 暫定で `resources` パスをすべてカタログへ集約
- [x] 「現在利用中パス一覧」を計画時点で台帳化（下記）

### 現在コードで確認できるパス（初期台帳）
- [x] `resources/engine/white1x1.png`
- [x] `resources/game/td_3105/Model/cube/cube.obj`（地面、草、敵、敵弾などで利用中）
- [x] `resources/game/td_3105/Sprite/Pause.png`
- [x] `resources/game/td_3105/Sprite/PauseToGame.png`
- [x] `resources/game/td_3105/Sprite/Restart.png`
- [x] `resources/game/td_3105/Sprite/OptionMenu.png`
- [x] `resources/game/td_3105/Sprite/PauseToTitle.png`
- [x] `resources/game/td_3105/Sprite/TitleRogo.png`
- [x] `resources/game/td_3105/Sprite/GameStart.png`
- [x] `resources/game/td_3105/Sprite/GameEnd.png`
- [x] `resources/game/td_3105/Sprite/Master.png`
- [x] `resources/game/td_3105/Sprite/BGM.png`
- [x] `resources/game/td_3105/Sprite/SE.png`
- [x] `resources/game/td_3105/Sprite/Vibration.png`
- [x] `resources/game/td_3105/Sprite/Back.png`
- [x] `resources/game/td_3105/Sprite/On.png`
- [x] `resources/game/td_3105/Sprite/Off.png`

### 新規追加予定リソース（要件リスト）
- [x] 自機スプライト
- [x] 自機攻撃スプライト
- [x] ボスモデル
- [x] 敵モデル
- [x] 射撃敵モデル
- [x] 敵弾モデル
- [x] 草モデル
- [x] ポーズ系スプライト各種
- [x] オプション系スプライト各種
- [x] タイトル系スプライト各種
- [x] オーブスプライト

---

## 2. レール進行バー表示

### 実装ステップ
- [x] `RailProgressBarComponent`（仮称）を追加
- [x] `RailProgressBarSystem`（仮称）を追加し、`distance / totalLength` で進行率更新
- [x] `start / goal / barBase / barFill / playerMarker` スプライトを生成・管理
- [x] `barFill` を「スタート→自機地点」まで色付きで描画
- [x] 自機地点マーカーは進行率にスムーズ追従（補間）
- [x] 自機地点マーカーに軽い拡縮アニメを付与
- [x] `%表示` は今回は未実装（将来拡張ポイントとして記録）

### 完了条件
- [x] 進行バーが常時表示される
- [x] スタート?現在地点が色付きで見える
- [x] マーカーが進行と連動し、見た目に動きがある

---

## 3. 敵射撃条件・削除条件の追加

### 実装ステップ
- [x] `EnemyTypePreset` に `shootDistanceMax` を追加
- [x] `EnemyTypePreset` に `despawnBehindDistance` を追加
- [x] プリセットJSON入出力（IO/Editor）へ新規項目を反映
- [x] カメラ可視判定ヘルパー（別ファイル）を追加
- [x] `EnemyShootSystem` で「画面内 + 距離以下」の時だけ発射
- [x] `EnemyMoveSystem` か専用Systemで「自機後方距離超過」削除を実装
- [x] 削除理由を区別するフラグ/列挙を導入（撃破/自然消滅）
- [x] 自然消滅時はオーブ未生成を保証
- [x] 対象箇所へ日本語コメントを追加（削除理由が分かる形）

### 完了条件
- [x] 画面外では敵が撃たない
- [x] 距離外では敵が撃たない
- [x] 後方に流れた敵が自然消滅し、オーブが出ない

---

## 4. ポーズ統合（エンジン `PauseComponent` 一本化）

### 実装ステップ
- [x] `PauseSystem` でエンジン `PauseComponent::isPause` を更新する責務に統一
- [x] `PauseStateComponent::isPaused` は同期用または廃止のどちらかに整理
- [x] ポーズ解除時/シーン遷移時に `PauseComponent::isPause=false` を明示
- [x] 各システムの `SetStopInPause(false)` をコンストラクタで設定

### `SetStopInPause(false)` 対象（現時点推奨）
- [x] `PauseSystem`
- [x] `OptionSystem`
- [x] `PauseViewSystem`
- [x] `OptionViewSystem`
- [x] （必要なら）`TitleSystem` / `TitleViewSystem` も同様に方針統一

### 完了条件
- [x] ポーズ開閉がエンジンpauseに一致する
- [x] ポーズメニュー中に必要なUI系Systemのみ動く
- [x] ポーズ解除漏れでゲームが止まり続ける事象がない

---

## 5. 自機慣性・無敵デバッグ

### 実装ステップ
- [x] `PlayerComponent` に `acceleration / deceleration / maxSpeed` を追加
- [x] `PlayerControlSystem` を速度ベース移動へ変更（慣性挙動）
- [x] ImGuiで移動パラメータ調整UIを追加
- [x] 左右Shift同時押しトグルで無敵状態を切替
- [x] 無敵状態を示すフラグをコンポーネントに追加
- [x] 無敵中は右上ImGuiに「自機無敵中」を表示

### 完了条件
- [x] 慣性ありの移動になり、停止時は減速で止まる
- [x] パラメータをImGuiで調整できる
- [x] 無敵トグルが機能し、状態表示される

---

## 6. フィールドエディタ（実装手順のみ）

### 6-1. データ設計
- [x] 配置データJSONを新設（ステージごと）
- [x] 種類別デフォルト当たり判定JSONを別ファイルで新設
- [x] オブジェクト種別を拡張しやすいID/キー方式で定義

### 6-2. 種別初期定義
- [x] `Skydome`（当たり判定なし）
- [x] `Building`（当たり判定あり）
- [x] `Building` は `scale(1,1,1)` 基準AABBをデフォルト設定JSONに保持
- [x] `Skydome` モデル: `resources/game/td_3105/Model/skydome/skydome.gltf`
- [x] `Building` モデル: 当面 `cube` で代用

### 6-3. コンポーネント/システム
- [x] 配置オブジェクト共通コンポーネントを作成（モデルキー、表示名、種別、衝突設定）
- [x] Transform編集対象を `座標/大きさ/回転` に限定
- [x] スケール変更時もAABBが正しく追従する計算を統一

### 6-4. エディタUI（Editタブ内）
- [x] 配置エディタ（追加/削除/編集）
- [x] 追加時の種別コンボ選択
- [x] 既配置オブジェクト一覧（制御点エディタに近い操作感）
- [x] 種類ごとのデフォルト当たり判定編集UI

### 6-5. 遮蔽仕様（第1段階）
- [x] プレイヤー攻撃のみ遮蔽判定対象にする
- [x] 敵弾遮蔽は将来拡張として仕様枠だけ残す

### 完了条件
- [x] 2種別を配置・保存・再読込できる
- [x] デフォルト当たり判定を種別ごとに編集できる
- [x] Building遮蔽時にプレイヤー攻撃が通らない

---

## 7. 回帰確認チェック
- [ ] `SpawnEnemy / RailStop / RailResume` が従来動作する
- [ ] `MoveOnly / MoveAndShoot / Boss` 生成が壊れていない
- [ ] ポーズ/オプション/タイトル遷移が正常
- [ ] HPバー・被ダメ・オーブ導線に副作用がない
- [ ] C++14でビルド成功

---

## 3.5 命中安定化・進行バー設定永続化

### 実装ステップ
- [x] レール進行バー設定のJSON保存/読込を追加
- [x] Start/Endスプライト共通サイズを調整可能化
- [x] 自機位置マーカーサイズを調整可能化
- [x] プレイヤー攻撃の初回フレーム限定判定を撤廃し命中を安定化
- [x] 敵同士遮蔽ロジックは継続
- [x] 後方削除判定を全敵共通へ適用
- [x] 敵弾にも後方削除判定を適用
- [x] 判定意図と運用方針を日本語コメントでコードへ明記

### 完了条件
- [x] 進行バー設定がJSONで復元される
- [x] ボス単体でも攻撃の取りこぼしが発生しにくい
- [x] 敵同士遮蔽が従来通り機能する
- [x] 後方へ流れた敵/敵弾が自然に消える

### コメント方針（運用明記）
- [x] 判定ロジック変更時は「なぜその判定が必要か」を日本語コメントで残す
- [x] 削除理由（撃破/自然消滅）は挙動差分が分かるコメントを付ける

---

## 8. フィールドオブジェクト統合（日本語UI / Ground追加 / 草廃止 / 敵弾遮蔽 / デバッグ表示）

### 実装ステップ
- [x] Field EditorのImGui表示文言を日本語化する（`typeKey` とJSONキーは英語のまま維持）
- [x] フィールド種別に `Ground` を追加する（`typeKey` は `Ground`）
- [x] `GameScene::Setup()` の手動 `groundEntity` 生成を削除し、フィールドオブジェクト管理へ一本化する
- [x] 草関連を撤去する（`SpawnGrass` / `CBGrassTag` / `GrassReactionSystem` / `HitBalloonSystem` / `GrassReactionComponent`）
- [x] `hasCollision=true` の全フィールドオブジェクトを敵弾衝突対象にする
- [x] 敵弾がフィールドオブジェクトと衝突したら即消滅する（演出差し込み可能な拡張ポイントを残す）
- [x] `FieldEditorSystem` に当たり判定デバッグ表示トグルを追加する

### 完了条件
- [x] Field Editorが日本語UIで操作できる
- [x] Groundがフィールドオブジェクトとして配置/保存/再読込できる
- [x] 手動Ground生成がなくても従来と同等にプレイ可能
- [x] 草関連コードが除去され、ビルドエラーが出ない
- [x] 敵弾が `hasCollision=true` のフィールドオブジェクトで消滅する
- [x] フィールドオブジェクトの当たり判定をトグルで可視化できる

---

## 9. 自機関連パラメータのJSON保存・配布コンポーネント化

### 実装ステップ
- [x] `PlayerConfigComponent`（仮称）を追加し、設定保持専用Entity（`CBPlayerConfigTag`）を導入する
- [x] 保存対象を定義する（`PlayerComponent` / `PlayerAttackComponent` / `HealthComponent.maxHp` / `InvincibleComponent.duration` / `PlayerHitboxComponent`）
- [x] `resources/game/td_3105/RailData/PlayerConfig.json` の読込/保存処理を実装する
- [x] `GameScene::Setup()` でPlayer設定Entityを生成し、各コンポーネントへ配布する
- [x] 自機の初期2D座標は `StartTransform2DComponent` で管理し、`Transform2DComponent` は保存対象から除外する
- [x] 必要に応じて編集UI（ImGui）を追加し、保存/読込の動作確認を行う

### 完了条件
- [x] 自機関連設定がJSONで復元される
- [x] 自機生成時に設定専用Entityから値が反映される
- [x] `Transform2DComponent` を直接保存しなくても初期座標が再現される

---

## 9.5 自機設定UI統合・日本語化・点滅設定拡張

### 実装ステップ
- [x] 自機関連ImGuiを `PlayerInfoDebugSystem` に統合する（移動・攻撃・当たり判定・JSON操作）
- [x] `PlayerControlSystem` の移動デバッグUIを削除する
- [x] `EnemyVisualSystem` の `PlayerHitbox` カメラゲート編集UIを削除する
- [x] `PlayerInfoDebugSystem` の項目名を日本語化し、カテゴリごとに整理する
- [x] `PlayerConfigComponent` / `PlayerConfig.json` に被弾点滅（フラッシュ）関連設定を追加する
- [x] 点滅設定をダメージ表示処理（`DamageApplySystem` / `DamageFlashSystem`）へ反映する
- [x] `Load JSON` 実行時にプレイヤーへ即時反映されることを確認する

### 完了条件
- [x] 自機設定が1つのImGuiウィンドウで完結して調整できる
- [x] 自機関連の変数表示名が日本語で分かりやすい
- [x] カメラゲート設定が自機側UIで編集できる
- [x] 点滅設定がJSONで保存/読込され、被弾表示へ反映される
- [x] JSONロード後に値が即座にプレイヤー挙動へ反映される

---

## 10. スプライトレイヤー管理の一元化（CommentBout内限定）

### 実装ステップ
- [ ] `CBSpriteLayer.h` を拡張し、用途別レイヤー定数を追加する
- [ ] `application/CommentBout` 配下のスプライトlayer直書きを棚卸しする
- [ ] layerのハードコード値を `CBSpriteLayer.h` 経由に置換する
- [ ] `OptionMenuConfigComponent` などのlayer可変設定を廃止し、固定定数参照に統一する
- [ ] 関連する読込/保存/ViewSystem側のlayer処理を整理する

### 完了条件
- [ ] `application/CommentBout` 内のスプライトlayer直書きが解消される
- [ ] layer管理が `CBSpriteLayer.h` から追える状態になる
- [ ] Option系を含め、layerがコード側固定方針で統一される

---

## 11. ファイル分割方針（設計手順の明文化フェーズ）

### 実装ステップ
- [ ] `Component` を `Player` / `Enemy` 軸で再配置する手順を策定する
- [ ] `System` を `Player` / `Enemy` 軸で再配置する手順を策定する
- [ ] `HPシステム` / `レールカメラ` は現状位置維持の例外ルールを明文化する
- [ ] `FieldObject` の再配置方針を明文化する（Editor系は `application/CommentBout/Editor`、System系は `application/CommentBout/System/FieldObject/*`）
- [ ] 依存include更新・段階移行・ビルド確認のチェックリストを作る

### 完了条件
- [ ] 物理移動前に、実施順と影響範囲がmdで共有されている
- [ ] 例外ルール（HP/レールカメラ維持）が明記されている
- [ ] FieldObjectの移設先ルールが明記されている

---

## 12. 衝突管理見直し検討（新規検討フェーズ）

### 検討テーマ
- [ ] Q1: 当たり判定デバッグ表示（敵/自機/フィールド）を一括管理する方式の可否を評価する
- [ ] Q2: 衝突判定を `CollisionSystem` に集約し、`OnCollision` 的な通知で各System責務を分離する方式を評価する

### 検討ステップ
- [ ] 現行の衝突関連責務を棚卸しする（判定/命中処理/演出/デバッグ表示）
- [ ] 一括デバッグ表示の導入コストと既存影響を見積もる
- [ ] `OnCollision` イベント方式の最小導入案を作る
- [ ] 過分離による煩雑化を避ける責務境界案を作る
- [ ] 採用/不採用を判断し、採用時は次フェーズへ昇格する

### 完了条件
- [ ] 一括デバッグ表示を「採用 / 非採用」で判断できる
- [ ] 衝突処理の集約方針を「段階導入案」まで落とし込める
- [ ] 次の実装フェーズへ進める判断材料が揃う
