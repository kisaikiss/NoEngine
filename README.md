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
