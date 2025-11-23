#include "GpuResource.h"

#include "engine/Debug/Logger/Log.h"

namespace NoEngine {
GpuResource::GpuResource() : 
gpuVirtualAddress_(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
usageState_(D3D12_RESOURCE_STATE_COMMON),
transitionState_((D3D12_RESOURCE_STATES)-1) {
}

GpuResource::GpuResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES CurrentState) :
	gpuVirtualAddress_(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
	resource_(resource),
	usageState_(CurrentState),
	transitionState_((D3D12_RESOURCE_STATES)-1) {
}


void GpuResource::Destroy() {
	resource_ = nullptr;
	gpuVirtualAddress_ = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	++versionID_;
	Log::DebugPrint("GpuResource Destroyed", VerbosityLevel::kDebug);
	}
}