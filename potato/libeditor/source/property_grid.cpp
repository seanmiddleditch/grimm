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
#include <numeric>

namespace up {
    namespace {
        struct DefaultPropertyEditor final : PropertyEditor {
            bool edit(PropertyInfo const& info) override {
                ImGui::Text("<%s>", info.schema.name.c_str());
                return false;
            }
        };

        struct BoolPropertyEditor final : PropertyEditor {
            bool edit(PropertyInfo const& info) override {
                return ImGui::Checkbox("##bool", static_cast<bool*>(info.object));
            }
        };

        struct IntegerPropertyEditor final : PropertyEditor {
            bool edit(PropertyInfo const& info) override {
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
            bool _edit(PropertyInfo const& info, ImGuiDataType imguiType, IntT& value) noexcept {
                ImGui::SetNextItemWidth(-1.f);

                auto const* const rangeAnnotation = queryAnnotation<up::schema::IntRange>(info.field);
                if (rangeAnnotation != nullptr) {
                    auto const minValue = narrow_cast<IntT>(rangeAnnotation->min);
                    auto const maxValue = narrow_cast<IntT>(rangeAnnotation->max);
                    return ImGui::SliderScalar("##value", imguiType, &value, &minValue, &maxValue);
                }

                return ImGui::InputScalar("##value", imguiType, &value);
            }
        };

        struct FloatPropertyEditor final : PropertyEditor {
            bool edit(PropertyInfo const& info) override {
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
            bool _edit(PropertyInfo const& info, ImGuiDataType imguiType, FloatT& value) noexcept {
                ImGui::SetNextItemWidth(-1.f);

                auto const* const rangeAnnotation = queryAnnotation<up::schema::FloatRange>(info.field);
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

                return ImGui::InputScalar("##value", imguiType, &value);
            }

            template <integral IntT>
            bool _edit(PropertyInfo const& info, IntT& value) noexcept {
                int tmp = static_cast<int>(value);
                bool const edit = _edit(info, tmp);
                value = static_cast<IntT>(tmp);
                return edit;
            }
        };

        struct Vec3PropertyEditor final : PropertyEditor {
            bool edit(PropertyInfo const& info) override {
                ImGui::SetNextItemWidth(-1.f);
                return ImGui::InputVec3("##vec3", *static_cast<glm::vec3*>(info.object));
            }
        };

        struct QuaternionPropertyEditor final : PropertyEditor {
            bool edit(PropertyInfo const& info) override {
                ImGui::SetNextItemWidth(-1.f);
                return ImGui::InputQuat("##quat", *static_cast<glm::quat*>(info.object));
            }
        };

        struct StringPropertyEditor final : PropertyEditor {
            bool edit(PropertyInfo const& info) override {
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
            bool edit(PropertyInfo const& info) override {
                if (info.schema.operations != nullptr && info.schema.operations->arrayGetSize != nullptr) {
                    auto const size = info.schema.operations->arrayGetSize(info.object);
                    ImGui::Text("%zu %s", size, size == 1 ? "item" : "items");
                }
                else {
                    ImGui::Text("<%s>");
                }
                return false;
            }
        };

        struct UuidPropertyEditor final : PropertyEditor {
            bool edit(PropertyInfo const& info) override {
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

            bool edit(PropertyInfo const& info) override {
                ImGui::BeginGroup();
                UP_GUARD(info.schema.primitive == reflex::SchemaPrimitive::AssetRef, false);

                zstring_view assetType{};
                if (auto const* const assetTypeAnno = queryAnnotation<schema::AssetType>(info.schema);
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

                ImGui::EndGroup();

                return edit;
            }

        private:
            AssetLoader& _assetLoader;
        };
    } // namespace

    struct PropertyGrid::ArrayOps {
        int moveFromIndex = -1;
        int moveToIndex = -1;
        bool canMove = false;
    };

    struct PropertyGrid::ItemState {
        bool open = false;
        bool wantAdd = false;
        bool wantRemove = false;
    };

    PropertyGrid::PropertyGrid(AssetLoader& assetLoader) noexcept {
        // default editor should always be index 0
        _propertyEditors.push_back(new_box<DefaultPropertyEditor>());

        {
            uint32 const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<BoolPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Bool, index);
        }

        {
            uint32 const index = static_cast<uint32>(_propertyEditors.size());
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
            uint32 const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<FloatPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Float, index);
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Double, index);
        }

        {
            uint32 const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<Vec3PropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Vec3, index);
        }

        {
            uint32 const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<QuaternionPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Quat, index);
        }

        {
            uint32 const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<StringPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::String, index);
        }

        {
            uint32 const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<ArrayPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Array, index);
        }

        {
            uint32 const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<UuidPropertyEditor>());
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::Uuid, index);
        }

        {
            uint32 const index = static_cast<uint32>(_propertyEditors.size());
            _propertyEditors.push_back(new_box<AssetRefPropertyEditor>(assetLoader));
            _primitiveEditorMap.insert(reflex::SchemaPrimitive::AssetRef, index);
        }
    }

    void PropertyGrid::addPropertyEditor(box<PropertyEditor> editor) { _propertyEditors.push_back(std::move(editor)); }

    // bool PropertyGrid::_editField(PropertyInfo const& info) {
    //     bool edit = false;

    //    switch (info.schema.primitive) {
    //        case reflex::SchemaPrimitive::Pointer:
    //            if (info.schema.operations->pointerMutableDeref != nullptr) {
    //                if (void* pointee = info.schema.operations->pointerMutableDeref(info.object)) {
    //                    edit = _editField({.field = info.field, .schema = *info.schema.elementType, .object =
    //                    pointee}); break;
    //                }
    //            }
    //            edit = false;
    //            break;
    //        default:
    //            ImGui::Text("Unsupported primitive type for schema `%s`", info.schema.name.c_str());
    //            break;
    //    }

    //    return edit;
    //}

    bool PropertyGrid::beginTable() {
        bool const open =
            ImGui::BeginTable("##property_grid", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp);
        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_None, 0.4f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None, 0.6f);
        return open;
    }

    void PropertyGrid::endTable() { ImGui::EndTable(); }

    bool PropertyGrid::editObjectRaw(reflex::Schema const& schema, void* object) {
        UP_GUARD(schema.primitive == reflex::SchemaPrimitive::Object, false);

        bool edits = false;

        for (reflex::SchemaField const& field : schema.fields) {
            edits |= _editProperty(
                {.field = field, .schema = *field.schema, .object = static_cast<char*>(object) + field.offset});
        }

        return edits;
    }

    bool PropertyGrid::_editElements(PropertyInfo const& info, ItemState& state) {
        if (!UP_VERIFY(info.schema.operations != nullptr)) {
            return false;
        }

        if (!UP_VERIFY(
                info.schema.operations->arrayGetSize != nullptr &&
                info.schema.operations->arrayMutableElementAt != nullptr)) {
            return false;
        }

        size_t const size = info.schema.operations->arrayGetSize(info.object);
        ArrayOps ops;
        ops.canMove = info.schema.operations->arrayMoveTo != nullptr;
        bool const canRemoveElement = info.schema.operations->arrayEraseAt != nullptr;

        bool wantRemove = false;
        size_t removeIndex = 0;

        bool edits = false;

        for (size_t index = 0; index != size; ++index) {
            PropertyInfo const elementInfo{
                .field = info.field,
                .schema = *info.schema.elementType,
                .object = info.schema.operations->arrayMutableElementAt(info.object, index),
                .index = narrow_cast<int>(index),
                .canRemove = canRemoveElement};

            ImGui::PushID(static_cast<int>(index));
            ImGui::TableNextRow();

            ItemState state{};
            edits |= _editField(elementInfo, state, &ops);
            if (state.wantRemove) {
                wantRemove = true;
                removeIndex = index;
            }

            ImGui::PopID();
        }

        if (wantRemove) {
            info.schema.operations->arrayEraseAt(info.object, removeIndex);
            edits = true;
        }
        else if (ops.moveToIndex >= 0 && ops.moveFromIndex >= 0) {
            info.schema.operations->arrayMoveTo(info.object, ops.moveToIndex, ops.moveFromIndex);
            edits = true;
        }

        return edits;
    }

    bool PropertyGrid::_editProperty(PropertyInfo const& info) {
        if (info.schema.primitive == reflex::SchemaPrimitive::Object &&
            queryAnnotation<schema::Flatten>(info.field) != nullptr) {
            return editObjectRaw(info.schema, info.object);
        }

        if (queryAnnotation<schema::Hidden>(info.field) != nullptr) {
            return false;
        }

        ImGui::PushID(info.field.name.c_str());
        ImGui::TableNextRow();

        ItemState state{};
        bool const edits = _editField(info, state);

        ImGui::PopID();

        return edits;
    }

    void PropertyGrid::_label(PropertyInfo const& info, ItemState& state, ArrayOps* ops) noexcept {
        bool const isExpandable = info.schema.primitive == reflex::SchemaPrimitive::Object ||
            info.schema.primitive == reflex::SchemaPrimitive::Array;

        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();

        // expand for objects/arrays, or drag for reordering array elements
        if (isExpandable) {
            ImGuiTreeNodeFlags const flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            state.open = ImGui::TreeNodeEx("##expand", flags);
            ImGui::SameLine();
        }
        else if (ops != nullptr && ops->canMove) {
            ImGuiTreeNodeFlags const flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf |
                ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx("##expand", flags);
            ImGui::SameLine();

            if (ImGui::BeginDragDropTarget()) {
                if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("reorder"); payload != nullptr) {
                    ops->moveFromIndex = *static_cast<int const*>(payload->Data);
                    ops->moveToIndex = info.index;
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAutoExpirePayload)) {
                ImGui::SetDragDropPayload("reorder", &info.index, sizeof(info.index));
                ImGui::Text("%d", info.index + 1);
                ImGui::EndDragDropSource();
            }
        }

        // button bar and text
        {
            auto const windowPos = ImGui::GetWindowPos();
            auto availSize = ImGui::GetContentRegionAvail();
            float const buttonWidth = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.x * 2.f;
            float const buttonSpacing = ImGui::GetStyle().ItemInnerSpacing.x;
            auto const pos = ImGui::GetCursorPos();

            // add row icon for arrays
            if (info.schema.operations != nullptr && info.schema.operations->arrayResize != nullptr) {
                availSize.x -= buttonWidth;
                ImGui::SetCursorPos({pos.x + availSize.x, pos.y});
                availSize.x -= buttonSpacing;
                if (ImGui::IconButton("##add", ICON_FA_PLUS)) {
                    state.wantAdd = true;
                }
            }

            // delete row icon for array elements
            if (info.index >= 0) {
                availSize.x -= buttonWidth;
                ImGui::SetCursorPos({pos.x + availSize.x, pos.y});
                availSize.x -= buttonSpacing;
                if (ImGui::IconButton("##remove", ICON_FA_TRASH)) {
                    state.wantRemove = true;
                }
            }

            ImGui::SetCursorPos(pos);
            ImGui::PushClipRect(
                ImVec2(windowPos.x + pos.x, windowPos.y + pos.y),
                ImVec2(windowPos.x + pos.x + availSize.x, windowPos.y + pos.y + availSize.y),
                true);

            // text label
            if (info.index >= 0) {
                ImGui::Text("%d", info.index + 1);
            }
            else if (auto const* const displayNameAnnotation = queryAnnotation<schema::DisplayName>(info.field);
                     displayNameAnnotation != nullptr && !displayNameAnnotation->name.empty()) {
                ImGui::Text("%s", displayNameAnnotation->name.c_str());
            }
            else {
                ImGui::Text("%s", info.field.name.c_str());
            }

            ImGui::PopClipRect();
        }

        // label tooltip
        if (ImGui::IsItemHovered()) {
            if (auto const* const tooltipAnnotation = queryAnnotation<schema::Tooltip>(info.field);
                tooltipAnnotation != nullptr) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", tooltipAnnotation->text.c_str());
                ImGui::EndTooltip();
            }
        }
    }

    bool PropertyGrid::_applyState(PropertyInfo const& info, ItemState const& state) {
        if (state.wantAdd && info.schema.operations != nullptr && info.schema.operations->arrayResize != nullptr &&
            info.schema.operations->arrayGetSize != nullptr) {
            info.schema.operations->arrayResize(info.object, info.schema.operations->arrayGetSize(info.object) + 1);
            return true;
        }
        return false;
    }

    bool PropertyGrid::_editField(PropertyInfo const& info, ItemState& state, ArrayOps* ops) {
        PropertyEditor* const propertyEditor = _selectEditor(info);
        UP_ASSERT(propertyEditor != nullptr);

        bool edits = false;

        _label(info, state, ops);
        edits |= _applyState(info, state);

        ImGui::TableSetColumnIndex(1);
        ImGui::AlignTextToFramePadding();

        edits |= propertyEditor->edit(info);

        if (state.open) {
            ImGui::Indent();
            if (info.schema.primitive == reflex::SchemaPrimitive::Object) {
                edits |= editObjectRaw(info.schema, info.object);
            }
            else if (info.schema.primitive == reflex::SchemaPrimitive::Array) {
                edits |= _editElements(info, state);
            }
            ImGui::Unindent();
        }

        return edits;
    }

    PropertyEditor* PropertyGrid::_selectEditor(PropertyInfo const& info) noexcept {
        auto const primIt = _primitiveEditorMap.find(info.schema.primitive);
        if (primIt) {
            return _propertyEditors[primIt->value].get();
        }

        return _propertyEditors.front().get();
    }
} // namespace up
