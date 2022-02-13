// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_editor.h"

#include "potato/editor/desktop.h"
#include "potato/editor/editor.h"
#include "potato/editor/editor_manager.h"
#include "potato/editor/imgui_ext.h"
#include "potato/editor/imgui_fonts.h"
#include "potato/recon/recon_client.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/path.h"
#include "potato/runtime/resource_manifest.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/enumerate.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace up::shell {
    static constexpr zstring_view AssetEditorRenameDialogName = "Rename Files and Folders##asset_browser_rename"_zsv;
    static constexpr zstring_view AssetEditorNewFolderDialogName = "New Folder##asset_browser_new_folder"_zsv;
    static constexpr zstring_view AssetEditorNewAssetDialogName = "New Asset##asset_browser_new_asset"_zsv;

    namespace {
        class AssetEditorFactory : public EditorFactory<AssetEditor> {
        public:
            AssetEditorFactory(
                AssetLoader& assetLoader,
                ReconClient& reconClient,
                AssetEditService& assetEditService,
                AssetEditor::OnFileSelected onFileSelected)
                : _assetLoader(assetLoader)
                , _reconClient(reconClient)
                , _assetEditService(assetEditService)
                , _onFileSelected(std::move(onFileSelected)) { }

            box<EditorBase> createEditor(EditorParams const& params) override {
                return new_box<AssetEditor>(params, _assetLoader, _reconClient, _assetEditService, _onFileSelected);
            }

        private:
            AssetLoader& _assetLoader;
            ReconClient& _reconClient;
            AssetEditService& _assetEditService;
            AssetEditor::OnFileSelected _onFileSelected;
        };
    } // namespace

    AssetEditor::AssetEditor(
        EditorParams const& params,
        AssetLoader& assetLoader,
        ReconClient& reconClient,
        AssetEditService& assetEditService,
        OnFileSelected& onFileSelected)
        : Editor(params)
        , _assetLoader(assetLoader)
        , _assetEditService(assetEditService)
        , _reconClient(reconClient)
        , _onFileSelected(onFileSelected) {
        addPanel("Asset Tree", PanelDir::Left, [this] { _showTreeFolders(); });

        _rebuild();
    }

    void AssetEditor::addFactory(
        EditorManager& editors,
        AssetLoader& assetLoader,
        ReconClient& reconClient,
        AssetEditService& assetEditService,
        AssetEditor::OnFileSelected onFileSelected) {
        editors.addFactory<AssetEditorFactory>(assetLoader, reconClient, assetEditService, std::move(onFileSelected));
    }

    void AssetEditor::content(CommandManager&) {
        if (_manifestRevision != _assetLoader.manifestRevision()) {
            _rebuild();
        }

        _showBreadcrumbs();

        if (ImGui::BeginIconGrid("##assets")) {
            if (_searchBuffer[0] != '\0') {
                _showSearchAssets(_searchBuffer);
            }
            else {
                _showAssets(_entries[_currentFolder]);
            }
            ImGui::EndIconGrid();
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered() &&
            ImGui::TableGetHoveredColumn() == 1) {
            ImGui::OpenPopup("##folder_popup");
        }
        if (ImGui::BeginPopup(
                "##folder_popup",
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
            if (ImGui::MenuItemEx("New Folder", ICON_FA_FOLDER)) {
                _command = Command::ShowNewFolderDialog;
            }
            ImGui::EndPopup();
        }

        _showRenameDialog();
        _showNewFolderDialog();
        _showNewAssetDialog();

        _executeCommand();
    }

    void AssetEditor::_showAssets(Entry const& folder) {
        for (Entry const& entry : _children(folder)) {
            if (entry.typeHash == folderTypeHash) {
                _showFolder(entry);
            }
            else {
                _showAsset(entry);
            }
        }
    }

    void AssetEditor::_showAsset(Entry const& asset) {
        UP_GUARD_VOID(asset.typeHash != folderTypeHash);

        if (ImGui::IconGridItem(
                static_cast<ImGuiID>(asset.id),
                asset.name.c_str(),
                _assetEditService.findInfoForAssetTypeHash(asset.typeHash).icon,
                _selection.selected(asset.id))) {
            _command = Command::EditAsset;
        }

        if (ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            _selection.click(
                asset.id,
                ImGui::IsModifierDown(ImGuiKeyModFlags_Ctrl),
                ImGui::IsMouseDown(ImGuiMouseButton_Right));
        }

        ImGui::GetItemID();

        if (ImGui::BeginContextPopup()) {
            if (ImGui::MenuItemEx("Edit Asset", ICON_FA_EDIT)) {
                _command = Command::EditAsset;
            }
            if (ImGui::MenuItemEx("Import", ICON_FA_DOWNLOAD)) {
                _command = Command::Import;
            }
            if (ImGui::MenuItem("Import (Force)")) {
                _command = Command::ForceImport;
            }
            ImGui::Separator();
            if (ImGui::MenuItemEx("Copy Path", ICON_FA_COPY)) {
                ImGui::SetClipboardText(asset.osPath.c_str());
            }
            if (ImGui::MenuItem("Copy UUID")) {
                char buf[UUID::strLength];
                nanofmt::format_to(buf, "{}", asset.uuid);
                ImGui::SetClipboardText(buf);
            }
            if (ImGui::MenuItemEx("Show in Explorer", ICON_FA_FOLDER_OPEN)) {
                _command = Command::ShowInExplorer;
            }
            ImGui::Separator();
            if (ImGui::MenuItemEx("Rename", ICON_FA_PEN)) {
                _command = Command::ShowRenameDialog;
            }
            ImGui::Separator();
            if (ImGui::MenuItemEx("Move to Trash", ICON_FA_TRASH)) {
                _command = Command::Trash;
            }
            ImGui::EndPopup();
        }
    }

    void AssetEditor::_showFolder(Entry const& folder) {
        UP_GUARD_VOID(folder.typeHash == folderTypeHash);

        if (ImGui::IconGridItem(
                static_cast<ImGuiID>(folder.id),
                folder.name.c_str(),
                ICON_FA_FOLDER,
                _selection.selected(folder.id))) {
            _command = Command::OpenFolder;
        }

        if (ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            _selection.click(
                folder.id,
                ImGui::IsModifierDown(ImGuiKeyModFlags_Ctrl),
                ImGui::IsMouseDown(ImGuiMouseButton_Right));
        }

        if (ImGui::BeginContextPopup()) {
            if (ImGui::MenuItemEx("Open", ICON_FA_FOLDER_OPEN)) {
                _command = Command::OpenFolder;
            }
            if (ImGui::MenuItem("Open in Explorer")) {
                _command = Command::OpenInExplorer;
            }
            ImGui::Separator();
            if (ImGui::MenuItemEx("Copy Path", ICON_FA_COPY)) {
                ImGui::SetClipboardText(folder.osPath.c_str());
            }
            ImGui::Separator();
            if (ImGui::MenuItemEx("Rename", ICON_FA_PEN)) {
                _command = Command::ShowRenameDialog;
            }
            ImGui::Separator();
            if (ImGui::MenuItemEx("Move to Trash", ICON_FA_TRASH)) {
                _command = Command::Trash;
            }
            ImGui::EndPopup();
        }
    }

    void AssetEditor::_showSearchAssets(string_view searchText) {
        for (Entry const& entry : _entries) {
            if (stringIndexOfNoCase(entry.name.data(), entry.name.size(), searchText.data(), searchText.size()) < 0) {
                continue;
            }

            if (entry.typeHash == folderTypeHash) {
                _showFolder(entry);
            }
            else {
                _showAsset(entry);
            }
        }
    }

    void AssetEditor::_showBreadcrumb(int index) {
        if (_entries[index].parentIndex != -1) {
            _showBreadcrumb(_entries[index].parentIndex);
            ImGui::SameLine(0, 0);
            ImGui::TextDisabled("%s", reinterpret_cast<char const*>(ICON_FA_CARET_RIGHT));
            ImGui::SameLine(0, 0);
        }

        ImGui::BeginDisabled(index == _folderHistory[_folderHistoryIndex]);
        if (ImGui::Button(_entries[index].name.c_str())) {
            _openFolder(index);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
    }

    void AssetEditor::_showBreadcrumbs() {
        ImGuiWindow* const window = ImGui::GetCurrentWindow();

        ImGui::Spacing();
        if (ImGui::BeginToolbar("##breadcrums")) {
            if (ImGui::BeginIconButtonDropdown("New", ICON_FA_PLUS)) {
                if (ImGui::MenuItemEx("New Asset", ICON_FA_FILE)) {
                    _command = Command::ShowNewAssetDialog;
                }
                if (ImGui::MenuItemEx("New Folder", ICON_FA_FOLDER)) {
                    _command = Command::ShowNewFolderDialog;
                }
                ImGui::EndIconButtonDropdown();
            }
            ImGui::SameLine();

            ImGui::BeginDisabled(_folderHistoryIndex == 0);
            if (ImGui::IconButton("##back", ICON_FA_ARROW_LEFT)) {
                _currentFolder = _folderHistory[--_folderHistoryIndex];
                _selection.clear();
            }
            ImGui::EndDisabled();
            ImGui::SameLine(0, 1.f);

            ImGui::BeginDisabled(_folderHistory.size() < 2 || _folderHistoryIndex == _folderHistory.size() - 1);
            if (ImGui::IconButton("##forward", ICON_FA_ARROW_RIGHT)) {
                _currentFolder = _folderHistory[++_folderHistoryIndex];
                _selection.clear();
            }
            ImGui::EndDisabled();
            ImGui::SameLine(0, 1.f);

            ImGui::BeginDisabled(_entries[_currentFolder].parentIndex == -1);
            if (ImGui::IconButton("##parent", ICON_FA_ARROW_UP)) {
                _openFolder(_entries[_currentFolder].parentIndex);
                _selection.clear();
            }
            ImGui::EndDisabled();
            ImGui::SameLine();

            _showBreadcrumb(_currentFolder);
            ImGui::SameLine();

            ImGui::Spacing();
            ImGui::SameLine();

            float const minSearchWidth = 100.f;
            float const maxSearchWidth = 400.f;
            float const availWidth = ImGui::GetContentRegionAvail().x;
            if (availWidth >= minSearchWidth) {
                float const searchWidth = min(availWidth, maxSearchWidth);
                float const spacingBefore = availWidth - searchWidth;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + spacingBefore);

                if (ImGui::BeginInlineFrame(ICON_FA_SEARCH "##searchbar")) {
                    ImGui::InputText("##search", _searchBuffer, sizeof(_searchBuffer));
                    ImGui::EndInlineFrame();
                }
            }

            ImGui::EndToolbar();
        }
        window->DC.CursorPos.y += ImGui::GetItemSpacing().y;
    }

    void AssetEditor::_showTreeFolder(int index) {
        int flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

        UP_GUARD_VOID(_entries[index].typeHash == folderTypeHash);

        if (_entries[index].childFolderCount == 0) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (index == _currentFolder) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (index == 0) {
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        char label[512];
        nanofmt::format_to(label, "{} {}", ICON_FA_FOLDER, _entries[index].name);

        if (ImGui::TreeNodeEx(label, flags)) {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                _openFolder(index);
            }

            for (int childIndex = _entries[index].firstChild; childIndex != -1;
                 childIndex = _entries[childIndex].nextSibling) {
                if (_entries[childIndex].typeHash == folderTypeHash) {
                    _showTreeFolder(childIndex);
                }
            }
            ImGui::TreePop();
        }
    }

    void AssetEditor::_showTreeFolders() {
        if (!_entries.empty()) {
            _showTreeFolder(0);
        }
    }

    void AssetEditor::_showRenameDialog() {
        if (ImGui::BeginPopupModal(AssetEditorRenameDialogName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto filterCallback = +[](ImGuiInputTextCallbackData* data) -> int {
                constexpr string_view banList = "/\\;:"_sv;
                if (banList.find(static_cast<char>(data->EventChar)) != string_view::npos) {
                    return 1;
                }
                return 0;
            };

            if (_selection.size() == 1) {
                ImGui::LabelText("Original Name", "%s", _nameBuffer);
                ImGui::InputText(
                    "New Name",
                    _renameBuffer,
                    sizeof(_renameBuffer),
                    ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCharFilter,
                    filterCallback);

                if (ImGui::Button("Accept")) {
                    ImGui::CloseCurrentPopup();
                    _command = Command::Rename;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
            }
            else {
                ImGui::TextDisabled("Multi-rename not yet supported.");
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
    }

    void AssetEditor::_showNewFolderDialog() {
        if (ImGui::BeginPopupModal(
                AssetEditorNewFolderDialogName.c_str(),
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize)) {
            auto filterCallback = +[](ImGuiInputTextCallbackData* data) -> int {
                constexpr string_view banList = "/\\;:"_sv;
                if (banList.find(static_cast<char>(data->EventChar)) != string_view::npos) {
                    return 1;
                }
                return 0;
            };

            ImGui::InputText(
                "Folder Name",
                _nameBuffer,
                sizeof(_nameBuffer),
                ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCharFilter,
                filterCallback);

            if (ImGui::Button("Accept")) {
                ImGui::CloseCurrentPopup();
                _command = Command::CreateFolder;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void AssetEditor::_showNewAssetDialog() {
        if (ImGui::BeginPopupModal(AssetEditorNewAssetDialogName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto filterCallback = +[](ImGuiInputTextCallbackData* data) -> int {
                constexpr string_view banList = "/\\;:"_sv;
                if (banList.find(static_cast<char>(data->EventChar)) != string_view::npos) {
                    return 1;
                }
                return 0;
            };

            if (ImGui::BeginCombo(
                    "Type",
                    _assetEditService.findInfoForAssetTypeHash(_newAssetType).name.c_str(),
                    ImGuiComboFlags_PopupAlignLeft)) {
                AssetEditService::AssetTypeInfo info;
                for (int index = 0; (info = _assetEditService.findInfoForIndex(index)).typeHash != 0; ++index) {
                    bool selected = _newAssetType == 0;
                    if (ImGui::Selectable(info.name.c_str(), &selected)) {
                        _newAssetType = info.typeHash;
                        if (_nameBuffer[0] != '\0' && !info.extension.empty()) {
                            auto const base = path::filebasename(_nameBuffer);
                            _nameBuffer[base.size()] = '\0';
                            nanofmt::format_append_to(_nameBuffer, "{}", info.extension);
                        }
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::LabelText("Folder", "%s", _entries[_currentFolder].name.c_str());
            ImGui::InputText(
                "Asset Name",
                _nameBuffer,
                sizeof(_nameBuffer),
                ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCharFilter,
                filterCallback);

            if (ImGui::Button("Accept")) {
                ImGui::CloseCurrentPopup();
                _command = Command::CreateAsset;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void AssetEditor::_rebuild() {
        ResourceManifest const* const manifest = _assetLoader.manifest();
        _manifestRevision = _assetLoader.manifestRevision();

        uint64 const selectedId = _currentFolder < static_cast<int>(_entries.size()) ? _entries[_currentFolder].id : 0;

        _entries.clear();
        _currentFolder = 0;

        auto rootOsPath = _assetEditService.makeFullPath("/");
        auto const id = hash_value(rootOsPath);
        _entries.push_back({.id = id, .osPath = std::move(rootOsPath), .name = "<root>", .typeHash = folderTypeHash});

        if (manifest == nullptr) {
            return;
        }

        for (ResourceManifest::Record const& record : manifest->records()) {
            auto const lastSepIndex = record.filename.find_last_of("\\/"_sv);
            auto const start = lastSepIndex != string::npos ? lastSepIndex + 1 : 0;

            if (record.logicalId != 0) {
                continue;
            }

            if (record.type == folderType) {
                _addFolders(record.filename);
                continue;
            }

            int folderIndex = 0;
            if (lastSepIndex != string::npos) {
                folderIndex = _addFolders(record.filename.substr(0, lastSepIndex));
            }

            int const newIndex = static_cast<int>(_entries.size());
            _entries.push_back(
                {.id = hash_value(record.uuid),
                 .uuid = record.uuid,
                 .osPath = _assetEditService.makeFullPath(record.filename),
                 .name = string{record.filename.substr(start)},
                 .typeHash = hash_value(record.type),
                 .parentIndex = folderIndex});

            if (_entries[folderIndex].firstChild == -1) {
                _entries[folderIndex].firstChild = newIndex;
            }
            else {
                for (int childIndex = _entries[folderIndex].firstChild; childIndex != -1;
                     childIndex = _entries[childIndex].nextSibling) {
                    if (_entries[childIndex].nextSibling == -1) {
                        _entries[childIndex].nextSibling = newIndex;
                        break;
                    }
                }
            }

            ++_entries[folderIndex].childFileCount;
        }

        for (auto const& [index, entry] : enumerate(_entries)) {
            if (entry.id == selectedId) {
                _currentFolder = static_cast<int>(index);
                break;
            }
        }
    }

    int AssetEditor::_addFolder(string_view name, int parentIndex) {
        UP_ASSERT(parentIndex >= 0 && parentIndex < static_cast<int>(_entries.size()));
        UP_ASSERT(_entries[parentIndex].typeHash == folderTypeHash);

        Entry& parent = _entries[parentIndex];

        int childIndex = -1;
        if (parent.firstChild != -1 && _entries[parent.firstChild].typeHash == folderTypeHash) {
            childIndex = parent.firstChild;
            while (_entries[childIndex].nextSibling != -1) {
                if (_entries[childIndex].name == name) {
                    return childIndex;
                }
                if (_entries[_entries[childIndex].nextSibling].typeHash != folderTypeHash) {
                    break;
                }
                childIndex = _entries[childIndex].nextSibling;
            }

            if (_entries[childIndex].name == name) {
                return childIndex;
            }
        }

        int const newIndex = static_cast<int>(_entries.size());

        auto osPath = path::join(path::Separator::Native, parent.osPath, name);
        auto const id = hash_value(osPath);
        _entries.push_back(
            {.id = id,
             .osPath = std::move(osPath),
             .name = string{name},
             .typeHash = folderTypeHash,
             .parentIndex = parentIndex});

        ++parent.childFolderCount;

        if (childIndex == -1) {
            _entries[newIndex].nextSibling = _entries[parentIndex].firstChild;
            _entries[parentIndex].firstChild = newIndex;
        }
        else {
            _entries[newIndex].nextSibling = _entries[childIndex].nextSibling;
            _entries[childIndex].nextSibling = newIndex;
        }

        return newIndex;
    }

    int AssetEditor::_addFolders(string_view folderPath) {
        int folderIndex = 0;

        string_view::size_type sep = string_view::npos;
        while ((sep = folderPath.find('/')) != string_view::npos) {
            if (sep != 0) {
                folderIndex = _addFolder(folderPath.substr(0, sep), folderIndex);
            }
            folderPath = folderPath.substr(sep + 1);
        }

        if (!folderPath.empty()) {
            folderIndex = _addFolder(folderPath, folderIndex);
        }

        return folderIndex;
    }

    void AssetEditor::_openFolder(int index) {
        // cut any of the "future" history
        if (_folderHistory.size() > _folderHistoryIndex + 1) {
            _folderHistory.resize(_folderHistoryIndex + 1);
        }

        // ensure our history has a limited length
        if (_folderHistory.size() >= maxFolderHistory) {
            _folderHistory.erase(_folderHistory.begin());
        }

        // add history item
        _folderHistoryIndex = _folderHistory.size();
        _folderHistory.push_back(index);

        // select new folder
        _currentFolder = index;
        _searchBuffer[0] = '\0';

        // clear prior selection
        _selection.clear();
    }

    void AssetEditor::_importAsset(UUID const& uuid, bool force) {
        if (!uuid.isValid()) {
            return;
        }

        schema::ReconImportMessage msg;
        msg.uuid = uuid;
        msg.force = force;
        _reconClient.send<ReconImportMessage>({});
    }

    void AssetEditor::_executeCommand() {
        Command cmd = _command;
        _command = Command::None;

        switch (cmd) {
            case Command::OpenFolder:
                for (auto const& [index, folder] : enumerate(_entries)) {
                    if (folder.typeHash == folderTypeHash && _selection.selected(folder.id)) {
                        _openFolder(static_cast<int>(index));
                        break;
                    }
                }
                break;
            case Command::OpenInExplorer:
                for (Entry const& folder : _entries) {
                    if (folder.typeHash == folderTypeHash && _selection.selected(folder.id)) {
                        desktop::openInExplorer(folder.osPath);
                    }
                }
                break;
            case Command::ShowInExplorer:
                // find paths to show in the explorer
                {
                    vector<zstring_view> files;
                    for (Entry const& asset : _entries) {
                        if (_selection.selected(asset.id)) {
                            files.push_back(asset.osPath);
                        }
                    }
                    desktop::selectInExplorer(_entries[_currentFolder].osPath, files);
                }
                break;
            case Command::EditAsset:
                for (Entry const& asset : _entries) {
                    if (_selection.selected(asset.id) && asset.uuid.isValid()) {
                        _onFileSelected(asset.uuid);
                    }
                }
                break;
            case Command::Trash:
                // recursively deletes folders and files
                {
                    vector<zstring_view> files;
                    for (Entry const& asset : _entries) {
                        if (_selection.selected(asset.id)) {
                            files.push_back(asset.osPath);
                        }
                    }
                    desktop::moveToTrash(files);
                }
                break;
            case Command::Import:
            case Command::ForceImport:
                for (Entry const& asset : _entries) {
                    if (asset.uuid.isValid() && _selection.selected(asset.id)) {
                        _importAsset(asset.uuid, cmd == Command::ForceImport);
                    }
                }
                break;
            case Command::ShowRenameDialog:
                if (_selection.size() == 1) {
                    for (Entry const& entry : _entries) {
                        if (_selection.selected(entry.id)) {
                            nanofmt::format_to(_nameBuffer, "{}", entry.name);
                            nanofmt::format_to(_renameBuffer, "{}", entry.name);
                            break;
                        }
                    }
                }

                ImGui::OpenPopup(AssetEditorRenameDialogName.c_str());
                break;
            case Command::ShowNewFolderDialog:
                _nameBuffer[0] = '\0';
                ImGui::OpenPopup(AssetEditorNewFolderDialogName.c_str());
                break;
            case Command::ShowNewAssetDialog:
                nanofmt::format_to(_nameBuffer, "new.scene");
                _newAssetType = hash_value("potato.asset.scene");
                ImGui::OpenPopup(AssetEditorNewAssetDialogName.c_str());
                break;
            case Command::CreateFolder:
                if (auto const rs = fs::createDirectories(
                        path::join(path::Separator::Native, _entries[_currentFolder].osPath, _nameBuffer));
                    rs != IOResult::Success) {
                    // FIXME: show diagnostics
                }
                break;
            case Command::Rename:
                if (_selection.size() == 1) {
                    for (Entry const& entry : _entries) {
                        if (_selection.selected(entry.id)) {
                            string newPath = path::join(path::parent(entry.osPath), _renameBuffer);
                            if (auto const rs = fs::moveFileTo(entry.osPath, newPath); rs != IOResult::Success) {
                                // FIXME: show diagnostics
                            }
                            break;
                        }
                    }
                }
                break;
            case Command::CreateAsset:
                if (auto const rs = fs::writeAllText(
                        path::join(path::Separator::Native, _entries[_currentFolder].osPath, _nameBuffer),
                        "{}"_sv);
                    rs != IOResult::Success) {
                    // FIXME: show diagnostics
                }
            default:
                break;
        }
    }
} // namespace up::shell
