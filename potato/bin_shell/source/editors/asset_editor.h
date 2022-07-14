// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/editor/asset_edit_service.h"
#include "potato/editor/editor.h"
#include "potato/shell/selection.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/delegate.h"
#include "potato/spud/generator.h"
#include "potato/spud/hash.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>

namespace up {
    class ReconClient;
}

namespace up::shell {
    class AssetEditor final : public Editor<AssetEditor> {
    public:
        static constexpr EditorTypeId editorTypeId{"potato.editor.asset_browser"};

        using OnFileSelected = delegate<void(UUID const& uuid)>;

        AssetEditor(
            EditorParams const& params,
            AssetLoader& assetLoader,
            ReconClient& reconClient,
            AssetEditService& assetEditService,
            OnFileSelected& onFileSelected);

        zstring_view displayName() const override { return "Assets"; }

        static void addFactory(
            Workspace& workspace,
            AssetLoader& assetLoader,
            ReconClient& reconClient,
            AssetEditService& assetEditService,
            AssetEditor::OnFileSelected onFileSelected);

    protected:
        void content(CommandManager&) override;
        bool isCloseable() override { return false; }

    private:
        struct Entry {
            uint64 id = 0;
            UUID uuid;
            string osPath;
            string name;
            uint64 typeHash = 0;
            size_t size = 0;
            uint32 childFileCount = 0;
            uint32 childFolderCount = 0;
            int firstChild = -1;
            int nextSibling = -1;
            int parentIndex = -1;
        };

        enum class Command {
            None,
            OpenFolder,
            OpenInExplorer,
            ShowInExplorer,
            EditAsset,
            Trash,
            Import,
            ForceImport,
            ShowRenameDialog,
            Rename,
            ShowNewFolderDialog,
            CreateFolder,
            ShowNewAssetDialog,
            CreateAsset,
        };

        void _showAssets(Entry const& folder);
        void _showAsset(Entry const& asset);
        void _showFolder(Entry const& folder);
        void _showSearchAssets(string_view searchText);

        void _showBreadcrumb(int index);
        void _showBreadcrumbs();

        void _showTreeFolder(int index);
        void _showTreeFolders();

        void _showRenameDialog();
        void _showNewFolderDialog();
        void _showNewAssetDialog();

        void _rebuild();
        int _addFolder(string_view name, int parentIndex = 0);
        int _addFolders(string_view folderPath);

        void _openFolder(int index);
        void _importAsset(UUID const& uuid, bool force = false);

        void _executeCommand();

        generator<Entry const&> _children(Entry const& folder) const {
            for (int childIndex = folder.firstChild; childIndex != -1; childIndex = _entries[childIndex].nextSibling) {
                co_yield _entries[childIndex];
            }
        }

        AssetLoader& _assetLoader;
        AssetEditService& _assetEditService;
        ReconClient& _reconClient;
        OnFileSelected& _onFileSelected;
        SelectionState _selection;
        vector<Entry> _entries;
        int _currentFolder = 0;
        int _manifestRevision = 0;
        Command _command = Command::None;
        char _nameBuffer[128] = {0};
        char _renameBuffer[128] = {0};
        char _searchBuffer[128] = {0};
        uint64 _newAssetType = 0;

        static constexpr int assetIconWidth = 96;
        static constexpr char folderType[] = "potato.folder";
        static constexpr uint64 folderTypeHash = hash_value(folderType);

        static constexpr size_t maxFolderHistory = 64;
        size_t _folderHistoryIndex = 0;
        vector<int> _folderHistory{0};
    };
} // namespace up::shell
