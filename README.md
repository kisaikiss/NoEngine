# NoEngine

<img width="256" height="256" alt="NoEngine icon" src="https://github.com/user-attachments/assets/5727812f-eb1b-43ab-98a6-ab510aa8749c" />

> DirectX 12 とゲームエンジンの仕組みを学びながら開発している、Windows 向けの自作ゲームエンジンです。

| ブランチ | Debug | Development | Release |
| --- | --- | --- | --- |
| master | [![DebugBuild](https://github.com/kisaikiss/NoEngine/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/kisaikiss/NoEngine/actions/workflows/DebugBuild.yml) | [![DevelopmentBuild](https://github.com/kisaikiss/NoEngine/actions/workflows/DevelopmentBuild.yml/badge.svg)](https://github.com/kisaikiss/NoEngine/actions/workflows/DevelopmentBuild.yml) | [![ReleaseBuild](https://github.com/kisaikiss/NoEngine/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/kisaikiss/NoEngine/actions/workflows/ReleaseBuild.yml) |

## プロジェクトについて

NoEngine は 2025 年 11 月 3 日に本格的な制作を開始しました。「できないことなし」を目指す……というのは冗談で、あらゆる用途を網羅する汎用エンジンではありません。Microsoft MiniEngine のリソース管理や DirectX 12 の初期化の考え方を参考に、エンジンとエディタを自作しながらグラフィックスやゲームエンジンの構造を学ぶことを目的としています。

現在のリポジトリには、エンジン本体に加えてサンプル用の `TestApp` と、実行時に起動するゲーム `ClockworksDisease` が含まれています。

## 主な機能

- DirectX 12 を使った描画基盤
- Entity Component System（ECS）によるゲームオブジェクトと処理の管理
- シーンの登録、切り替え、遷移演出
- メッシュ、スプライト、テキスト、パーティクルなどの描画
- ライティング、シャドウ、ポストエフェクト、レイトレーシング関連の描画処理
- ウィンドウ、入力、オーディオなどのランタイム機能
- ImGui を使った開発・デバッグ向け UI

## 動作環境

- Windows
- Visual Studio と C++ によるデスクトップ開発環境
- Windows SDK
- DirectX 12 に対応した GPU とドライバー

ソリューションは `project/NoEngine.sln` にあります。プロジェクトでは C++23 と x64 構成（Debug、Development、Release）を使用します。ビルドに必要な外部ライブラリは `project/externals` 以下に含まれています。

## ビルドと実行

1. `project/NoEngine.sln` を Visual Studio で開きます。
2. プラットフォームに `x64` を選び、`Debug`、`Development`、`Release` のいずれかを選択します。
3. `NoEngine` をスタートアッププロジェクトにしてビルドし、実行します。

ビルド成果物は `generated/outputs/<構成名>/` 以下に出力されます。初回ビルド時に必要な Visual Studio コンポーネントが不足している場合は、Visual Studio Installer から C++ によるデスクトップ開発と Windows SDK を追加してください。

## ディレクトリ構成

```text
project/
  engine/        エンジン本体
  application/   ゲームとサンプルアプリケーション
  externals/     外部ライブラリ
  NoEngine.sln   Visual Studio ソリューション
generated/       ビルド成果物と中間ファイル
```

## Assetブラウザ

`Folders` の検索欄では、Addressable名（Prefabは表示名）・元ファイル名・パスを検索できます。英字の大小とパス区切りの `/`・`\` を区別せず、空白で区切った複数の語をすべて含む素材を表示します。検索時の対象は、全フォルダまたは選択中のフォルダから切り替えられます。

`Assets` は上部のメニューバーを除く領域全体にアセット一覧を表示します。`表示` メニューでサムネイル／一覧表示、サムネイルサイズ（64〜192）、名前・種類・パスによる昇順／降順を変更できます。`種類` メニューでフィルタを選び、`操作 > Refresh` で一覧を更新します。表示設定と検索条件は起動中のみ保持します。

素材を右クリックすると、テクスチャ・音声のプレビューやAddressable名の変更を別のImGuiウィンドウで開けます。プレビューと名前編集は、一覧で選択する素材を変えても対象を保持します。音声の試聴はプレビュー内の `Play`／`Stop` で操作し、プレビューを閉じるか別の素材に切り替えると停止します。名前編集では空欄・重複名の保存を防ぎます。

右クリックメニューには、エクスプローラーでの表示、元ファイルを開く、Addressable名・パスのコピー、Prefabの編集もあります。フォルダの右クリックにも、エクスプローラーで開く操作とパスのコピーがあります。

`Assets` の素材をInspectorやPrefab編集内のアセット指定欄へドロップして割り当てられます。モデルは `MeshComponent.meshName`、画像はSprite・Material・Dissolve・各Emitterのテクスチャ欄、フォントは `TextComponent.fontName` と `DialogueComponent.fontName` に対応します。会話の `portraitTexture` と `voiceFiles` の各要素、Prefab参照の `prefabPath` にも割り当てられます。音声リストが空の場合は `+ Add` で要素を追加してからドロップします。

指定欄は対応する種類の素材だけを受け付け、通常の文字列欄や会話本文はドロップ対象になりません。欄にカーソルを合わせると受け付ける種類と参照方法を確認できます。ドロップした変更は `Ctrl+Z` でUndo、`Ctrl+Shift+Z` でRedoできます。通常はAddressable名を格納し、Prefab参照には元ファイルパスを格納します。

新しいアセット指定欄はリフレクションに `REFLECT_ASSET_FIELD(field, Texture)` のように登録します。元ファイルパスを使う欄には `REFLECT_ASSET_PATH_FIELD`、パスの配列には `REFLECT_ASSET_PATH_ARRAY_FIELD` を使います。

## TransformRoutineの複数ルート

`TransformRoutineComponent`は、ID付きの`routines`と一つの再生状態を持ちます。Inspectorの`Add Routine`でルートを追加し、`Edit Routine`で編集対象を選びます。`Routine ID`は空欄・重複を避けて入力し、Enterで確定してください。IDを変更した場合は、グラフ側の参照も更新します。

選択したルートの`keyframes`、時間、Easing、補間方式、`loop`を編集できます。シーンビューの経路表示とWaypointギズモも`Edit Routine`の選択に追従します。追加したルートは現在のTransformを最初のWaypointに使い、`loop = false`で作成します。`keyframes`の`+ Add`で移動先を追加してください。ルートの追加・削除や各値の編集はUndo/Redoに対応します。

グラフの`Activate Mover`には、`Target Entity`と`Routine ID`を設定します。例えば、一度目の会話では`to_gate`、次の会話では`to_shop`を指定します。対象には`TransformComponent`、`TransformRoutineComponent`、`MoverComponent`を付け、`MoverSystem`と`TransformRoutineSystem`を有効にしてください。`Routine ID`を省略する既存グラフは、Inspectorの`Default Routine`を再生します。`startOnAwake`もこの既定ルートを使用します。

同時に再生するルートは一つです。再生中に別の`Activate Mover`が来ると、指定ルートを先頭から再生します。存在しないIDや重複IDの指定はログを出して無視します。非ループの移動完了まで後続Actionを待つため、会話後の移動には`loop = false`を使用してください。座標は従来どおり親基準の絶対値なので、連続するルートの終点と始点を合わせます。

従来の単一`keyframes`形式のシーン・Prefabは、読み込み時にIDが`default`のルートへ移行し、次回保存時に複数ルート形式で保存します。`TransformRoutineComponent2D`は単一ルート形式です。

## エディタのグリッド表示

Debug / Development 構成では、シーンビューにグリッドを表示します。`Grid` ウィンドウで表示の切り替え、XZ / XY / YZ 平面、基準位置、セル間隔、主線間隔、ピクセル単位の線幅、色、描画範囲、フェードを調整できます。

`Adaptive spacing` は遠方の表示を滑らかに粗くします。配置用スナップの値は変更しません。通常は地形の深度で隠れ、`Overlay through geometry` で透過表示に切り替えられます。再生中は初期設定で非表示です。設定は起動中のみ保持し、ファイルへの保存は行いません。

## 3Dアニメーションの再生操作

`AnimatorComponent`は自動再生・ループが既定です。Inspectorの`loop`をオフにするか、`Play(name, false)`で単発再生します。単発再生の完了後は最後のポーズを保持し、`IsFinished()`が`true`になります。この状態は再生・再スタート・停止・シーク・別クリップへの切り替えまで保持します。

```cpp
animator->Play("Attack", false); // 同じ名前でも先頭から単発再生
if (animator->IsFinished()) {
    animator->Play("Idle");     // ループ再生に切り替え
}
```

| API | 動作 |
| --- | --- |
| `Play(name, shouldLoop = true)` | 名前で指定して再生。別クリップへの切り替えは`blendDuration`で補間。同じクリップは先頭から再生 |
| `Play()` / `Restart()` | 現在のクリップを先頭から再生。`loop`の設定は保持し、ブレンドを解除 |
| `Pause()` / `Resume()` | 時刻とブレンドを一時停止／再開。`Resume()`は一時停止中だけ有効 |
| `Stop()` | 選択中のクリップの先頭ポーズへ戻して停止。ブレンドと終了状態を解除 |
| `Seek(seconds)` | 秒単位で移動。範囲外は端に補正し、ブレンドを解除。再生・一時停止・停止の状態は保持。完了後のシークは一時停止へ移行 |
| `IsPlaying()` / `IsPaused()` / `IsFinished()` | 現在の再生・一時停止・単発完了状態を取得 |
| `GetDuration()` / `GetNormalizedTime()` | 長さを秒で取得／再生位置を0〜1で取得 |

`Play`・`Restart`・`Seek`・`Resume`は成功時に`true`を返します。不明な名前や未ロードのクリップなどで失敗した場合は状態を変更しません。シークへのNaNや無限大の指定も無視します。ポーズの表示には次回の`AnimationSystem`更新が必要で、一時停止・停止中も更新を通すことでシーク先や先頭ポーズを反映します。

毎フレームの状態選択には`SetAnimation(name)`を使います。同じ名前なら再スタートせず、単発再生の完了状態も保持します。`animationSpeedMagnification`が負の場合、`Play`と`Restart`は終端から逆再生し、単発再生は先頭で完了します。モデルを差し替えると再生状態はリセットされます。

## アニメーションイベント

`AnimatorComponent.animationEvents`に、クリップ名`animationName`・時刻`time`（秒）・イベント名`name`・任意の文字列`parameter`を登録します。Inspectorの配列編集から追加でき、シーン／Prefabに保存されます。クリップ名は`GetAnimationNames()`で取得する名前を使います。同じモデルでもイベント設定と発火状態はAnimatorごとに独立します。

モデルの読み込み後に、一度だけコードで登録することもできます。`AddEventMarker`は、名前が見つからない、イベント名が空、時刻がクリップの範囲外・NaN・無限大の場合に`false`を返し、設定を変更しません。Inspectorで不正な値を指定したマーカーは再生時に無視します。

```cpp
// モデルの読み込み後、初期設定として登録する。
animator->AddEventMarker("Attack", 0.25f, "Hit", "right_hand");

// 攻撃開始時に単発再生する。
animator->Play("Attack", false);

// AnimationSystemより後に動くゲーム側のSystemで受け取る。
for (const auto& event : animator->ConsumeEvents()) {
    if (event.name == "Hit") {
        LogInfo("Attack hit marker: " + event.parameter);
    }
}
```

発火イベントにはクリップ名・マーカー時刻・イベント名・任意文字列と、逆再生かどうかの`reverse`が含まれます。`ConsumeEvents()`は一覧を返して消費します。読み取りのみの`GetTriggeredEvents()`は消費せず、複数のSystemで同じ一覧を確認できます。一覧は次回の`AnimationSystem`更新でクリアされるため、受信側のSystemをその後に配置してください。受信処理中に`Play`や`Stop`を呼ぶ場合は、先に`ConsumeEvents()`で取り出します。

通常は直前の時刻を除き、到達した時刻までのマーカーを再生方向の順に発火します。同時刻のマーカーは登録順です。再生開始時のマーカーは最初の更新で一度発火します。ループでは周回した回数分を検出し、正再生の境界は終端→先頭、逆再生は先頭→終端の順になります。ブレンド中は切り替え先クリップのイベントだけを発火します。

一時停止・停止・単発再生の完了後には発火しません。`Seek`は移動した区間のイベントを発火せず、既存の発火一覧もクリアします。`Play`・`Restart`・別クリップへの切り替えは開始時のイベントを再び有効にします。`Stop`も既存の一覧をクリアします。

非常に大きい経過時間による大量発火は、1更新につき先頭1024件まで保持します。超過数は`GetDroppedEventCount()`で取得でき、次回更新でリセットされます。数え切れない場合は`uint64_t`の最大値になります。

## アプリケーションの構成

ゲームアプリケーションは `NoEngine::IGameApp` を継承して作成します。`Startup()` でシーンを登録し、起動時のシーンを選びます。各シーンは `NoEngine::Scene::IScene` を継承し、`Setup()` でエンティティやコンポーネントを用意してシステムを追加します。

```cpp
void MyGame::Startup() {
    RegisterScene("Main", [] {
        return std::make_unique<MainScene>();
    });
    ChangeScene("Main");
}
```

ECS のシステムでは、必要なコンポーネントを持つエンティティを `Registry::View` で取得して処理できます。

```cpp
auto view = registry.View<NoEngine::Component::TransformComponent,
                          NoEngine::Component::VelocityComponent>();
for (auto entity : view) {
    auto* transform = registry.GetComponent<NoEngine::Component::TransformComponent>(entity);
    auto* velocity = registry.GetComponent<NoEngine::Component::VelocityComponent>(entity);
    transform->translate += velocity->linear;
}
```

## ライセンスと外部ライブラリ

外部ライブラリは `project/externals` 以下に配置されています。各ライブラリのライセンスと利用条件は、それぞれの配布物に含まれるライセンス文書を確認してください。

## Render Passesエディタ

左の一覧で実行順と編集対象を選び、右の詳細で設定を変更します。追加・削除・上下移動・ドラッグによる移動と設定変更は、共通のUndo/Redo履歴（Ctrl+Z / Ctrl+Shift+Z）に入ります。スライダーのドラッグは一回の操作として扱います。

- **適用**: 検証済みの編集内容を描画へ反映します。ファイルには保存しません。
- **保存**: 編集内容をシーンJSONへ保存します。描画への反映は適用で行います。
- **保存状態に戻す**: 編集内容を最後の保存状態へ戻します。描画も戻す場合は適用を押します。
- **ファイルから再読込**: 確認後にディスクから再読込します。通常のシーン切替やパネルを閉じる操作では、未保存の編集はメモリ上に保持されます。アプリ終了前には保存してください。

リソースは詳細内の「詳細設定：リソースとカメラ」から選びます。カラー／深度／UAV、フォーマット、配列数が互換なものだけを候補に表示します。生成順序、基盤処理の依存、同一リソースへの読み書きなどのエラーがあると適用と保存は無効になります。下部の診断から該当パスを選択できます。

保存はシーンの他の項目を保持し、一時ファイルへの書き込み成功後に置換します。外部でrenderPassesが変更された場合は上書きせず、ファイルから再読込を案内します。