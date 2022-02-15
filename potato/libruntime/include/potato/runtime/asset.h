// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "assertion.h"
#include "uuid.h"

#include "potato/spud/hash.h"
#include "potato/spud/int_types.h"
#include "potato/spud/key.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"

namespace up {
    using AssetId = Key<struct AssetIdTag, uint64>;

    class Asset;
    template <typename AssetT>
    class AssetHandle;

    struct AssetKey {
        UUID uuid;
        string logical;

        AssetId makeAssetId() const noexcept {
            uint64 hash = hash_value(uuid);
            if (!logical.empty()) {
                hash = hash_combine(hash, hash_value(logical));
            }
            return static_cast<AssetId>(hash);
        }

        constexpr bool operator==(AssetKey const&) const noexcept = default;
    };

    class Asset : public shared<Asset> {
    public:
        explicit Asset(AssetKey key) noexcept : _key(std::move(key)) { }

        virtual zstring_view assetType() const noexcept = 0;
        AssetKey const& assetKey() const noexcept { return _key; }
        AssetId assetId() const noexcept { return _key.makeAssetId(); }

        void addRef() const noexcept { ++_refs; }
        void removeRef() const noexcept { UP_ASSERT(--_refs >= 0); }

        // Note: this might return true for an asset that's about to be un-doomed;
        // only rely on this when the asset database is locked
        bool isDoomed() const noexcept { return _refs == 0; }

    protected:
        virtual ~Asset() = default;

    private:
        mutable std::atomic<int> _refs = 1;
        AssetKey _key{};

        friend class AssetLoader;
    };

    template <typename DerivedT>
    class AssetBase : public Asset {
    public:
        using Asset::Asset;

        using Handle = AssetHandle<DerivedT>;

        zstring_view assetType() const noexcept final { return DerivedT::assetTypeName; }
    };

    class UntypedAssetHandle {
    public:
        UntypedAssetHandle() = default;
        explicit UntypedAssetHandle(AssetKey key) noexcept : _key(std::move(key)) { }
        explicit UntypedAssetHandle(rc<Asset> asset) noexcept : _asset(std::move(asset)) {
            if (_asset != nullptr) {
                _key = _asset->assetKey();
            }
        }
        UntypedAssetHandle(AssetKey key, rc<Asset> asset) noexcept : _key(std::move(key)), _asset(std::move(asset)) {
            UP_ASSERT(asset == nullptr || asset->assetKey() == _key);
        }

        [[nodiscard]] bool isSet() const noexcept { return _key.uuid.isValid(); }
        [[nodiscard]] bool ready() const noexcept { return _asset != nullptr; }

        [[nodiscard]] AssetKey const& assetKey() const noexcept { return _key; }
        [[nodiscard]] AssetId assetId() const noexcept { return _key.makeAssetId(); }

        template <typename AssetT>
        [[nodiscard]] AssetHandle<AssetT> cast() const& noexcept;
        template <typename AssetT>
        [[nodiscard]] AssetHandle<AssetT> cast() && noexcept;

        [[nodiscard]] Asset* asset() const noexcept { return _asset.get(); }
        [[nodiscard]] Asset* release() noexcept { return _asset.release(); }

    private:
        AssetKey _key{};
        rc<Asset> _asset;
    };

    template <typename AssetT>
    class AssetHandle : public UntypedAssetHandle {
    public:
        using AssetType = AssetT;

        AssetHandle() = default;
        // NOLINTNEXTLINE(performance-unnecessary-value-param) -- clang-tidy false-positive
        AssetHandle(AssetKey key, rc<AssetT> asset) noexcept : UntypedAssetHandle(std::move(key), std::move(asset)) { }
        explicit AssetHandle(rc<AssetT> asset) noexcept : UntypedAssetHandle(std::move(asset)) { }

        zstring_view typeName() const noexcept { return AssetT::assetTypeName; }

        AssetT* asset() const noexcept { return static_cast<AssetT*>(UntypedAssetHandle::asset()); }
        AssetT* release() noexcept { return static_cast<AssetT*>(UntypedAssetHandle::release()); }
    };

    template <typename AssetT>
    AssetHandle<AssetT> UntypedAssetHandle::cast() const& noexcept {
        return {_key, _asset};
    }

    template <typename AssetT>
    AssetHandle<AssetT> UntypedAssetHandle::cast() && noexcept {
        return {_key, rc{static_cast<AssetT*>(_asset.release())}};
    }
} // namespace up
