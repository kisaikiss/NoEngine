#pragma once
#include "PSO.h"
namespace NoEngine {
class ComputePSO : public PSO {
	friend class CommandContext;
public:
	ComputePSO(std::wstring name = L"Unnamed Compute PSO");

	void SetComputeShader(const void* binary, size_t size) { psoDesc_.CS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }
	void SetComputeShader(const D3D12_SHADER_BYTECODE& binary) { psoDesc_.CS = binary; }

	void Finalize();
private:
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc_;
};
}
