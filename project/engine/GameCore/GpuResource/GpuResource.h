#pragma once
#include "../Graphics/GraphicsDevice.h"

namespace NoEngine {
/// <summary>
/// GpuResource管理の基底クラス
/// </summary>
class GpuResource {
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	GpuResource();

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="resource">リソースのポインタ</param>
	/// <param name="CurrentState">現在のリソースステート</param>
	GpuResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES CurrentState);

	virtual ~GpuResource() { Destroy(); }

	virtual void Destroy();

	ID3D12Resource* GetResource() const { return resource_.Get(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return gpuVirtualAddress_; }
	void SetName(const std::wstring& name) { name_ = name; };

	ID3D12Resource* operator->() { return resource_.Get(); }
	const ID3D12Resource* operator->() const { return resource_.Get(); }

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	D3D12_RESOURCE_STATES usageState_;
	D3D12_RESOURCE_STATES transitioningState_;
	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress_;
	std::wstring name_;

	// リソースが変更されたタイミングを識別するために使用します。それによって記述子をコピーするなどの処理が可能です。
	uint32_t versionID_= 0;
};
}

