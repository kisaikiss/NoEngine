#pragma once
#include "engine/Utilities/NonCopyable.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/ECS/Registry.h"
#include "PipelineState/GraphicsPSOManager.h"
#include "PipelineState/RootSignatureManager.h"
#include "PipelineState/StateObjectManager.h"

namespace NoEngine {
/// <summary>
/// 描画に必要な情報を描画Passから他の描画Passへ受け渡しするためのクラス
/// </summary>
class RenderContext : NonCopyable {
public:
	RenderContext();

	void Update(ECS::Registry& registry);

	/// <summary>
	/// それぞれのライトの数
	/// </summary>
	struct LightNums {
		uint32_t directionalLightNum = 0;
		uint32_t pointLightNum = 0;
		uint32_t spotLightNum = 0;
	};

	/// <summary>
	/// グラフィックスのパイプラインステートオブジェクト (PSO) を登録する関数。
	/// </summary>
	/// <param name="name">登録する PSO の名前を表す文字列。</param>
	/// <param name="pso">登録する GraphicsPSO オブジェクトへの参照。</param>
	/// <returns>登録された PSO に対応する uint32_t 型の識別子を返す。</returns>
	uint32_t RegisterGraphicsPSO(const std::string& name, const GraphicsPSO& pso) { return graphicsPSOs_.Register(name, pso); }

	/// <summary>
	/// 名前でルートシグネチャを内部に登録し、その識別子を返す。
	/// </summary>
	/// <param name="name">登録に使用する一意の名前。</param>
	/// <param name="rootSignature">登録する RootSignature オブジェクトへの参照。</param>
	/// <returns>登録されたルートシグネチャを識別する uint32_t の ID。</returns>
	uint32_t RegisterRootSignature(const std::string& name, RootSignature rootSignature) { return rootSignatures_.Register(name, std::move(rootSignature)); }

	/// <summary>
	/// 名前をキーに StateObjectManager に状態オブジェクトを登録する。
	/// </summary>
	/// <param name="name">登録するオブジェクトの識別名。</param>
	/// <param name="entry">登録する StateObjectManager::Entry の情報。</param>
	void RegisterStateObject(const std::string& name, const StateObject& entry) { stateObjects_.Register(name, entry); }

	/// <summary>
	/// 方向ライトをセットします
	/// </summary>
	/// <param name="gfx">描画用コマンドリストのラッパークラス</param>
	/// <param name="directionalLightUpload">方向ライトのUploadバッファ</param>
	/// <param name="directionalLightNum">方向ライトの数</param>
	void SetDirectionalLight(GraphicsContext& gfx, UploadBuffer& directionalLightUpload, uint32_t directionalLightNum);

	/// <summary>
	/// ポイントライトをセットします。
	/// </summary>
	/// <param name="gfx">描画用コマンドリストのラッパークラス</param>
	/// <param name="pointLightUpload">ポイントライトのUploadバッファ</param>
	/// <param name="directionalLightNum">ポイントライトの数</param>
	void SetPointLight(GraphicsContext& gfx, UploadBuffer& pointLightUpload, uint32_t pointLightNum);

	/// <summary>
	/// スポットライトをセットします。
	/// </summary>
	/// <param name="gfx">描画用コマンドリストのラッパークラス</param>
	/// <param name="spotLightUpload">スポットライトののUploadバッファ</param>
	/// <param name="spotLightNum">スポットライトの数</param>
	void SetSpotLight(GraphicsContext& gfx, UploadBuffer& spotLightUpload, uint32_t spotLightNum);

	/// <summary>
	/// レイトレーシング用のインスタンスバッファを作成する。
	/// </summary>
	/// <param name="uploadHeap">アップロード用ヒープのプロパティ。バッファ作成に使用するヒープ情報を参照で受け取る。</param>
	/// <param name="instDesc">作成するインスタンスバッファのリソース記述（サイズや使用法など）を参照で受け取る。</param>
	void CreateRaytraceInstanceBuffer(D3D12_HEAP_PROPERTIES& uploadHeap, CD3DX12_RESOURCE_DESC& instDesc);

	/// <summary>
	/// 指定されたリソース記述に基づいてトップレベルアクセラレーション構造（TLAS）を作成する。
	/// </summary>
	/// <param name="tlasDesc">TLAS作成に使用するCD3DX12_RESOURCE_DESC構造体への参照。</param>
	void CreateTLAS(CD3DX12_RESOURCE_DESC& tlasDesc);

	/// <summary>
	/// それぞれのライトの数を取得します。
	/// </summary>
	/// <returns>それぞれのライトの数</returns>
	const LightNums* GetLightNums() { return &lightNums_; }


	GraphicsPSO& GetGraphicsPSO(const std::string& name) { return graphicsPSOs_.Get(name); }
	GraphicsPSO& GetGraphicsPSO(uint32_t index) { return graphicsPSOs_.Get(index); }
	RootSignature& GetRootSignature(const std::string& name) { return rootSignatures_.Get(name); }
	RootSignature& GetRootSignature(uint32_t index) { return rootSignatures_.Get(index); }
	StateObject& GetStateObject(const std::string& name) { return stateObjects_.Get(name); }
	uint32_t GetPSOID(const std::string& name) { return graphicsPSOs_.GetID(name); }
	uint32_t GetRootSignatureID(const std::string& name) { return rootSignatures_.GetID(name); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDirectionalLightSRV() { return directionalLightBuffer_.GetSRV(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetPointLightSRV() { return pointLightBuffer_.GetSRV(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetSpotLightSRV() { return spotLightBuffer_.GetSRV(); }
	Component::CameraComponent* GetCamera() { return camera_; }
	Component::CameraComponent* GetDebugCamera() { return debugCamera_; }
	bool IsInitialized() { return isInitialized_; }

	StructuredBuffer& GetDirectionalLightBuffer() { return directionalLightBuffer_; }

	Microsoft::WRL::ComPtr<ID3D12Resource>& GetRaytraceInstanceBuffer() { return instanceBuffer_; }
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetTLAS() { return tlas_; }
private:
	// 初期化済みか
	bool isInitialized_ = false;

	// ライト
	StructuredBuffer directionalLightBuffer_;
	StructuredBuffer pointLightBuffer_;
	StructuredBuffer spotLightBuffer_;
	LightNums lightNums_;

	// カメラ
	Component::CameraComponent* camera_;
	Component::CameraComponent* debugCamera_;
	std::unordered_map<std::string, Component::CameraComponent*> cameras_;

	// 描画Pipeline
	GraphicsPSOManager graphicsPSOs_;
	RootSignatureManager rootSignatures_;
	StateObjectManager stateObjects_;

	// Raytracing
	Microsoft::WRL::ComPtr<ID3D12Resource> tlas_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer_;
};
}

