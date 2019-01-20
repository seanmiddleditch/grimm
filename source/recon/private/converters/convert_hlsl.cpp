// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_hlsl.h"
#include "grimm/gpu/com_ptr.h"
#include "grimm/gpu/direct3d.h"
#include "grimm/foundation/out_ptr.h"
#include "grimm/foundation/string_view.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/filesystem/path_util.h"
#include <d3dcompiler.h>
#include <iostream>
#include <fstream>

namespace {
    struct ReconIncludeHandler : public ID3DInclude {
        ReconIncludeHandler(gm::fs::FileSystem& fileSystem, gm::recon::Context& ctx, gm::string_view folder)
            : _fileSystem(fileSystem),
              _ctx(ctx),
              _folder(folder) {}

        HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override {
            std::string absolutePath = gm::fs::path::join({_folder, pFileName});

            std::cout << "Including `" << absolutePath << "'\n";

            _ctx.addSourceDependency(pFileName);

            auto stream = _fileSystem.openRead(absolutePath.c_str(), gm::fs::FileOpenMode::Text);
            if (!stream) {
                return E_FAIL;
            }

            stream.seekg(0, std::ios::end);
            auto tell = stream.tellg();
            stream.seekg(0, std::ios::beg);

            gm::vector<char> bytes;
            bytes.resize(tell);

            stream.read(bytes.data(), bytes.size());

            *ppData = bytes.data();
            *pBytes = static_cast<UINT>(bytes.size());

            _shaders.push_back(std::move(bytes));

            return S_OK;
        }

        HRESULT __stdcall Close(LPCVOID pData) override {
            return S_OK;
        }

        gm::fs::FileSystem& _fileSystem;
        gm::recon::Context& _ctx;
        gm::string_view _folder;
        gm::vector<gm::vector<char>> _shaders;
    };
} // namespace

gm::recon::HlslConverter::HlslConverter() = default;

gm::recon::HlslConverter::~HlslConverter() = default;

bool gm::recon::HlslConverter::convert(Context& ctx) {
    fs::FileSystem fs;

    auto absoluteSourcePath = fs::path::join({string_view(ctx.sourceFolderPath()), ctx.sourceFilePath()});

    auto stream = fs.openRead(absoluteSourcePath.c_str(), gm::fs::FileOpenMode::Text);
    if (!stream) {
        return false;
    }

    stream.seekg(0, std::ios::end);
    auto tell = stream.tellg();
    stream.seekg(0, std::ios::beg);

    gm::vector<char> bytes;
    bytes.resize(tell);

    stream.read(bytes.data(), bytes.size());

    zstring_view const entry = "vertex_main";
    zstring_view const target = "vs_5_1";

    std::cout << "Compiling " << absoluteSourcePath << "':" << entry << '(' << target << ")\n";

    ReconIncludeHandler includeHandler(fs, ctx, gm::fs::path::parent(absoluteSourcePath.c_str()));

    com_ptr<ID3DBlob> blob;
    com_ptr<ID3DBlob> errors;
    HRESULT hr = D3DCompile2(bytes.data(), bytes.size(), ctx.sourceFilePath().c_str(), nullptr, &includeHandler, entry.c_str(), target.c_str(), D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS, 0, 0, nullptr, 0, out_ptr(blob), out_ptr(errors));
    if (!SUCCEEDED(hr)) {
        std::cerr << "Compilation failed for `" << absoluteSourcePath << "':" << entry << '(' << target << ")\n"
                  << string_view((char*)errors->GetBufferPointer(), errors->GetBufferSize()) << '\n';
        return false;
    }

    auto destPath = fs::path::changeExtension(ctx.sourceFilePath(), ".vs_6_0.dxo");
    auto destAbsolutePath = fs::path::join({string_view(ctx.destinationFolderPath()), destPath.c_str()});

    std::string destParentAbsolutePath(fs::path::parent(string_view(destAbsolutePath)));
    if (!fs.directoryExists(destParentAbsolutePath.c_str())) {
        if (!fs.createDirectories(destParentAbsolutePath.c_str())) {
            std::cerr << "Failed to create `" << destParentAbsolutePath << "'\n";
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    auto compiledOutput = fs.openWrite(destAbsolutePath.c_str(), gm::fs::FileOpenMode::Binary);
    if (!compiledOutput.is_open()) {
        std::cerr << "Cannot write `" << destAbsolutePath << '\n';
        return false;
    }

    ctx.addOutput(destPath.c_str());

    compiledOutput.write((char*)blob->GetBufferPointer(), blob->GetBufferSize());
    compiledOutput.close();

    return true;
}
