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
- [ ] `GameResourceComponent` を拡張して実施する（既存参照を壊しにくく、段階移行が安全）
- [ ] 将来的に名前を `GameResourceCatalogComponent` へ変更できる構成にする

### 補足（Q2への回答）
- [ ] 同一ファイルを別キーで持つ運用は可能
- [ ] 例: `PlayerSprite` と `PlayerAttackSprite` が同一パスでも、キーは分離可能
- [ ] エンジンの `TextureManager` はパス単位キャッシュのため、同一実体を共有できる

### 実装ステップ
- [ ] `GameResourceComponent` に `std::unordered_map<std::string, TextureRef>` / `std::unordered_map<std::string, ModelId>` 相当を追加
- [ ] `GameScene::Setup()` に「リソース登録関数」を新設（読込場所を1か所化）
- [ ] 既存の直接 `LoadCovertTexture` / `LoadModel` 呼び出しを、段階的にカタログ参照へ置換
- [ ] 暫定で `resources` パスをすべてカタログへ集約
- [ ] 「現在利用中パス一覧」を計画時点で台帳化（下記）

### 現在コードで確認できるパス（初期台帳）
- [ ] `resources/engine/white1x1.png`
- [ ] `resources/game/td_3105/Model/cube/cube.obj`（地面、草、敵、敵弾などで利用中）
- [ ] `resources/game/td_3105/Sprite/Pause.png`
- [ ] `resources/game/td_3105/Sprite/PauseToGame.png`
- [ ] `resources/game/td_3105/Sprite/Restart.png`
- [ ] `resources/game/td_3105/Sprite/OptionMenu.png`
- [ ] `resources/game/td_3105/Sprite/PauseToTitle.png`
- [ ] `resources/game/td_3105/Sprite/TitleRogo.png`
- [ ] `resources/game/td_3105/Sprite/GameStart.png`
- [ ] `resources/game/td_3105/Sprite/GameEnd.png`
- [ ] `resources/game/td_3105/Sprite/Master.png`
- [ ] `resources/game/td_3105/Sprite/BGM.png`
- [ ] `resources/game/td_3105/Sprite/SE.png`
- [ ] `resources/game/td_3105/Sprite/Vibration.png`
- [ ] `resources/game/td_3105/Sprite/Back.png`
- [ ] `resources/game/td_3105/Sprite/On.png`
- [ ] `resources/game/td_3105/Sprite/Off.png`

### 新規追加予定リソース（要件リスト）
- [ ] 自機スプライト
- [ ] 自機攻撃スプライト
- [ ] ボスモデル
- [ ] 敵モデル
- [ ] 射撃敵モデル
- [ ] 敵弾モデル
- [ ] 草モデル
- [ ] ポーズ系スプライト各種
- [ ] オプション系スプライト各種
- [ ] タイトル系スプライト各種
- [ ] オーブスプライト

---

## 2. レール進行バー表示

### 実装ステップ
- [ ] `RailProgressBarComponent`（仮称）を追加
- [ ] `RailProgressBarSystem`（仮称）を追加し、`distance / totalLength` で進行率更新
- [ ] `start / goal / barBase / barFill / playerMarker` スプライトを生成・管理
- [ ] `barFill` を「スタート→自機地点」まで色付きで描画
- [ ] 自機地点マーカーは進行率にスムーズ追従（補間）
- [ ] 自機地点マーカーに軽い拡縮アニメを付与
- [ ] `%表示` は今回は未実装（将来拡張ポイントとして記録）

### 完了条件
- [ ] 進行バーが常時表示される
- [ ] スタート?現在地点が色付きで見える
- [ ] マーカーが進行と連動し、見た目に動きがある

---

## 3. 敵射撃条件・削除条件の追加

### 実装ステップ
- [ ] `EnemyTypePreset` に `shootDistanceMax` を追加
- [ ] `EnemyTypePreset` に `despawnBehindDistance` を追加
- [ ] プリセットJSON入出力（IO/Editor）へ新規項目を反映
- [ ] カメラ可視判定ヘルパー（別ファイル）を追加
- [ ] `EnemyShootSystem` で「画面内 + 距離以下」の時だけ発射
- [ ] `EnemyMoveSystem` か専用Systemで「自機後方距離超過」削除を実装
- [ ] 削除理由を区別するフラグ/列挙を導入（撃破/自然消滅）
- [ ] 自然消滅時はオーブ未生成を保証
- [ ] 対象箇所へ日本語コメントを追加（削除理由が分かる形）

### 完了条件
- [ ] 画面外では敵が撃たない
- [ ] 距離外では敵が撃たない
- [ ] 後方に流れた敵が自然消滅し、オーブが出ない

---

## 4. ポーズ統合（エンジン `PauseComponent` 一本化）

### 実装ステップ
- [ ] `PauseSystem` でエンジン `PauseComponent::isPause` を更新する責務に統一
- [ ] `PauseStateComponent::isPaused` は同期用または廃止のどちらかに整理
- [ ] ポーズ解除時/シーン遷移時に `PauseComponent::isPause=false` を明示
- [ ] 各システムの `SetStopInPause(false)` をコンストラクタで設定

### `SetStopInPause(false)` 対象（現時点推奨）
- [ ] `PauseSystem`
- [ ] `OptionSystem`
- [ ] `PauseViewSystem`
- [ ] `OptionViewSystem`
- [ ] （必要なら）`TitleSystem` / `TitleViewSystem` も同様に方針統一

### 完了条件
- [ ] ポーズ開閉がエンジンpauseに一致する
- [ ] ポーズメニュー中に必要なUI系Systemのみ動く
- [ ] ポーズ解除漏れでゲームが止まり続ける事象がない

---

## 5. 自機慣性・無敵デバッグ

### 実装ステップ
- [ ] `PlayerComponent` に `acceleration / deceleration / maxSpeed` を追加
- [ ] `PlayerControlSystem` を速度ベース移動へ変更（慣性挙動）
- [ ] ImGuiで移動パラメータ調整UIを追加
- [ ] 左右Shift同時押しトグルで無敵状態を切替
- [ ] 無敵状態を示すフラグをコンポーネントに追加
- [ ] 無敵中は右上ImGuiに「自機無敵中」を表示

### 完了条件
- [ ] 慣性ありの移動になり、停止時は減速で止まる
- [ ] パラメータをImGuiで調整できる
- [ ] 無敵トグルが機能し、状態表示される

---

## 6. フィールドエディタ（実装手順のみ）

### 6-1. データ設計
- [ ] 配置データJSONを新設（ステージごと）
- [ ] 種類別デフォルト当たり判定JSONを別ファイルで新設
- [ ] オブジェクト種別を拡張しやすいID/キー方式で定義

### 6-2. 種別初期定義
- [ ] `Skydome`（当たり判定なし）
- [ ] `Building`（当たり判定あり）
- [ ] `Building` は `scale(1,1,1)` 基準AABBをデフォルト設定JSONに保持
- [ ] `Skydome` モデル: `resources/game/td_3105/Model/skydome/skydome.gltf`
- [ ] `Building` モデル: 当面 `cube` で代用

### 6-3. コンポーネント/システム
- [ ] 配置オブジェクト共通コンポーネントを作成（モデルキー、表示名、種別、衝突設定）
- [ ] Transform編集対象を `座標/大きさ/回転` に限定
- [ ] スケール変更時もAABBが正しく追従する計算を統一

### 6-4. エディタUI（Editタブ内）
- [ ] 配置エディタ（追加/削除/編集）
- [ ] 追加時の種別コンボ選択
- [ ] 既配置オブジェクト一覧（制御点エディタに近い操作感）
- [ ] 種類ごとのデフォルト当たり判定編集UI

### 6-5. 遮蔽仕様（第1段階）
- [ ] プレイヤー攻撃のみ遮蔽判定対象にする
- [ ] 敵弾遮蔽は将来拡張として仕様枠だけ残す

### 完了条件
- [ ] 2種別を配置・保存・再読込できる
- [ ] デフォルト当たり判定を種別ごとに編集できる
- [ ] Building遮蔽時にプレイヤー攻撃が通らない

---

## 7. 回帰確認チェック
- [ ] `SpawnEnemy / RailStop / RailResume` が従来動作する
- [ ] `MoveOnly / MoveAndShoot / Boss` 生成が壊れていない
- [ ] ポーズ/オプション/タイトル遷移が正常
- [ ] HPバー・被ダメ・オーブ導線に副作用がない
- [ ] C++14でビルド成功
