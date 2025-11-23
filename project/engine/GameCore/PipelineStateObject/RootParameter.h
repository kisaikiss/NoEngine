#pragma once

namespace NoEngine {
class RootParameter {
	friend class RootSignature;
public:
	RootParameter() {
		rootParameter_.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
	}

	~RootParameter() {
		Clear();
	}

	void Clear() {
		if (rootParameter_.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			delete[] rootParameter_.DescriptorTable.pDescriptorRanges;

		rootParameter_.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
	}

    void InitAsConstants(UINT Register, UINT numDwords, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, UINT space = 0) {
        rootParameter_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParameter_.ShaderVisibility = visibility;
        rootParameter_.Constants.Num32BitValues = numDwords;
        rootParameter_.Constants.ShaderRegister = Register;
        rootParameter_.Constants.RegisterSpace = space;
    }

    void InitAsConstantBuffer(UINT Register, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, UINT space = 0) {
        rootParameter_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameter_.ShaderVisibility = visibility;
        rootParameter_.Descriptor.ShaderRegister = Register;
        rootParameter_.Descriptor.RegisterSpace = space;
    }

    void InitAsBufferSRV(UINT Register, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, UINT space = 0) {
        rootParameter_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rootParameter_.ShaderVisibility = visibility;
        rootParameter_.Descriptor.ShaderRegister = Register;
        rootParameter_.Descriptor.RegisterSpace = space;
    }

    void InitAsBufferUAV(UINT Register, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, UINT space = 0) {
        rootParameter_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
        rootParameter_.ShaderVisibility = visibility;
        rootParameter_.Descriptor.ShaderRegister = Register;
        rootParameter_.Descriptor.RegisterSpace = space;
    }

    void InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT Register, UINT count, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, UINT space = 0) {
        InitAsDescriptorTable(1, visibility);
        SetTableRange(0, type, Register, count, space);
    }

    void InitAsDescriptorTable(UINT rangeCount, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
        rootParameter_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameter_.ShaderVisibility = visibility;
        rootParameter_.DescriptorTable.NumDescriptorRanges = rangeCount;
        rootParameter_.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[rangeCount];
    }

    void SetTableRange(UINT rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE type, UINT Register, UINT count, UINT space = 0) {
        D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(rootParameter_.DescriptorTable.pDescriptorRanges + rangeIndex);
        range->RangeType = type;
        range->NumDescriptors = count;
        range->BaseShaderRegister = Register;
        range->RegisterSpace = space;
        range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    }

    const D3D12_ROOT_PARAMETER& operator() (void) const { return rootParameter_; }

protected:
	D3D12_ROOT_PARAMETER rootParameter_;
};
}

