// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_root_signature.h"
#include "d3d12_command_list.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/out_ptr.h"

up::d3d12::RootSignatureD3D12::RootSignatureD3D12()  {
}

up::d3d12::RootSignatureD3D12::~RootSignatureD3D12() {
};


static inline void Init_1_1(
    _Out_ D3D12_VERSIONED_ROOT_SIGNATURE_DESC& desc,
    UINT numParameters,
    _In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER1* _pParameters,
    UINT numStaticSamplers = 0,
    _In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept {
    desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    desc.Desc_1_1.NumParameters = numParameters;
    desc.Desc_1_1.pParameters = _pParameters;
    desc.Desc_1_1.NumStaticSamplers = numStaticSamplers;
    desc.Desc_1_1.pStaticSamplers = _pStaticSamplers;
    desc.Desc_1_1.Flags = flags;
}

static inline void InitRootSignatureDesc(
    _Out_ D3D12_ROOT_SIGNATURE_DESC& desc,
    UINT numParameters,
    _In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
    UINT numStaticSamplers = 0,
    _In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept {
    desc.NumParameters = numParameters;
    desc.pParameters = _pParameters;
    desc.NumStaticSamplers = numStaticSamplers;
    desc.pStaticSamplers = _pStaticSamplers;
    desc.Flags = flags;
}

static std::vector<up::rc<up::d3d12::RootSignatureD3D12>> s_RootSignatures;

auto up::d3d12::RootSignatureD3D12::initializeSignatures(ID3D12Device* device) -> bool {
    s_RootSignatures.resize(RootSignatureType::Max);

    SignatureDesc desc;
    desc._range.resize(2);
    desc._range[0].initRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    desc._range[1].initRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);

    // InitAsConstantBufferView(parameters[RootParamIndex::ConstantBuffer], 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE);
    desc._params.resize(3);
    desc._params[0].initAsConstants(16, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    desc._params[1].initAsDescriptorTable(1, &desc._range[0]._range, D3D12_SHADER_VISIBILITY_PIXEL);
    desc._params[2].initAsDescriptorTable(1, &desc._range[1]._range, D3D12_SHADER_VISIBILITY_PIXEL);

    s_RootSignatures[RootSignatureType::ImGui] = RootSignatureD3D12::createRootSignature(device, desc);
    s_RootSignatures[RootSignatureType::Model] = RootSignatureD3D12::createRootSignature(device, desc);

    return true;
 };

auto up::d3d12::RootSignatureD3D12::destroySignatures() -> bool {
    s_RootSignatures.clear();
    return true;
 };

auto up::d3d12::RootSignatureD3D12::getRootSignature(RootSignatureType type) -> rc<up::d3d12::RootSignatureD3D12> {
    return s_RootSignatures[type];
};

//------------------------------------------------------------------------------------------------
// D3D12 exports a new method for serializing root signatures in the Windows 10 Anniversary Update.
// To help enable root signature 1.1 features when they are available and not require maintaining
// two code paths for building root signatures, this helper method reconstructs a 1.0 signature when
// 1.1 is not supported.
inline HRESULT D3DX12SerializeVersionedRootSignature(
    _In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc,
    D3D_ROOT_SIGNATURE_VERSION MaxVersion,
    _Outptr_ ID3DBlob** ppBlob,
    _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob) noexcept {
    if (ppErrorBlob != nullptr) {
        *ppErrorBlob = nullptr;
    }

    switch (MaxVersion) {
        case D3D_ROOT_SIGNATURE_VERSION_1_0:
            switch (pRootSignatureDesc->Version) {
                case D3D_ROOT_SIGNATURE_VERSION_1_0:
                    return D3D12SerializeRootSignature(
                        &pRootSignatureDesc->Desc_1_0,
                        D3D_ROOT_SIGNATURE_VERSION_1,
                        ppBlob,
                        ppErrorBlob);

                case D3D_ROOT_SIGNATURE_VERSION_1_1: {
                    HRESULT hr = S_OK;
                    const D3D12_ROOT_SIGNATURE_DESC1& desc_1_1 = pRootSignatureDesc->Desc_1_1;

                    const SIZE_T ParametersSize = sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters;
                    void* pParameters = (ParametersSize > 0) ? HeapAlloc(GetProcessHeap(), 0, ParametersSize) : nullptr;
                    if (ParametersSize > 0 && pParameters == nullptr) {
                        hr = E_OUTOFMEMORY;
                    }
                    auto pParameters_1_0 = static_cast<D3D12_ROOT_PARAMETER*>(pParameters);

                    if (SUCCEEDED(hr)) {
                        for (UINT n = 0; n < desc_1_1.NumParameters; n++) {
                            __analysis_assume(ParametersSize == sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters);
                            pParameters_1_0[n].ParameterType = desc_1_1.pParameters[n].ParameterType;
                            pParameters_1_0[n].ShaderVisibility = desc_1_1.pParameters[n].ShaderVisibility;

                            switch (desc_1_1.pParameters[n].ParameterType) {
                                case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
                                    pParameters_1_0[n].Constants.Num32BitValues =
                                        desc_1_1.pParameters[n].Constants.Num32BitValues;
                                    pParameters_1_0[n].Constants.RegisterSpace =
                                        desc_1_1.pParameters[n].Constants.RegisterSpace;
                                    pParameters_1_0[n].Constants.ShaderRegister =
                                        desc_1_1.pParameters[n].Constants.ShaderRegister;
                                    break;

                                case D3D12_ROOT_PARAMETER_TYPE_CBV:
                                case D3D12_ROOT_PARAMETER_TYPE_SRV:
                                case D3D12_ROOT_PARAMETER_TYPE_UAV:
                                    pParameters_1_0[n].Descriptor.RegisterSpace =
                                        desc_1_1.pParameters[n].Descriptor.RegisterSpace;
                                    pParameters_1_0[n].Descriptor.ShaderRegister =
                                        desc_1_1.pParameters[n].Descriptor.ShaderRegister;
                                    break;

                                case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                                    const D3D12_ROOT_DESCRIPTOR_TABLE1& table_1_1 =
                                        desc_1_1.pParameters[n].DescriptorTable;

                                    const SIZE_T DescriptorRangesSize =
                                        sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges;
                                    void* pDescriptorRanges = (DescriptorRangesSize > 0 && SUCCEEDED(hr))
                                        ? HeapAlloc(GetProcessHeap(), 0, DescriptorRangesSize)
                                        : nullptr;
                                    if (DescriptorRangesSize > 0 && pDescriptorRanges == nullptr) {
                                        hr = E_OUTOFMEMORY;
                                    }
                                    auto pDescriptorRanges_1_0 =
                                        static_cast<D3D12_DESCRIPTOR_RANGE*>(pDescriptorRanges);

                                    if (SUCCEEDED(hr)) {
                                        for (UINT x = 0; x < table_1_1.NumDescriptorRanges; x++) {
                                            __analysis_assume(
                                                DescriptorRangesSize ==
                                                sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges);
                                            pDescriptorRanges_1_0[x].BaseShaderRegister =
                                                table_1_1.pDescriptorRanges[x].BaseShaderRegister;
                                            pDescriptorRanges_1_0[x].NumDescriptors =
                                                table_1_1.pDescriptorRanges[x].NumDescriptors;
                                            pDescriptorRanges_1_0[x].OffsetInDescriptorsFromTableStart =
                                                table_1_1.pDescriptorRanges[x].OffsetInDescriptorsFromTableStart;
                                            pDescriptorRanges_1_0[x].RangeType =
                                                table_1_1.pDescriptorRanges[x].RangeType;
                                            pDescriptorRanges_1_0[x].RegisterSpace =
                                                table_1_1.pDescriptorRanges[x].RegisterSpace;
                                        }
                                    }

                                    D3D12_ROOT_DESCRIPTOR_TABLE& table_1_0 = pParameters_1_0[n].DescriptorTable;
                                    table_1_0.NumDescriptorRanges = table_1_1.NumDescriptorRanges;
                                    table_1_0.pDescriptorRanges = pDescriptorRanges_1_0;
                            }
                        }
                    }

                    if (SUCCEEDED(hr)) {
                        D3D12_ROOT_SIGNATURE_DESC desc_1_0;
                        InitRootSignatureDesc(
                            desc_1_0,
                            desc_1_1.NumParameters,
                            pParameters_1_0,
                            desc_1_1.NumStaticSamplers,
                            desc_1_1.pStaticSamplers,
                            desc_1_1.Flags);
                        hr = D3D12SerializeRootSignature(&desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);
                    }

                    if (pParameters) {
                        for (UINT n = 0; n < desc_1_1.NumParameters; n++) {
                            if (desc_1_1.pParameters[n].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
                                HeapFree(
                                    GetProcessHeap(),
                                    0,
                                    reinterpret_cast<void*>(const_cast<D3D12_DESCRIPTOR_RANGE*>(
                                        pParameters_1_0[n].DescriptorTable.pDescriptorRanges)));
                            }
                        }
                        HeapFree(GetProcessHeap(), 0, pParameters);
                    }
                    return hr;
                }
            }
            break;

        case D3D_ROOT_SIGNATURE_VERSION_1_1:
            return D3D12SerializeVersionedRootSignature(pRootSignatureDesc, ppBlob, ppErrorBlob);
    }

    return E_INVALIDARG;
}

auto up::d3d12::RootSignatureD3D12::createRootSignature(ID3D12Device* device, const SignatureDesc& desc) -> rc<RootSignatureD3D12> {
    auto signature = new_shared<RootSignatureD3D12>();
    signature->create(device, desc);
    return signature;
}

auto up::d3d12::RootSignatureD3D12::create(ID3D12Device* device, const SignatureDesc& desc) -> bool {
    ID3DRootSignaturePtr root;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned
    // will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    
    up::com_ptr<ID3DBlob> signature;
    up::com_ptr<ID3DBlob> error;
 
    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = 0.0f;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    Init_1_1(rootSignatureDesc, static_cast<uint32>(desc._params.size()), (const D3D12_ROOT_PARAMETER1*)desc._params.data(), 0, &sampler, rootSignatureFlags);

   if (FAILED(D3DX12SerializeVersionedRootSignature(
            &rootSignatureDesc,
            featureData.HighestVersion,
            out_ptr(signature),
            out_ptr(error)))) {

       auto msg = static_cast<const char*>(error->GetBufferPointer());
       UP_ASSERT(0, msg);
       return false; 
   };
    device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        __uuidof(ID3D12RootSignature),
        out_ptr(_signature));

    UP_ASSERT(signature.get());
    return true;
}
