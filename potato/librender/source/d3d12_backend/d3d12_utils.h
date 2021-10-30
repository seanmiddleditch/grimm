// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_common.h"

#include <d3d12.h>

namespace up::d3d12 {

    //*********************************************************
    //
    // Copyright (c) Microsoft. All rights reserved.
    // This code is licensed under the MIT License (MIT).
    // THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
    // ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
    // IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
    // PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
    //
    //*********************************************************

    // Assign a name to the object to aid with debugging.
#if defined(_DEBUG) || defined(DBG)
    inline void SetName(ID3D12Object* pObject, LPCWSTR name) { pObject->SetName(name); }
    inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index) {
        WCHAR fullName[50];
        if (swprintf_s(fullName, L"%s[%u]", name, index) > 0) {
            pObject->SetName(fullName);
        }
    }
#else
    inline void SetName(ID3D12Object*, LPCWSTR) {}
    inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT) {}
#endif


    //------------------------------------------------------------------------------------------------
    // Row-by-row memcpy
    inline void MemcpySubresource(
        _In_ const D3D12_MEMCPY_DEST* pDest,
        _In_ const D3D12_SUBRESOURCE_DATA* pSrc,
        SIZE_T RowSizeInBytes,
        UINT NumRows,
        UINT NumSlices) noexcept {
        for (UINT z = 0; z < NumSlices; ++z) {
            auto pDestSlice = static_cast<BYTE*>(pDest->pData) + pDest->SlicePitch * z;
            auto pSrcSlice = static_cast<const BYTE*>(pSrc->pData) + pSrc->SlicePitch * LONG_PTR(z);
            for (UINT y = 0; y < NumRows; ++y) {
                memcpy(pDestSlice + pDest->RowPitch * y, pSrcSlice + pSrc->RowPitch * LONG_PTR(y), RowSizeInBytes);
            }
        }
    }

    //------------------------------------------------------------------------------------------------
    // Returns required size of a buffer to be used for data upload
    inline UINT64 GetRequiredIntermediateSize(
        _In_ ID3D12Resource* pDestinationResource,
        _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
        _In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources) noexcept {
        auto Desc = pDestinationResource->GetDesc();
        UINT64 RequiredSize = 0;

        ID3D12Device* pDevice = nullptr;
        pDestinationResource->GetDevice(IID_ID3D12Device, reinterpret_cast<void**>(&pDevice));
        pDevice->GetCopyableFootprints(
            &Desc,
            FirstSubresource,
            NumSubresources,
            0,
            nullptr,
            nullptr,
            nullptr,
            &RequiredSize);
        pDevice->Release();

        return RequiredSize;
    }

    //------------------------------------------------------------------------------------------------
    // All arrays must be populated (e.g. by calling GetCopyableFootprints)
    inline UINT64 UpdateSubresources(
        _In_ ID3D12GraphicsCommandList* pCmdList,
        _In_ ID3D12Resource* pDestinationResource,
        _In_ ID3D12Resource* pIntermediate,
        _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
        _In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources,
        UINT64 RequiredSize,
        _In_reads_(NumSubresources) const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
        _In_reads_(NumSubresources) const UINT* pNumRows,
        _In_reads_(NumSubresources) const UINT64* pRowSizesInBytes,
        _In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* pSrcData) noexcept {
        // Minor validation
        auto IntermediateDesc = pIntermediate->GetDesc();
        auto DestinationDesc = pDestinationResource->GetDesc();
        if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
            IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset || RequiredSize > SIZE_T(-1) ||
            (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&
             (FirstSubresource != 0 || NumSubresources != 1))) {
            return 0;
        }

        BYTE* pData;
        HRESULT hr = pIntermediate->Map(0, nullptr, reinterpret_cast<void**>(&pData));
        if (FAILED(hr)) {
            return 0;
        }

        for (UINT i = 0; i < NumSubresources; ++i) {
            if (pRowSizesInBytes[i] > SIZE_T(-1))
                return 0;
            D3D12_MEMCPY_DEST DestData = {
                pData + pLayouts[i].Offset,
                pLayouts[i].Footprint.RowPitch,
                SIZE_T(pLayouts[i].Footprint.RowPitch) * SIZE_T(pNumRows[i])};
            MemcpySubresource(
                &DestData,
                &pSrcData[i],
                static_cast<SIZE_T>(pRowSizesInBytes[i]),
                pNumRows[i],
                pLayouts[i].Footprint.Depth);
        }
        pIntermediate->Unmap(0, nullptr);

        if (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
            pCmdList->CopyBufferRegion(
                pDestinationResource,
                0,
                pIntermediate,
                pLayouts[0].Offset,
                pLayouts[0].Footprint.Width);
        }
        else {
            for (UINT i = 0; i < NumSubresources; ++i) {
                D3D12_TEXTURE_COPY_LOCATION Dst = {};
                Dst.pResource = pDestinationResource;
                Dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                Dst.PlacedFootprint = {};
                Dst.SubresourceIndex = i + FirstSubresource;
            
                D3D12_TEXTURE_COPY_LOCATION Src = {};
                Src.pResource = pIntermediate;
                Src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                Src.PlacedFootprint = pLayouts[i];
               
                pCmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
            }
        }
        return RequiredSize;
    }

    //------------------------------------------------------------------------------------------------
    // Heap-allocating UpdateSubresources implementation
    inline UINT64 UpdateSubresources(
        _In_ ID3D12Device* pDevice,
        _In_ ID3D12GraphicsCommandList* pCmdList,
        _In_ ID3D12Resource* pDestinationResource,
        _In_ ID3D12Resource* pIntermediate,
        UINT64 IntermediateOffset,
        _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
        _In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources,
        _In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* pSrcData) noexcept {
        UINT64 RequiredSize = 0;
        UINT64 MemToAlloc =
            static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) *
            NumSubresources;
        if (MemToAlloc > SIZE_MAX) {
            return 0;
        }
        void* pMem = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T>(MemToAlloc));
        if (pMem == nullptr) {
            return 0;
        }
        auto pLayouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(pMem);
        UINT64* pRowSizesInBytes = reinterpret_cast<UINT64*>(pLayouts + NumSubresources);
        UINT* pNumRows = reinterpret_cast<UINT*>(pRowSizesInBytes + NumSubresources);

        auto Desc = pDestinationResource->GetDesc();
        pDevice->GetCopyableFootprints(
            &Desc,
            FirstSubresource,
            NumSubresources,
            IntermediateOffset,
            pLayouts,
            pNumRows,
            pRowSizesInBytes,
            &RequiredSize);

        UINT64 Result = UpdateSubresources(
            pCmdList,
            pDestinationResource,
            pIntermediate,
            FirstSubresource,
            NumSubresources,
            RequiredSize,
            pLayouts,
            pNumRows,
            pRowSizesInBytes,
            pSrcData);
        HeapFree(GetProcessHeap(), 0, pMem);
        return Result;
    }

    //------------------------------------------------------------------------------------------------
    // Stack-allocating UpdateSubresources implementation
    template <UINT MaxSubresources>
    inline UINT64 UpdateSubresources(
        _In_ ID3D12GraphicsCommandList* pCmdList,
        _In_ ID3D12Resource* pDestinationResource,
        _In_ ID3D12Resource* pIntermediate,
        UINT64 IntermediateOffset,
        _In_range_(0, MaxSubresources) UINT FirstSubresource,
        _In_range_(1, MaxSubresources - FirstSubresource) UINT NumSubresources,
        _In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* pSrcData) noexcept {
        UINT64 RequiredSize = 0;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts[MaxSubresources];
        UINT NumRows[MaxSubresources];
        UINT64 RowSizesInBytes[MaxSubresources];

        auto Desc = pDestinationResource->GetDesc();
        ID3D12Device* pDevice = nullptr;
        pDestinationResource->GetDevice(IID_ID3D12Device, reinterpret_cast<void**>(&pDevice));
        pDevice->GetCopyableFootprints(
            &Desc,
            FirstSubresource,
            NumSubresources,
            IntermediateOffset,
            Layouts,
            NumRows,
            RowSizesInBytes,
            &RequiredSize);
        pDevice->Release();

        return UpdateSubresources(
            pCmdList,
            pDestinationResource,
            pIntermediate,
            FirstSubresource,
            NumSubresources,
            RequiredSize,
            Layouts,
            NumRows,
            RowSizesInBytes,
            pSrcData);
    }

    // collection of common descriptors used in DX12
    class Desc {
    public: 
        class Buffer : public D3D12_RESOURCE_DESC {
        public:
            Buffer(uint32 size, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, uint32 alignment = 0) noexcept {
                Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                Alignment = alignment;
                Width = size;
                Height = 1;
                DepthOrArraySize = 1;
                MipLevels = 1;
                Format = DXGI_FORMAT_UNKNOWN;
                SampleDesc.Count = 1;
                SampleDesc.Quality = 0;
                Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                Flags = flags;
            }
        };

        class Tex1D : public D3D12_RESOURCE_DESC {
        public:
            Tex1D( DXGI_FORMAT format,uint32 width,uint32 arraySize = 1,uint16 mipLevels = 0,D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,uint32 alignment = 0) noexcept {
                Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                Alignment = alignment;
                Width = width;
                Height = 1;
                DepthOrArraySize = 1;
                MipLevels = mipLevels;
                Format = format;
                SampleDesc.Count = 1;
                SampleDesc.Quality = 0;
                Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                Flags = flags;
            }
        };

        class Tex2D : public D3D12_RESOURCE_DESC {
        public:
            Tex2D(
                DXGI_FORMAT format,uint32 width,uint32 height,uint16 arraySize = 1,uint16 mipLevels = 0,uint sampleCount = 1,uint sampleQuality = 0,
                        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,uint32 alignment = 0) noexcept {
                Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                Alignment = alignment;
                Width = width;
                Height = height;
                DepthOrArraySize = 1;
                MipLevels = mipLevels;
                Format = format;
                SampleDesc.Count = sampleCount;
                SampleDesc.Quality = sampleQuality;
                Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                Flags = flags;
            }
        };

        class Tex3D : public D3D12_RESOURCE_DESC {
        public: 
            Tex3D(DXGI_FORMAT format,uint32 width,uint32 height,uint32 depth,uint16 mipLevels = 0,D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
                    uint32 alignment = 0) noexcept {
                Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                Alignment = alignment;
                Width = width;
                Height = height;
                DepthOrArraySize = depth;
                MipLevels = mipLevels;
                Format = format;
                SampleDesc.Count = 1;
                SampleDesc.Quality = 0;
                Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                Flags = flags;
            }
        };
    };

    class Heap {
    public: 
        class Basic : public D3D12_HEAP_PROPERTIES {
        public: 
            Basic(D3D12_HEAP_TYPE type, UINT creationNodeMask = 1, UINT nodeMask = 1) noexcept {
                Type = type;
                CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
                CreationNodeMask = creationNodeMask;
                VisibleNodeMask = nodeMask;
            }
        };
    };

} // namespace up::d3d12
