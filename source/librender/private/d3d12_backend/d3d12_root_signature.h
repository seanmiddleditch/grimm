// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_pipeline_state.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/rc.h"
#include "potato/spud/vector.h"

namespace up::d3d12 {

    class CommandListD3D12;
    class DescriptorHeapD3D12;


    enum RootParamType : uint32 { Texture, Sampler, ConstValues, ConstBuffer, RootParamMax };

    struct SignatureParam {
        D3D12_ROOT_PARAMETER1 _parameter;

        void initRootDescriptor(
            _Out_ D3D12_ROOT_DESCRIPTOR1& table,
            UINT shaderRegister,
            UINT registerSpace = 0,
            D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) noexcept {
            table.ShaderRegister = shaderRegister;
            table.RegisterSpace = registerSpace;
            table.Flags = flags;
        }

        void initRootDescriptorTable(
            _Out_ D3D12_ROOT_DESCRIPTOR_TABLE1& rootDescriptorTable,
            UINT numDescriptorRanges,
            _In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* _pDescriptorRanges) noexcept {
            rootDescriptorTable.NumDescriptorRanges = numDescriptorRanges;
            rootDescriptorTable.pDescriptorRanges = _pDescriptorRanges;
        }

        void initRootConstant(
            _Out_ D3D12_ROOT_CONSTANTS& rootConstants,
            UINT num32BitValues,
            UINT shaderRegister,
            UINT registerSpace = 0) noexcept {
            rootConstants.Num32BitValues = num32BitValues;
            rootConstants.ShaderRegister = shaderRegister;
            rootConstants.RegisterSpace = registerSpace;
        }

        void initRootDescriptorTable1(
            _Out_ D3D12_ROOT_DESCRIPTOR_TABLE1& rootDescriptorTable,
            UINT numDescriptorRanges,
            _In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* _pDescriptorRanges) noexcept {
            rootDescriptorTable.NumDescriptorRanges = numDescriptorRanges;
            rootDescriptorTable.pDescriptorRanges = _pDescriptorRanges;
        }
        void initAsDescriptorTable(
            UINT numDescriptorRanges,
            _In_reads_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges,
            D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept {
            _parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            _parameter.ShaderVisibility = visibility;
            initRootDescriptorTable(_parameter.DescriptorTable, numDescriptorRanges, pDescriptorRanges);
        }

        void initAsConstants(
            UINT num32BitValues,
            UINT shaderRegister,
            UINT registerSpace = 0,
            D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept {
            _parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            _parameter.ShaderVisibility = visibility;
            initRootConstant(_parameter.Constants, num32BitValues, shaderRegister, registerSpace);
        }

        void initAsConstantBufferView(
            UINT shaderRegister,
            UINT registerSpace = 0,
            D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
            D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept {
            _parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            _parameter.ShaderVisibility = visibility;
            initRootDescriptor(_parameter.Descriptor, shaderRegister, registerSpace, flags);
        }

        void initAsShaderResourceView(
            UINT shaderRegister,
            UINT registerSpace = 0,
            D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
            D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept {
            _parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            _parameter.ShaderVisibility = visibility;
            initRootDescriptor(_parameter.Descriptor, shaderRegister, registerSpace, flags);
        }

        void initAsUnorderedAccessView(
            UINT shaderRegister,
            UINT registerSpace = 0,
            D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
            D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept {
            _parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            _parameter.ShaderVisibility = visibility;
            initRootDescriptor(_parameter.Descriptor, shaderRegister, registerSpace, flags);
        }
    };

    struct SignatureRange {
        D3D12_DESCRIPTOR_RANGE1 _range;

        void initRange(
            D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
            UINT numDescriptors,
            UINT baseShaderRegister,
            UINT registerSpace = 0,
            D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
            UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) noexcept {
            _range.RangeType = rangeType;
            _range.NumDescriptors = numDescriptors;
            _range.BaseShaderRegister = baseShaderRegister;
            _range.RegisterSpace = registerSpace;
            _range.Flags = flags;
            _range.OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
        }
    };

    struct SignatureDesc {

        vector<SignatureParam> _params;
        vector<SignatureRange> _range;
        uint32 _offsets[RootParamType::RootParamMax];

        void resize(uint32 params) {
            _params.resize(params);
            _range.resize(2);
        }

        void initSamplers(uint32 offset, uint32 count, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
            _range[1].initRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, count, 0, 0);
            _params[offset].initAsDescriptorTable(1, &_range[1]._range, visibility);
            _offsets[RootParamType::Sampler] = offset;
        }
        void initSRVs(uint32 offset, uint32 count, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
            _range[0].initRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, count, 0, 0);
            _params[offset].initAsDescriptorTable(1, &_range[0]._range, visibility);
            _offsets[RootParamType::Texture] = offset;
        }
        void initConstValues(uint32 offset, uint32 count, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
            _params[offset].initAsConstants(count, 0, 0, visibility);
            _offsets[RootParamType::ConstValues] = offset;
        }
        void initConstBuffer(uint32 offset, uint32 reg, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL){
            _params[offset].initAsConstantBufferView(reg, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, visibility);
            _offsets[RootParamType::ConstBuffer] = offset;
        } 
    };

    class RootSignatureD3D12 : public shared<RootSignatureD3D12> {
    public:

        explicit RootSignatureD3D12();
        virtual ~RootSignatureD3D12();

        // placeholder management of different root managers
        // @todo: come up with better way of managing root signatures during runtime
        static bool initializeSignatures(ID3D12Device* device);
        static bool destroySignatures();
        static rc<RootSignatureD3D12> getRootSignature(RootSignatureType type);

        static rc<RootSignatureD3D12> createRootSignature(ID3D12Device* device, const SignatureDesc& desc);

        ID3D12RootSignature* signature() const { return _signature.get(); }
        uint32 getRootOffset(RootParamType type) const { return _offsetMap[type]; }

    private:

        bool create(ID3D12Device* device, const SignatureDesc& desc);

    private:
        uint32 _offsetMap[RootParamType::RootParamMax];
        ID3DRootSignaturePtr _signature;
    };
} // namespace up::d3d12
