# Editorリファクタリング 実装計画

## 方針
- 影響範囲を抑えるため、`Phase`単位で実装・ビルド・記録を繰り返す。
- 各Phase完了時にビルドを実行し、いったん停止して確認可能な状態にする。

## Phase 1: ステージ再読込と再生成抑止
### 目的
1. ステージ名確定時に `field/rail/event` をすべて再読込する。
2. 存在しないステージ名確定時に、現状状態から新規ステージファイル群を作成する。
3. レール/イベント個別読込で敵が再生成される問題を抑止する。

### 実装予定
- `EditorManager` に再読込共通処理を追加。
- ステージ名確定時、存在確認後に全読込または新規保存＋全読込を実行。
- レール個別読込時は既存敵を削除し、再スポーンを防止。
- イベント個別読込時は「現在距離より手前」のイベントを既発火扱いにして再スポーン抑止。

### 検証
- ビルド成功。
- ステージ確定時に3種データが反映されること。
- レール/イベント個別読込で敵重複が起きないこと。

## Phase 1 実装結果

### 変更内容
- `EditorManager`:
  - ステージ名確定時に、既存ステージなら `LoadAll`、未存在ステージなら `SaveAll` 後に `LoadAll` を実行するよう変更。
  - `LoadAll` で `rail/event` 読込後に、既存の敵・敵弾・スポーン要求を削除する処理を追加。
  - レール個別読込時に、既存の敵・敵弾・スポーン要求を削除して再生成重複を抑止。
  - イベント個別読込時に、現在レール距離より手前のイベントを既発火扱いにする処理を追加。
- `FieldObjectEditor`:
  - `Save` 時の保存先ステージ名を `RailCameraComponent::stageName` から解決するよう変更。
  - これにより、新規ステージ確定直後の `SaveAll` でもフィールド配置が新ステージ名で保存される。

### ビルド
- 結果: 成功

## Phase 2: Editorデバッグ表示改良
### 目的
1. `デバッグ表示` 内に衝突デバッグ設定UIを統合する。
2. `LeftShift+1` で Debug/Rail カメラ切替。
3. `LeftShift+2` でデバッグ表示一括開閉トグル。

### 実装予定
- 衝突デバッグ設定コンポーネントを `EditorManager` から編集可能にする。
- 既存の衝突デバッグ専用ウィンドウ表示を抑制または移設。
- `GameScene::NotSystemUpdate()` 付近でショートカットトグル処理を追加。

### 検証
- ビルド成功。
- UI統合とショートカットの動作確認。

## Phase 3: 敵弾パラメータ拡張と保存不整合修正
### 目的
1. 敵弾のモデルサイズ・コライダーサイズ・オフセットを編集可能にする。
2. ボス射撃パラメータ変更が保存されない問題を解消する。

### 実装予定
- `EnemyConfig` / `EnemyDataIO` / `EnemyShootSystem` / `EditorManager` の対応項目を拡張。
- 保存対象キーと読込キーの整合を確認・修正。

### 検証
- ビルド成功。
- JSON保存後の再読込で値が保持されること。

## C案 実装（状態機械）

### C-Phase 1: レール停止/再開の状態機械化
#### 変更内容
- `RailCameraComponent` に実行状態を追加:
  - `RailRuntimeState { Playing / Stopped / WaitingResumeCondition }`
  - 待機条件の実行コンテキスト（条件種別・秒数・対象グループ・経過時間・発生元イベントindex）
- `RailDataIO::ResetEventRuntime` で、イベント実行フラグに加えて状態機械コンテキストも初期化。
- `RailCameraSystem` を状態機械ベースへ更新:
  - `RailStop` で停止。
  - `RailStop` 自身に再開条件がある場合は `WaitingResumeCondition` へ遷移。
  - 待機条件（秒数/敵全滅）を毎フレーム評価し、成立で `Playing` へ復帰。
  - `RailResume` は後方互換として維持（明示再開/条件待機の両方対応）。
- `GameEventEditor` 拡張:
  - `RailStop` に「停止後再開条件」を設定可能化（C案の操作導線）。
- `EditorManager` のイベント再読込補正を更新:
  - `MarkPastEventsFiredByDistance` で現在距離時点の停止/待機状態を復元。
  - 停止位置でロードしても、状態機械側で再開条件評価が継続される構成に変更。

#### ビルド
- 結果: 成功

## C-Phase 2 設計（互換コード排斥）

### 全体精査の要否
- 必要。
- 理由: `RailResume` 互換コードが `Component / DataIO / Editor / RuntimeSystem / Scene初期化` に分散しており、部分修正では不整合が残るため。

### 目標
- `Stop/Resume分離モデル` を廃止し、`RailStop` + `resumeCondition` のみで停止/再開を完結させる。
- 旧互換コード（`RailResume` 分岐、イベントごとの `waitingCondition` など）を削除する。

### 仕様確定
1. イベント種別は `SpawnEnemy` と `RailStop` のみ。
2. `RailStop` は以下を持つ:
   - `resumeCondition = None`: 停止し続ける（エディタで手動再開 or 条件変更）
   - `AfterSeconds`: 指定秒で再開
   - `EnemiesCleared`: 指定グループ全滅で再開
3. ランタイムの待機状態は `RailCameraComponent` 側にのみ保持（状態機械）。
4. イベント読込時の距離補正は、`RailStop` だけを使って現在状態を再構築する。

### 排斥対象
- `RailEventType::RailResume`
- `GameEventEditor` の再開イベント追加/UI
- `RailCameraSystem` の `RailResume` 処理分岐
- `GameScene` 初期イベント生成の `RailResume`
- `RailEventData::waitingCondition / waitingElapsedSeconds`（イベント側実行状態）

### リスクと対策
- 旧JSONに `RailResume` が残ると読込意図が変わる。
  - 本Phaseでは互換を排除し、旧データ変換は対象外。
  - 必要なら後続でオフライン変換ツールを別Phaseで用意。

### C-Phase 2 実装結果
#### 実施
- `RailEventType` を `SpawnEnemy / RailStop` の2種へ整理。
- `RailEventData` から旧イベント側実行状態 (`waitingCondition`, `waitingElapsedSeconds`) を削除。
- `GameEventEditor`:
  - 「再開イベント追加」を削除。
  - 種別コンボを2種へ縮退。
  - 停止イベントに再開条件編集を集約。
- `RailCameraSystem`:
  - `RailResume` 分岐を削除。
  - 実行順を `Spawn -> Stop` の2パスで固定化（同距離時の挙動安定化）。
- `EditorManager`:
  - イベント読込後の状態復元を `RailStop` ベースに簡素化。
- `GameScene`:
  - 初期フォールバックイベントを `RailStop + AfterSeconds` 1件に統一。
- `RailDataIO`:
  - 非対応イベント種別を読込スキップする厳格パースへ変更。

#### ビルド
- 結果: 成功

## C-Phase 3 設計（旧JSON自動移行）

### 目的
- `RailResume` を含む旧イベントJSONを、現行仕様（`SpawnEnemy` + `RailStop`）へ安全に移行する。
- 互換コードを戻さず、データのみを読込時に正規化する。

### 方針
1. `LoadEventsToComponent` で `RailResume` を検出したら通常読込対象から除外し、レガシーデータとして収集する。
2. 収集後に、各 `RailResume` を最も近い直前の `RailStop` へ統合（`resumeCondition/resumeAfterSeconds/targetGroupId` を上書き）。
3. 対応する `RailStop` が存在しない場合は、新規 `RailStop` を生成して補完。
4. 移行が発生したファイルは現行形式で自動再保存する。

## C-Phase 3 実装結果
#### 実施
- `RailDataIO` に旧 `RailResume` 収集構造を追加。
- 旧 `RailResume` を `RailStop` へ統合する移行処理を追加。
- イベント配列を `triggerDistance` と種別優先（`Spawn -> Stop`）で再整列する処理を追加。
- 旧イベント検出時に `SaveEventsToJson` を呼び、ファイルを現行フォーマットへ自動更新。

- 既存データの即時整合:
  - `Stage_01_events.json` / `Stage_02_events.json` から `RailResume` を排除。
  - 停止イベントへ再開条件を統合した現行形式へ更新。

#### ビルド
- 結果: 成功
