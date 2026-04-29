#pragma once
#include "engine/Utilities/NonCopyable.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"
#include "engine/Runtime/Command/GraphicsContext.h"
#include "LightForGPU.h"

namespace NoEngine {
/// <summary>
/// 描画に必要な情報を描画Passから他の描画Passへ受け渡しするためのクラス
/// </summary>
class RenderContext : NonCopyable {
public:
	/// <summary>
	/// それぞれのライトの数
	/// </summary>
	struct LightNums {
		uint32_t directionalLightNum = 0;
		uint32_t pointLightNum = 0;
		uint32_t spotLightNum = 0;
	};

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

	D3D12_CPU_DESCRIPTOR_HANDLE GetDirectionalLightSRV() { return directionalLightBuffer_.GetSRV(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetPointLightSRV() { return pointLightBuffer_.GetSRV(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetSpotLightSRV() { return spotLightBuffer_.GetSRV(); }

	Microsoft::WRL::ComPtr<ID3D12Resource>& GetRaytraceInstanceBuffer() { return instanceBuffer_; }
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetTLAS() { return tlas_; }
private:
	// ライト
	StructuredBuffer directionalLightBuffer_;
	StructuredBuffer pointLightBuffer_;
	StructuredBuffer spotLightBuffer_;
	LightNums lightNums_;

	// Raytracing
	Microsoft::WRL::ComPtr<ID3D12Resource> tlas_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer_;
};
}

