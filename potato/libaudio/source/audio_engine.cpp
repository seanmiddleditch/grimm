// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "audio_engine.h"
#include "sound_resource.h"

#include "potato/runtime/asset_loader.h"
#include "potato/runtime/stream.h"
#include "potato/spud/vector.h"

#include <soloud.h>
#include <soloud_wav.h>

namespace up {
    namespace {
        class AudioEngineImpl final : public AudioEngine {
        public:
            explicit AudioEngineImpl();
            ~AudioEngineImpl() override;

            void registerAssetBackends(AssetLoader& assetLoader) override;
            auto play(SoundResource const* sound) -> PlayHandle override;

        private:
            SoLoud::Soloud _soloud;
        };

        class SoundResourceWav final : public SoundResource {
        public:
            using SoundResource::SoundResource;

            mutable SoLoud::Wav _wav;
        };

        class SoundAssetLoaderBackend : public AssetLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return SoundResource::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override;
        };
    } // namespace
} // namespace up

up::AudioEngineImpl::AudioEngineImpl() {
    _soloud = SoLoud::Soloud();
    _soloud.init();
}

up::AudioEngineImpl::~AudioEngineImpl() {
    _soloud.deinit();
}

auto up::AudioEngine::create() -> box<AudioEngine> {
    return new_box<AudioEngineImpl>();
}

void up::AudioEngineImpl::registerAssetBackends(AssetLoader& assetLoader) {
    assetLoader.registerBackend(new_box<SoundAssetLoaderBackend>());
}

auto up::AudioEngineImpl::play(SoundResource const* sound) -> PlayHandle {
    if (sound == nullptr) {
        return {};
    }

    auto handle = _soloud.play(static_cast<SoundResourceWav const*>(sound)->_wav);
    return static_cast<PlayHandle>(handle);
}

auto up::SoundAssetLoaderBackend::loadFromStream(AssetLoadContext const& ctx) -> rc<Asset> {
    vector<byte> contents;
    if (auto rs = readBinary(ctx.stream, contents); rs != IOResult::Success) {
        return nullptr;
    }

    auto wav = new_shared<SoundResourceWav>(ctx.key);
    auto const result = wav->_wav.loadMem(
        reinterpret_cast<unsigned char const*>(contents.data()),
        static_cast<unsigned int>(contents.size()),
        true,
        false);
    if (result != 0) {
        return nullptr;
    }

    return wav;
}
