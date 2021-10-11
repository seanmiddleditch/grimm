// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "model_importer.h"

#include "potato/render/model_generated.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/spud/std_iostream.h"

#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

up::ModelImporter::ModelImporter() = default;

up::ModelImporter::~ModelImporter() = default;

bool up::ModelImporter::import(ImporterContext& ctx) {
    using namespace flatbuffers;

    auto sourceAbsolutePath = path::join(ctx.sourceFolderPath(), ctx.sourceFilePath());
    auto destPath = path::changeExtension(ctx.sourceFilePath(), ".model");
    auto destAbsolutePath = path::join(ctx.destinationFolderPath(), destPath);

    string destParentAbsolutePath(path::parent(string_view(destAbsolutePath)));

    if (!fs::directoryExists(destParentAbsolutePath.c_str())) {
        if (fs::createDirectories(destParentAbsolutePath.c_str()) != IOResult::Success) {
            ctx.logger().error("Failed to create `{}'", destParentAbsolutePath);
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    auto file = fs::openRead(sourceAbsolutePath);

    vector<byte> contents;
    if (readBinary(file, contents) != IOResult::Success) {
        ctx.logger().error("Failed to read");
        return false;
    }

    file.close();

    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFileFromMemory(
        contents.data(),
        contents.size(),
        aiProcess_FlipWindingOrder | aiProcess_GenNormals | aiProcess_CalcTangentSpace |
            aiProcessPreset_TargetRealtime_Fast);
    if (scene == nullptr) {
        ctx.logger().error("Failed to decode");
        return false;
    }

    if (scene->mNumMeshes != 1) {
        ctx.logger().error("Only single meshes are supported");
        return false;
    }

    auto const* mesh = scene->mMeshes[0];

    FlatBufferBuilder builder;

    auto flatIndices = builder.CreateVector<uint16>(mesh->mNumFaces * 3, [mesh](auto index) {
        auto face = index / 3;
        auto vert = index % 3;
        return mesh->mFaces[face].mIndices[vert];
    });
    auto flatVerts = builder.CreateVectorOfStructs<schema::Vec3>(mesh->mNumVertices, [mesh](auto index, auto* dest) {
        auto [x, y, z] = mesh->mVertices[index];
        *dest = {x, y, z};
    });
    Offset<Vector<schema::Vec3 const*>> flatColors;
    if (mesh->GetNumColorChannels() >= 1) {
        flatColors = builder.CreateVectorOfStructs<schema::Vec3>(mesh->mNumVertices, [mesh](auto index, auto* dest) {
            auto [r, g, b, a] = mesh->mColors[0][index];
            *dest = {r, g, b};
        });
    }
    Offset<Vector<schema::Vec3 const*>> flatNormals;
    if (mesh->HasNormals()) {
        flatNormals = builder.CreateVectorOfStructs<schema::Vec3>(mesh->mNumVertices, [mesh](auto index, auto* dest) {
            auto [x, y, z] = mesh->mNormals[index];
            *dest = {x, y, z};
        });
    }
    Offset<Vector<schema::Vec3 const*>> flatTangents;
    if (mesh->HasTangentsAndBitangents()) {
        flatTangents = builder.CreateVectorOfStructs<schema::Vec3>(mesh->mNumVertices, [mesh](auto index, auto* dest) {
            auto [x, y, z] = mesh->mTangents[index];
            *dest = {x, y, z};
        });
    }
    Offset<Vector<schema::Vec2 const*>> flatUVs;
    if (mesh->HasTextureCoords(0)) {
        flatUVs = builder.CreateVectorOfStructs<schema::Vec2>(mesh->mNumVertices, [mesh](auto index, auto* dest) {
            auto [s, t, u] = mesh->mTextureCoords[0][index];
            *dest = {s, t};
        });
    }

    auto flatMesh = schema::CreateMesh(builder, flatIndices, flatVerts, flatNormals, flatTangents, flatUVs, flatColors);
    auto flatMeshes = builder.CreateVector<schema::Mesh>(&flatMesh, 1);
    auto flatModel = schema::CreateModel(builder, flatMeshes);

    builder.Finish(flatModel);

    file = fs::openWrite(destAbsolutePath);
    file.write(span{reinterpret_cast<byte const*>(builder.GetBufferPointer()), builder.GetSize()});
    file.close();

    ctx.addMainOutput(destPath, "potato.asset.model");

    ctx.logger().info("Wrote optimized model to `{}'", destAbsolutePath);

    return true;
}
