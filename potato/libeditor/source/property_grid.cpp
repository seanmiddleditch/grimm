// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/property_grid.h"

#include "potato/editor/asset_browser_popup.h"
#include "potato/editor/icons.h"
#include "potato/editor/imgui_ext.h"
#include "potato/schema/common_schema.h"
#include "potato/schema/constraint_schema.h"
#include "potato/schema/tools_schema.h"
#include "potato/runtime/asset.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/resource_manifest.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <numeric>

namespace up {
    namespace {
        struct BoolPropertyEditor final : PropertyEditor {
            bool edit(PropertyItemInfo const& info) override {
                return ImGui::Checkbox("##bool", static_cast<bool*>(info.object));
            }
        };

        struct IntegerPropertyEditor final : PropertyEditor {
            bool edit(PropertyItemInfo const& info) override {
                switch (info.schema.primitive) {
                    case reflex::SchemaPrimitive::Int8:
                        return _edit(info, ImGuiDataType_S8, *static_cast<int8*>(info.object));
                    case reflex::SchemaPrimitive::UInt8:
                        return _edit(info, ImGuiDataType_U8, *static_cast<uint8*>(info.object));
                    case reflex::SchemaPrimitive::Int16:
                        return _edit(info, ImGuiDataType_S16, *static_cast<int16*>(info.object));
                    case reflex::SchemaPrimitive::UInt16:
                        return _edit(info, ImGuiDataType_U16, *static_cast<uint16*>(info.object));
                    case reflex::SchemaPrimitive::Int32:
                        return _edit(info, ImGuiDataType_S32, *static_cast<int32*>(info.object));
                    case reflex::SchemaPrimitive::UInt32:
                        return _edit(info, ImGuiDataType_U32, *static_cast<uint32*>(info.object));
                    case reflex::SchemaPrimitive::Int64:
                        return _edit(info, ImGuiDataType_S64, *static_cast<int64*>(info.object));
                    case reflex::SchemaPrimitive::UInt64:
                        return _edit(info, ImGuiDataType_U64, *static_cast<uint64*>(info.object));
                    default:
                        return false;
                }
            }

        private:
            template <integral IntT>
            bool _edit(PropertyItemInfo const& info, ImGuiDataType imguiType, IntT& value) noexcept {
                ImGui::SetNextItemWidth(-1.f);

                if (info.field != nullptr) {
                    auto const* const rangeAnnotation = reflex::queryAnnotation<up::schema::IntRange>(*info.field);
                    if (rangeAnnotation != nullptr) {
                        auto const minValue = narrow_cast<IntT>(rangeAnnotation->min);
                        auto const maxValue = narrow_cast<IntT>(rangeAnnotation->max);
                        return ImGui::SliderScalar("##value", imguiType, &value, &minValue, &maxValue);
                    }
                }

                return ImGui::InputScalar("##value", imguiType, &value);
            }
        };

        struct FloatPropertyEditor final : PropertyEditor {
            bool edit(PropertyItemInfo const& info) override {
                switch (info.schema.primitive) {
                    case reflex::SchemaPrimitive::Float:
                        return _edit(info, ImGuiDataType_Float, *static_cast<float*>(info.object));
                    case reflex::SchemaPrimitive::Double:
                        return _edit(info, ImGuiDataType_Double, *static_cast<double*>(info.object));
                    default:
                        return false;
                }
            }

        private:
            template <typename FloatT>
            bool _edit(PropertyItemInfo const& info, ImGuiDataType imguiType, FloatT& value) noexcept {
                ImGui::SetNextItemWidth(-1.f);

                if (info.field != nullptr) {
                    auto const* const rangeAnnotation = reflex::queryAnnotation<up::schema::FloatRange>(*info.field);
                    if (rangeAnnotation != nullptr) {
                        auto const minValue = narrow_cast<FloatT>(rangeAnnotation->min);
                        auto const maxValue = narrow_cast<FloatT>(rangeAnnotation->max);
                        return ImGui::SliderScalar(
                            "##value",
                            imguiType,
                            &value,
                            &minValue,
                            &maxValue,
                            "%f",
                            ImGuiSliderFlags_AlwaysClamp);
                    }
                }

                return ImGui::InputScalar("##value", imguiType, &value);
            }

            template <integral IntT>
            bool _edit(PropertyItemInfo const& info, IntT& value) noexcept {
                int tmp = static_cast<int>(value);
                bool const edit = _edit(info, tmp);
                value = static_cast<IntT>(tmp);
                return edit;
            }
        };

        struct Vec3PropertyEditor final : PropertyEditor {
            bool edit(PropertyItemInfo const& info) override {
                ImGui::SetNextItemWidth(-1.f);
                return ImGui::InputVec3("##vec3", *static_cast<glm::vec3*>(info.object));
            }
        };

        struct QuaternionPropertyEditor final : PropertyEditor {
            bool edit(PropertyItemInfo const& info) override {
                ImGui::SetNextItemWidth(-1.f);
                return ImGui::InputQuat("##quat", *static_cast<glm::quat*>(info.object));
            }
        };

        struct StringPropertyEditor final : PropertyEditor {
            bool edit(PropertyItemInfo const& info) override {
                ImGui::SetNextItemWidth(-1.f);

                auto& value = *static_cast<string*>(info.object);

                // FIXME:
                // up::string is an immutable string type, which isn't easy to make editable
                // in a well-performing way. we ideally want to know when a string is being
                // edited, make a temporary copy into a cheaply-resizable buffer, then post-
                // edit copy that back into a new up::string. For now... just this.
                char buffer[512];
                nanofmt::format_to(buffer, "{}", value);

                if (ImGui::InputText("##string", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    value = string(buffer);
                    return true;
                }

                return false;
            }
        };

        struct ArrayPropertyEditor final : PropertyEditor {
            bool edit(PropertyItemInfo const& info) override {
                if (info.schema.operations != nullptr && info.schema.operations->arrayGetSize != nullptr) {
                    auto const size = info.schema.operations->arrayGetSize(info.object);
                    ImGui::Text("%zu %s", size, size == 1 ? "item" : "items");
                }
                else {
                    ImGui::Text("<%s>", info.schema.name.c_str());
                }
                return false;
            }

            PropertyExpandable expandable(PropertyItemInfo const& info) override {
                if (info.schema.operations != nullptr && info.schema.operations->arrayGetSize != nullptr &&
                    info.schema.operations->arrayGetSize(info.object) != 0) {
                    return PropertyExpandable::Children;
                }
                return PropertyExpandable::Empty;
            }

            bool children(PropertyGrid& propertyGrid, PropertyItemInfo const& info) override {
                if (!UP_VERIFY(info.schema.operations != nullptr)) {
                    return false;
                }

                if (!UP_VERIFY(
                        info.schema.operations->arrayGetSize != nullptr &&
                        info.schema.operations->arrayMutableElementAt != nullptr)) {
                    return false;
                }

                size_t const size = info.schema.operations->arrayGetSize(info.object);
                bool const canRemoveElement = info.schema.operations->arrayEraseAt != nullptr;
                bool const canMoveElement = info.schema.operations->arrayMoveTo != nullptr;

                bool wantRemove = false;
                size_t removeIndex = 0;

                bool wantMove = false;
                size_t moveFromIndex = 0;
                size_t moveToIndex = 0;

                bool edits = false;

                for (size_t index = 0; index != size; ++index) {
                    PropertyItemInfo::ArrayOps ops{.canRemove = canRemoveElement, .canMove = canMoveElement};
                    PropertyItemInfo const elementInfo{
                        .schema = *info.schema.elementType,
                        .object = info.schema.operations->arrayMutableElementAt(info.object, index),
                        .index = narrow_cast<int>(index),
                        .arrayOps = &ops};

                    ImGui::PushID(static_cast<int>(index));

                    edits |= propertyGrid.editItem(elementInfo);
                    if (ops.wantRemove) {
                        wantRemove = true;
                        removeIndex = index;
                    }
                    if (ops.moveFromIndex >= 0) {
                        wantMove = true;
                        moveToIndex = index;
                        moveFromIndex = ops.moveFromIndex;
                    }

                    ImGui::PopID();
                }

                if (wantRemove && canRemoveElement) {
                    info.schema.operations->arrayEraseAt(info.object, removeIndex);
                    edits = true;
                }
                else if (wantMove && canMoveElement) {
                    info.schema.operations->arrayMoveTo(info.object, moveToIndex, moveFromIndex);
                    edits = true;
                }

                return edits;
            }

            bool canAddItem(PropertyItemInfo const& info) override {
                return info.schema.operations != nullptr && info.schema.operations->arrayGetSize != nullptr &&
                    info.schema.operations->arrayResize != nullptr;
            }

            void addItem(PropertyItemInfo const& info) override {
                info.schema.operations->arrayResize(info.object, info.schema.operations->arrayGetSize(info.object) + 1);
            }
        };

        struct ObjectPropertyEditor final : PropertyEditor {
            bool edit(PropertyItemInfo const& info) override {
                ImGui::Text("<%s>", info.schema.name.c_str());
                return false;
            }

            PropertyExpandable expandable(PropertyItemInfo const& info) override {
                if (info.schema.fields.empty()) {
                    return PropertyExpandable::None;
                }
                return PropertyExpandable::Children;
            }

            bool children(PropertyGrid& propertyGrid, PropertyItemInfo const& info) override {
                bool edits = false;
                for (reflex::SchemaField const& field : info.schema.fields) {
                    edits |= propertyGrid.editItem({
                        .schema = *field.schema,
                        .object = static_cast<char*>(info.object) + field.offset,
                        .field = &field,
                    });
                }
                return edits;
            }
        };

        struct PointerPropertyEditor final : PropertyEditor {
            bool edit(PropertyItemInfo const& info) override {
                if (info.schema.operations != nullptr && info.schema.operations->pointerDeref(info.object) != nullptr) {
                    ImGui::Text("<%s>", info.schema.elementType->name.c_str());
                }
                else {
                    ImGui::Text("empty :: <%s>", info.schema.elementType->name.c_str());
                }
                return false;
            }

            PropertyExpandable expandable(PropertyItemInfo const& info) override {
                if (info.schema.operations != nullptr && info.schema.operations->pointerDeref(info.object) != nullptr) {
                    return PropertyExpandable::Children;
                }
                return PropertyExpandable::Empty;
            }

            bool children(PropertyGrid& propertyGrid, PropertyItemInfo const& info) override {
                if (void* const pointee = info.schema.operations->pointerMutableDeref(info.object);
                    pointee != nullptr) {
                    return propertyGrid.editItem({
                        .schema = *info.schema.elementType,
                        .object = pointee,
                        .index = info.index,
                        .arrayOps = info.arrayOps,
                    });
                }
                return false;
            }
        };

        struct UuidPropertyEditor final : PropertyEditor {
            bool edit(PropertyItemInfo const& info) override {
                ImGui::SetNextItemWidth(-1.f);

                auto& value = *static_cast<UUID*>(info.object);

                char buffer[UUID::strLength];
                nanofmt::format_to(buffer, "{}", value);

                if (ImGui::InputText(
                        "##uuid",
                        buffer,
                        sizeof(buffer),
                        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter,
                        &_callback,
                        this)) {
                    value = UUID::fromString(buffer);
                    return true;
                }

                return false;
            }

            static int _callback(ImGuiInputTextCallbackData* data) {
                switch (data->EventFlag) {
                    case ImGuiInputTextFlags_CallbackCharFilter:
                        if (data->EventChar <= 255) {
                            char const ch = static_cast<char>(data->EventChar);
                            return ascii::is_hex_digit(ch) || ch == '-' ? 0 : 1;
                        }
                        return 1;
                    default:
                        return 0;
                }
            }
        };

        struct AssetRefPropertyEditor final : PropertyEditor {
            explicit AssetRefPropertyEditor(AssetLoader& assetLoader) noexcept : _assetLoader(assetLoader) { }

            bool edit(PropertyItemInfo const& info) override {
                UP_GUARD(info.schema.primitive == reflex::SchemaPrimitive::AssetRef, false);

                zstring_view assetType{};
                if (auto const* const assetTypeAnno = reflex::queryAnnotation<schema::AssetType>(info.schema);
                    assetTypeAnno != nullptr) {
                    assetType = assetTypeAnno->assetType;
                }

                auto* const handle = static_cast<UntypedAssetHandle*>(info.object);
                AssetId const assetId = handle->assetId();
                zstring_view displayName = "<empty>"_zsv;

                if (handle->isSet()) {
                    displayName = _assetLoader.debugName(assetId);
                }

                bool edit = false;

                ImGui::Text("%s", displayName.c_str());
                ImGui::SameLine();
                if (ImGui::IconButton("##clear", ICON_FA_TRASH) && info.schema.operations->pointerAssign != nullptr) {
                    info.schema.operations->pointerAssign(info.object, nullptr);
                    edit = true;
                }
                ImGui::SameLine();

                char browserId[32] = {
                    0,
                };
                nanofmt::format_to(browserId, "##assets{}", ImGui::GetID("popup"));

                if (ImGui::IconButton("##select", ICON_FA_FOLDER)) {
                    ImGui::OpenPopup(browserId);
                }

                if (info.schema.operations != nullptr && info.schema.operations->pointerAssign != nullptr) {
                    AssetId targetAssetId = assetId;
                    if (up::assetBrowserPopup(browserId, targetAssetId, assetType, _assetLoader) &&
                        targetAssetId != assetId) {
                        *handle = _assetLoader.loadAssetSync(targetAssetId, assetType);
                        edit = true;
                    }
                }

                return edit;
            }

        private:
            AssetLoader& _assetLoader;
        };
    } // namespace

    PropertyGrid::PropertyGrid(AssetLoader& assetLoader) noexcept {
        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<BoolPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Bool, index);
        }

        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<IntegerPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Int8, index);
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::UInt8, index);
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Int16, index);
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::UInt16, index);
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Int32, index);
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::UInt32, index);
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Int64, index);
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::UInt64, index);
        }

        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<FloatPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Float, index);
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Double, index);
        }

        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<Vec3PropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Vec3, index);
        }

        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<QuaternionPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Quat, index);
        }

        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<StringPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::String, index);
        }

        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<ArrayPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Array, index);
        }

        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<ObjectPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Object, index);
        }

        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<PointerPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Pointer, index);
        }

        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<UuidPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Uuid, index);
        }

        {
            auto const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<AssetRefPropertyEditor>(assetLoader));
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::AssetRef, index);
        }
    }

    void PropertyGrid::addPropertyEditor(box<PropertyEditor> editor) { _propertyEditors.push_back(std::move(editor)); }

    bool PropertyGrid::beginTable(char const* label) {
        bool const open = ImGui::BeginTable(
            label != nullptr ? label : "##property_grid",
            2,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp);
        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_None, 0.4f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None, 0.6f);
        return open;
    }

    void PropertyGrid::endTable() { ImGui::EndTable(); }

    bool PropertyGrid::editObjectRaw(reflex::Schema const& schema, void* object) {
        PropertyEditor* const editor = findPropertyEditor(schema);
        if (!UP_VERIFY(editor != nullptr)) {
            ImGui::TextColored(ImVec4{1.f, 0.f, 0.f, 1.f}, "Unsupported Schema type");
            return false;
        }

        return editor->children(*this, {.schema = schema, .object = object});
    }

    bool PropertyGrid::editItem(PropertyItemInfo const& info) {
        ImGui::TableNextRow();

        PropertyEditor* const editor = findPropertyEditor(info.schema);
        if (UP_VERIFY(editor != nullptr)) {
            if (info.index >= 0) {
                ImGui::PushID(info.index);
            }
            else if (info.field != nullptr) {
                ImGui::PushID(info.field->name.c_str());
            }
            else {
                ImGui::PushID(info.schema.name.c_str());
            }

            bool const edits = _editInternal(*editor, info);

            ImGui::PopID();

            return edits;
        }

        ImGui::TableSetColumnIndex(0);
        _showLabel(info);

        ImGui::TableSetColumnIndex(1);
        ImGui::TextColored(ImVec4{1.f, 0.f, 0.f, 1.f}, "Unsupported Schema type");

        return false;
    }

    PropertyEditor* PropertyGrid::findPropertyEditor(reflex::Schema const& schema) const noexcept {
        auto const primIt = _primitiveEditorMap.find(schema.primitive);
        if (primIt) {
            return _propertyEditors[primIt->value].get();
        }
        return nullptr;
    }

    bool PropertyGrid::_editInternal(PropertyEditor& propertyEditor, PropertyItemInfo const& info) {
        // behavior overrides
        if (info.field != nullptr) {
            if (reflex::queryAnnotation<schema::Hidden>(*info.field) != nullptr) {
                return false;
            }

            if (reflex::queryAnnotation<schema::Flatten>(*info.field) != nullptr) {
                return editObjectRaw(info.schema, info.object);
            }
        }

        bool edits = false;
        bool open = false;

        // label
        {
            ImGui::TableSetColumnIndex(0);

            // expand for objects/arrays, or drag for reordering array elements
            {
                auto const expandable = propertyEditor.expandable(info);

                if (expandable == PropertyExpandable::Children) {
                    ImGuiTreeNodeFlags const flags = ImGuiTreeNodeFlags_DefaultOpen |
                        ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth |
                        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap |
                        ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding;
                    open = ImGui::TreeNodeEx("##expand", flags);
                    ImGui::SameLine();
                }
                else if (expandable == PropertyExpandable::Empty) {
                    ImGuiTreeNodeFlags const flags = ImGuiTreeNodeFlags_NoTreePushOnOpen |
                        ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
                        ImGuiTreeNodeFlags_FramePadding;
                    ImGui::BeginDisabled(true);
                    ImGui::SetNextItemOpen(false, ImGuiCond_Always);
                    open = ImGui::TreeNodeEx("##empty", flags);
                    ImGui::SameLine();
                    ImGui::EndDisabled();
                }
                else if (info.arrayOps != nullptr && info.arrayOps->canMove) {
                    ImGui::Interactive("##drag", ImGuiInteractiveFlags_AllowItemOverlap);
                    ImGui::SameLine();

                    if (ImGui::BeginDragDropTarget()) {
                        if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("reorder"); payload != nullptr) {
                            info.arrayOps->moveFromIndex = *static_cast<int const*>(payload->Data);
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if (ImGui::BeginDragDropSource(
                            ImGuiDragDropFlags_SourceAutoExpirePayload | ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("reorder", &info.index, sizeof(info.index));
                        _showLabel(info);
                        ImGui::EndDragDropSource();
                    }
                }
                else {
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetItemSpacing().x);
                }
            }

            // label buttons
            {
                auto const windowPos = ImGui::GetWindowPos();
                auto availSize = ImGui::GetContentRegionAvail();
                float const buttonWidth = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.x * 2.f;
                float const buttonSpacing = ImGui::GetStyle().ItemInnerSpacing.x;
                auto const pos = ImGui::GetCursorPos();

                // add row icon for arrays
                if (propertyEditor.canAddItem(info)) {
                    availSize.x -= buttonWidth + buttonSpacing;
                    ImGui::SetCursorPos({pos.x + availSize.x + buttonSpacing, pos.y});
                    if (ImGui::IconButton("##add", ICON_FA_PLUS)) {
                        propertyEditor.addItem(info);
                    }
                }

                // delete row icon for array elements
                if (info.arrayOps != nullptr && info.arrayOps->canRemove) {
                    availSize.x -= buttonWidth + buttonSpacing;
                    ImGui::SetCursorPos({pos.x + availSize.x + buttonSpacing, pos.y});
                    if (ImGui::IconButton("##remove", ICON_FA_TRASH)) {
                        info.arrayOps->wantRemove = true;
                    }
                }

                ImGui::SetCursorPos(pos);
                ImGui::PushClipRect(
                    ImVec2(windowPos.x + pos.x, windowPos.y + pos.y),
                    ImVec2(windowPos.x + pos.x + availSize.x, windowPos.y + pos.y + availSize.y),
                    true);
            }

            // label
            {
                ImGui::AlignTextToFramePadding();
                _showLabel(info);

                ImGui::PopClipRect();
            }

            // label tooltip
            if (info.field != nullptr && ImGui::IsItemHovered()) {
                if (auto const* const tooltipAnnotation = reflex::queryAnnotation<schema::Tooltip>(*info.field);
                    tooltipAnnotation != nullptr) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", tooltipAnnotation->text.c_str());
                    ImGui::EndTooltip();
                }
            }
        }

        // editor
        {
            ImGui::TableSetColumnIndex(1);
            ImGui::AlignTextToFramePadding();

            edits |= propertyEditor.edit(info);
        }

        // recurse into children
        if (open) {
            ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
            propertyEditor.children(*this, info);
            ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
        }

        return edits;
    }

    void PropertyGrid::_showLabel(PropertyItemInfo const& info) noexcept {
        if (info.index >= 0) {
            char buffer[64];
            nanofmt::format_to(buffer, "{}", info.index + 1);
            ImGui::TextUnformatted(buffer);
            return;
        }

        if (info.field == nullptr) {
            ImGui::TextUnformatted("<Unknown>");
            return;
        }

        if (auto const* const displayNameAnnotation = reflex::queryAnnotation<schema::DisplayName>(*info.field);
            displayNameAnnotation != nullptr && !displayNameAnnotation->name.empty()) {
            ImGui::TextUnformatted(displayNameAnnotation->name.c_str());
            return;
        }

        // format field names pretty
        {
            bool first = true;
            char const* ch = info.field->name.c_str();
            while (*ch != '\0') {
                if (*ch == '_') {
                    ++ch;
                    continue;
                }

                if (!first) {
                    ImGui::TextUnformatted(" ");
                    ImGui::SameLine(0.f, 0.f);
                }
                first = false;

                char const* start = ch;

                if (ascii::is_digit(*ch)) {
                    ++ch;
                    while (ascii::is_digit(*ch)) {
                        ++ch;
                    }
                }
                else if (ascii::is_upper(*ch)) {
                    ++ch;
                    if (ascii::is_upper(*ch)) {
                        ++ch;
                        while (ascii::is_upper(*ch)) {
                            ++ch;
                        }
                    }
                    else if (ascii::is_lower(*ch)) {
                        ++ch;
                        while (ascii::is_lower(*ch)) {
                            ++ch;
                        }
                    }
                }
                else {
                    ImGui::Text("%c", ascii::toUppercase(*ch));
                    ImGui::SameLine(0.f, 0.f);

                    start = ++ch;
                    while (ascii::is_lower(*ch)) {
                        ++ch;
                    }
                }

                ImGui::TextUnformatted(start, ch);
                ImGui::SameLine(0.f, 0.f);
            }
        }
    }
} // namespace up
