// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "hlsl_importer.h"

#include "potato/runtime/com_ptr.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/spud/out_ptr.h"
#include "potato/spud/std_iostream.h"
#include "potato/spud/string_view.h"
#include "potato/spud/string_writer.h"

#if defined(UP_GPU_ENABLE_D3D11)
#    include <d3d11.h>
#    include <d3dcompiler.h>
#    include <fstream>

namespace {
    struct ReconIncludeHandler : public ID3DInclude {
        ReconIncludeHandler(up::ImporterContext& ctx, up::string_view folder) : _ctx(ctx), _folder(folder) {}

        HRESULT __stdcall Open(
            D3D_INCLUDE_TYPE IncludeType,
            LPCSTR pFileName,
            LPCVOID pParentData,
            LPCVOID* ppData,
            UINT* pBytes) noexcept override {
            up::string absolutePath = up::path::join(_folder, pFileName);

            _ctx.logger().info("Including `{}'", absolutePath);

            auto relPath = absolutePath.substr(_ctx.sourceFolderPath().size() + 1);
            _ctx.addSourceDependency(relPath.data());

            auto [rs, source] = up::fs::readText(absolutePath.c_str());
            if (rs != up::IOResult::Success) {
                return E_FAIL;
            }

            *ppData = source.data();
            *pBytes = static_cast<UINT>(source.size());

            _shaders.push_back(std::move(source));

            return S_OK;
        }

        HRESULT __stdcall Close(LPCVOID pData) noexcept override { return S_OK; }

        up::ImporterContext& _ctx;
        up::string_view _folder;
        up::vector<up::string> _shaders;
    }; // namespace
} // namespace
#endif

up::HlslImporter::HlslImporter() = default;

up::HlslImporter::~HlslImporter() = default;

bool up::HlslImporter::import(ImporterContext& ctx) {
#if defined(UP_GPU_ENABLE_D3D11)
    auto absoluteSourcePath = path::join(string_view(ctx.sourceFolderPath()), ctx.sourceFilePath());

    auto stream = fs::openRead(absoluteSourcePath.c_str(), fs::OpenMode::Text);
    if (!stream) {
        return false;
    }

    string shader;
    if (readText(stream, shader) != IOResult::Success) {
        ctx.logger().error("Failed to read `{}'", absoluteSourcePath);
        return false;
    }

    stream.close();

    bool success = _compile(ctx, absoluteSourcePath, shader, "vertex", "vertex_main", "vs_5_0");
    success = _compile(ctx, absoluteSourcePath, shader, "pixel", "pixel_main", "ps_5_0") && success;

    return success;
#else
    return true;
#endif
}

#if defined(UP_GPU_ENABLE_D3D11)
bool up::HlslImporter::_compile(
    ImporterContext& ctx,
    zstring_view absoluteSourcePath,
    string_view source,
    string_view logicalName,
    zstring_view entryName,
    zstring_view targetProfileName) {
    ctx.logger().info("Compiling `{}':{} ({})", ctx.sourceFilePath(), entryName, targetProfileName);

    ReconIncludeHandler includeHandler(ctx, up::path::parent(absoluteSourcePath));

    com_ptr<ID3DBlob> blob;
    com_ptr<ID3DBlob> errors;
    HRESULT hr = D3DCompile2(
        source.data(),
        source.size(),
        ctx.sourceFilePath().c_str(),
        nullptr,
        &includeHandler,
        entryName.c_str(),
        targetProfileName.c_str(),
        D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS,
        0,
        0,
        nullptr,
        0,
        out_ptr(blob),
        out_ptr(errors));
    if (!SUCCEEDED(hr)) {
        ctx.logger().error("Compilation failed for `{}':{} ({})", ctx.sourceFilePath(), entryName, targetProfileName);
        ctx.logger().error(string_view{static_cast<char const*>(errors->GetBufferPointer()), errors->GetBufferSize()});
        return false;
    }

    string_writer ext;
    ext.append('.');
    ext.append(targetProfileName);
    ext.append(".cbo");

    auto destPath = path::changeExtension(ctx.sourceFilePath(), ext.c_str());
    auto destAbsolutePath = path::join(string_view(ctx.destinationFolderPath()), destPath.c_str());

    string destParentAbsolutePath(path::parent(destAbsolutePath));

    if (!fs::directoryExists(destParentAbsolutePath.c_str())) {
        if (fs::createDirectories(destParentAbsolutePath.c_str()) != IOResult::Success) {
            ctx.logger().error("Failed to create `{}'", destParentAbsolutePath);
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    auto compiledOutput = fs::openWrite(destAbsolutePath.c_str(), fs::OpenMode::Binary);
    if (!compiledOutput.isOpen()) {
        ctx.logger().error("Cannot write `{}'", destAbsolutePath);
        return false;
    }

    ctx.addOutput(string{logicalName}, destPath, "potato.asset.shader");

    compiledOutput.write({static_cast<up::byte const*>(blob->GetBufferPointer()), blob->GetBufferSize()});
    compiledOutput.close();

    return true;
}
#endif
