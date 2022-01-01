// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/shader.h"

#include "potato/runtime/asset_loader.h"
#include "potato/runtime/stream.h"

namespace up {
    namespace {
        class ShaderLoader : public AssetLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return Shader::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                vector<byte> contents;
                if (auto rs = readBinary(ctx.stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                ctx.stream.close();

                return up::new_shared<Shader>(ctx.key, std::move(contents));
            }
        };
    } // namespace

    void Shader::registerLoader(AssetLoader& assetLoader, GpuDevice&) {
        assetLoader.registerBackend(new_box<ShaderLoader>());
    }
} // namespace up
