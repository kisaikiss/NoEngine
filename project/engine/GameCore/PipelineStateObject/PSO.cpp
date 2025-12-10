#include "PSO.h"

namespace NoEngine {
std::unordered_map< uint64_t, Microsoft::WRL::ComPtr<ID3D12PipelineState> > PSO::sGraphicsPSOHashMap;
std::unordered_map< uint64_t, Microsoft::WRL::ComPtr<ID3D12PipelineState> > PSO::sComputePSOHashMap;

void PSO::DestroyAll(void) {
	sGraphicsPSOHashMap.clear();
	sComputePSOHashMap.clear();
}

}