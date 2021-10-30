// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_browser.h"
#include "editor.h"

#include "potato/editor/desktop.h"
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

namespace up {
    static constexpr zstring_view assetBrowserRenameDialogName = "Rename Files and Folders##asset_browser_rename"_zsv;
    static constexpr zstring_view assetBrowserNewFolderDialogName = "New Folder##asset_browser_new_folder"_zsv;
    static constexpr zstring_view assetBrowserNewAssetDialogName = "New Asset##asset_browser_new_asset"_zsv;
} // namespace up

namespace up::shell {
    namespace {
        class AssetBrowserFactory : public EditorFactory {
        public:
            AssetBrowserFactory(
                AssetLoader& assetLoader,
                ReconClient& reconClient,
                AssetEditService& assetEditService,
                AssetBrowser::OnFileSelected onFileSelected)
                : _assetLoader(assetLoader)
                , _reconClient(reconClient)
                , _assetEditService(assetEditService)
                , _onFileSelected(std::move(onFileSelected)) {}

            zstring_view editorName() const noexcept override { return AssetBrowser::editorName; }

            box<Editor> createEditor() override {
                return new_box<AssetBrowser>(_assetLoader, _reconClient, _assetEditService, _onFileSelected);
            }

            box<Editor> createEditorForDocument(zstring_view) override { return nullptr; }

        private:
            AssetLoader& _assetLoader;
            ReconClient& _reconClient;
            AssetEditService& _assetEditService;
            AssetBrowser::OnFileSelected _onFileSelected;
        };
    } // namespace
} // namespace up::shell

auto up::shell::AssetBrowser::createFactory(
    AssetLoader& assetLoader,
    ReconClient& reconClient,
    AssetEditService& assetEditService,
    AssetBrowser::OnFileSelected onFileSelected) -> box<EditorFactory> {
    return new_box<AssetBrowserFactory>(assetLoader, reconClient, assetEditService, std::move(onFileSelected));
}

void up::shell::AssetBrowser::configure() {
    _rebuild();
}

void up::shell::AssetBrowser::content() {
    if (_manifestRevision != _assetLoader.manifestRevision()) {
        _rebuild();
    }

    if (ImGui::BeginTable(
            "##asset_browser",
            2,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp,
            ImGui::GetContentRegionAvail())) {
        ImGui::TableSetupColumn("##files", 0, 1);
        ImGui::TableSetupColumn("##assets", 0, 4);

        ImGui::TableNextColumn();
        _showTreeFolders();

        ImGui::TableNextColumn();
        _showBreadcrumbs();
        _showAssets(_entries[_currentFolder]);

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered() &&
            ImGui::TableGetHoveredColumn() == 1) {
            ImGui::OpenPopup("##folder_popup");
        }
        if (ImGui::BeginPopup(
                "##folder_popup",
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
            if (ImGui::IconMenuItem("New Folder", ICON_FA_FOLDER)) {
                _command = Command::ShowNewFolderDialog;
            }
            ImGui::EndPopup();
        }

        ImGui::EndTable();
    }

    _showRenameDialog();
    _showNewFolderDialog();
    _showNewAssetDialog();

    _executeCommand();
}

void up::shell::AssetBrowser::_showAssets(Entry const& folder) {
    if (ImGui::BeginIconGrid("##assets")) {
        for (Entry const& entry : _children(folder)) {
            if (entry.typeHash == folderTypeHash) {
                _showFolder(entry);
            }
            else {
                _showAsset(entry);
            }
        }
        ImGui::EndIconGrid();
    }
}

void up::shell::AssetBrowser::_showAsset(Entry const& asset) {
    UP_ASSERT(asset.typeHash != folderTypeHash);

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

    if (ImGui::BeginIconMenuContextPopup()) {
        if (ImGui::IconMenuItem("Edit Asset", ICON_FA_EDIT)) {
            _command = Command::EditAsset;
        }
        if (ImGui::IconMenuItem("Import", ICON_FA_DOWNLOAD)) {
            _command = Command::Import;
        }
        if (ImGui::IconMenuItem("Import (Force)")) {
            _command = Command::ForceImport;
        }
        ImGui::IconMenuSeparator();
        if (ImGui::IconMenuItem("Copy Path", ICON_FA_COPY)) {
            ImGui::SetClipboardText(asset.osPath.c_str());
        }
        if (ImGui::IconMenuItem("Copy UUID")) {
            char buf[UUID::strLength];
            nanofmt::format_to(buf, "{}", asset.uuid);
            ImGui::SetClipboardText(buf);
        }
        if (ImGui::IconMenuItem("Show in Explorer", ICON_FA_FOLDER_OPEN)) {
            _command = Command::ShowInExplorer;
        }
        ImGui::IconMenuSeparator();
        if (ImGui::IconMenuItem("Rename", ICON_FA_PEN)) {
            _command = Command::ShowRenameDialog;
        }
        ImGui::IconMenuSeparator();
        if (ImGui::IconMenuItem("Move to Trash", ICON_FA_TRASH)) {
            _command = Command::Trash;
        }
        ImGui::EndPopup();
    }
}

void up::shell::AssetBrowser::_showFolder(Entry const& folder) {
    UP_ASSERT(folder.typeHash == folderTypeHash);

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

    if (ImGui::BeginIconMenuContextPopup()) {
        if (ImGui::IconMenuItem("Open", ICON_FA_FOLDER_OPEN)) {
            _command = Command::OpenFolder;
        }
        if (ImGui::IconMenuItem("Open in Explorer", nullptr)) {
            _command = Command::OpenInExplorer;
        }
        ImGui::IconMenuSeparator();
        if (ImGui::IconMenuItem("Copy Path", ICON_FA_COPY)) {
            ImGui::SetClipboardText(folder.osPath.c_str());
        }
        ImGui::IconMenuSeparator();
        if (ImGui::IconMenuItem("Rename", ICON_FA_PEN)) {
            _command = Command::ShowRenameDialog;
        }
        ImGui::IconMenuSeparator();
        if (ImGui::IconMenuItem("Move to Trash", ICON_FA_TRASH)) {
            _command = Command::Trash;
        }
        ImGui::EndPopup();
    }
}

void up::shell::AssetBrowser::_showBreadcrumb(int index) {
    if (_entries[index].parentIndex != -1) {
        _showBreadcrumb(_entries[index].parentIndex);
        ImGui::SameLine(0, 0);
        ImGui::TextDisabled("%s", reinterpret_cast<char const*>(ICON_FA_CARET_RIGHT));
        ImGui::SameLine(0, 0);
    }

    if (ImGui::Button(_entries[index].name.c_str())) {
        _openFolder(index);
    }
    ImGui::SameLine();
}

void up::shell::AssetBrowser::_showBreadcrumbs() {
    ImGuiWindow* const window = ImGui::GetCurrentWindow();
    ImDrawList* const drawList = window->DrawList;

    drawList->AddRectFilled(
        window->DC.CursorPos,
        window->DC.CursorPos +
            ImVec2{
                ImGui::GetContentRegionAvail().x,
                ImGui::GetTextLineHeightWithSpacing() + ImGui::GetItemSpacing().y * 3},
        ImGui::GetColorU32(ImGuiCol_Header),
        4.f);

    window->DC.CursorPos += ImGui::GetItemSpacing();
    ImGui::BeginGroup();
    ImGui::PushID("##breadcrumbs");

    if (ImGui::IconButton("New Asset", ICON_FA_FILE)) {
        _command = Command::ShowNewAssetDialog;
    }
    ImGui::SameLine();
    if (ImGui::IconButton("New Folder", ICON_FA_FOLDER)) {
        _command = Command::ShowNewFolderDialog;
    }
    ImGui::SameLine(0.f, 12.f);

    if (ImGui::IconButton("##back", ICON_FA_BACKWARD)) {
        if (_folderHistoryIndex > 0) {
            _currentFolder = _folderHistory[--_folderHistoryIndex];
            _selection.clear();
        }
    }
    ImGui::SameLine();
    if (ImGui::IconButton("##forward", ICON_FA_FORWARD)) {
        if (_folderHistoryIndex + 1 < _folderHistory.size()) {
            _currentFolder = _folderHistory[++_folderHistoryIndex];
            _selection.clear();
        }
    }
    ImGui::SameLine();

    _showBreadcrumb(_currentFolder);

    ImGui::PopID();
    ImGui::EndGroup();
    window->DC.CursorPos.y += ImGui::GetItemSpacing().y;
}

void up::shell::AssetBrowser::_showTreeFolder(int index) {
    int flags = 0;

    UP_ASSERT(_entries[index].typeHash == folderTypeHash);

    bool const hasChildren = _entries[index].firstChild != -1;
    if (!hasChildren) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (index == _currentFolder) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (index == 0) {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    if (ImGui::TreeNodeEx(_entries[index].name.c_str(), flags)) {
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
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

void up::shell::AssetBrowser::_showTreeFolders() {
    if (!_entries.empty()) {
        _showTreeFolder(0);
    }
}

void up::shell::AssetBrowser::_showRenameDialog() {
    if (ImGui::BeginPopupModal(assetBrowserRenameDialogName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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

void up::shell::AssetBrowser::_showNewFolderDialog() {
    if (ImGui::BeginPopupModal(assetBrowserNewFolderDialogName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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

void up::shell::AssetBrowser::_showNewAssetDialog() {
    if (ImGui::BeginPopupModal(assetBrowserNewAssetDialogName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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

void up::shell::AssetBrowser::_rebuild() {
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
        auto const lastSepIndex = record.filename.find_last_of("/"_sv);
        auto const start = lastSepIndex != string::npos ? lastSepIndex + 1 : 0;

        if (record.logicalId != ResourceManifest::Id{}) {
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
    }

    for (auto const& [index, entry] : enumerate(_entries)) {
        if (entry.id == selectedId) {
            _currentFolder = static_cast<int>(index);
            break;
        }
    }
}

int up::shell::AssetBrowser::_addFolder(string_view name, int parentIndex) {
    UP_ASSERT(parentIndex >= 0 && parentIndex < static_cast<int>(_entries.size()));
    UP_ASSERT(_entries[parentIndex].typeHash == folderTypeHash);

    Entry const& parent = _entries[parentIndex];

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

int up::shell::AssetBrowser::_addFolders(string_view folderPath) {
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

void up::shell::AssetBrowser::_openFolder(int index) {
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

    // clear prior selection
    _selection.clear();
}

void up::shell::AssetBrowser::_importAsset(UUID const& uuid, bool force) {
    if (!uuid.isValid()) {
        return;
    }

    schema::ReconImportMessage msg;
    msg.uuid = uuid;
    msg.force = force;
    _reconClient.send<ReconImportMessage>({});
}

void up::shell::AssetBrowser::_executeCommand() {
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

            ImGui::OpenPopup(assetBrowserRenameDialogName.c_str());
            break;
        case Command::ShowNewFolderDialog:
            _nameBuffer[0] = '\0';
            ImGui::OpenPopup(assetBrowserNewFolderDialogName.c_str());
            break;
        case Command::ShowNewAssetDialog:
            nanofmt::format_to(_nameBuffer, "new.scene");
            _newAssetType = hash_value("potato.asset.scene");
            ImGui::OpenPopup(assetBrowserNewAssetDialogName.c_str());
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
