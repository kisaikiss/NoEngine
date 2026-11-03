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

## エディタのグリッド表示

Debug / Development 構成では、シーンビューにグリッドを表示します。`Grid` ウィンドウで表示の切り替え、XZ / XY / YZ 平面、基準位置、セル間隔、主線間隔、ピクセル単位の線幅、色、描画範囲、フェードを調整できます。

`Adaptive spacing` は遠方の表示を滑らかに粗くします。配置用スナップの値は変更しません。通常は地形の深度で隠れ、`Overlay through geometry` で透過表示に切り替えられます。再生中は初期設定で非表示です。設定は起動中のみ保持し、ファイルへの保存は行いません。

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
