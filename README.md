# NoEngine
<img width="256" height="256" alt="noicon" src="https://github.com/user-attachments/assets/5727812f-eb1b-43ab-98a6-ab510aa8749c" />

> [!NOTE]
> | Branch | Debug | Development | Release |
> | ------------- | ------------- | ------------- | ------------- |
> | master |[![DebugBuild](https://github.com/kisaikiss/NoEngine/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/kisaikiss/NoEngine/actions/workflows/DebugBuild.yml)|[![DevelopmentBuild](https://github.com/kisaikiss/NoEngine/actions/workflows/DevelopmentBuild.yml/badge.svg)](https://github.com/kisaikiss/NoEngine/actions/workflows/DevelopmentBuild.yml)| [![ReleaseBuild](https://github.com/kisaikiss/NoEngine/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/kisaikiss/NoEngine/actions/workflows/ReleaseBuild.yml) |
  
## NoEngineについて
NoEngineは2025年11月3日にて本格的に制作が開始されたゲームエンジンです。  
「できないことなし」がコンセプトです。嘘です。  
Microsoft MiniEngineのリソース管理やDirectX12の初期化部分などを再現し、  
それを元にエディタの制作を行うことによってDirectX12に対する理解を深めるのを目的としたゲームエンジンです。

### NoEngine独自の機能
#### ECS(Entity Component System)  
NoEngineでは効率的なデータ配置が出来るECSを採用しています。  
namespace ECS内にあるクラス群がECSに必要なクラスです。そのうちのRegistryクラスがEntityとComponentの管理を行っています。
Registryのメンバ関数であるView()を使用してSystemを構築します。
  
* 以下の様に使用。指定した複数のコンポーネントを持ったエンティティだけを取得できます。  
auto view = registry.View<Transform, Velocity>();  
for(auto entity : view) {  
  　 　auto* transform = registry.GetComponent< Transform >(entity);  
 　　 auto* velocity = registry.GetComponent< Velocity >(entity);  
　 　 transform->translate += velocity->v;  
}

* システムの適応方法  
システムの適応はシーンごとに行います。  
ISceneを継承したクラスのSetup()内でAddSystem(std::make_unique<TestSystem>());のように書くことでシステムが適応されます。  
  
#### ゲームアプリケーション
新たなゲームはIGameAppを継承して作成します。  
IGameApp::Startup()をオーバーライドした関数内でシーンの登録と初期シーンの生成を行います。  
  
// シーンの登録  
RegisterScene("TestScene", []() {
	return std::make_unique< TestScene >();
	});   
// 初期シーンの生成  
ChangeScene("TestScene");
