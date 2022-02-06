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

    void PropertyEditor::label(PropertyInfo const& info) {
        auto const* const displayNameAnnotation = queryAnnotation<schema::DisplayName>(info.field);
        zstring_view const displayName = displayNameAnnotation != nullptr && !displayNameAnnotation->name.empty()
            ? displayNameAnnotation->name
            : info.field.name;

        ImGui::Text("%s", displayName.c_str());

        auto const* const tooltipAnnotation = queryAnnotation<schema::Tooltip>(info.field);
        if (tooltipAnnotation != nullptr && ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", tooltipAnnotation->text.c_str());
            ImGui::EndTooltip();
        }
    }

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

    bool PropertyGrid::_editField(reflex::SchemaField const& field, reflex::Schema const& schema, void* object) {
        ImGui::PushID(object);

        bool edit = false;

        switch (schema.primitive) {
            case reflex::SchemaPrimitive::Pointer:
                if (schema.operations->pointerMutableDeref != nullptr) {
                    if (void* pointee = schema.operations->pointerMutableDeref(object)) {
                        edit = _editField(field, *schema.elementType, pointee);
                        break;
                    }
                }
                edit = false;
                break;
            case reflex::SchemaPrimitive::Array:
                edit = _editArrayField(field, schema, object);
                break;
            case reflex::SchemaPrimitive::Object:
                edit = _drawObjectEditor(schema, object);
                break;
            default:
                ImGui::Text("Unsupported primitive type for schema `%s`", schema.name.c_str());
                break;
        }

        ImGui::PopID();

        return edit;
    }

    bool PropertyGrid::_drawObjectEditor(reflex::Schema const& schema, void* object) {
        ImGui::TextDisabled("%s", schema.name.c_str());

        return _editProperties(schema, object);
    }

    bool PropertyGrid::_editArrayField(reflex::SchemaField const& field, reflex::Schema const& schema, void* object) {
        if (schema.operations == nullptr) {
            ImGui::TextDisabled("Unsupported type");
            return false;
        }

        if (schema.operations->arrayGetSize == nullptr || schema.operations->arrayElementAt == nullptr) {
            ImGui::TextDisabled("Unsupported type");
            return false;
        }

        size_t const size = schema.operations->arrayGetSize(object);

        ImGui::Text("%d items :: %s", static_cast<int>(size), schema.name.c_str());

        size_t eraseIndex = size;
        size_t swapFirst = size;
        size_t swapSecond = size;

        bool edit = false;

        for (size_t index = 0; index != size; ++index) {
            void* element = schema.operations->arrayMutableElementAt(object, index);

            ImGui::PushID(static_cast<int>(index));
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::BeginGroup();

            float const rowY = ImGui::GetCursorPosY();

            // Row for handling drag-n-drop reordering of items
            if (schema.operations->arraySwapIndices != nullptr) {
                bool selected = false;
                ImGui::Selectable("##row", &selected, ImGuiSelectableFlags_AllowItemOverlap);

                if (ImGui::BeginDragDropTarget()) {
                    if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("reorder"); payload != nullptr) {
                        swapFirst = *static_cast<size_t const*>(payload->Data);
                        swapSecond = index;
                    }
                    ImGui::EndDragDropTarget();
                }

                if (ImGui::BeginDragDropSource(
                        ImGuiDragDropFlags_SourceAutoExpirePayload | ImGuiDragDropFlags_SourceNoPreviewTooltip |
                        ImGuiDragDropFlags_SourceNoDisableHover)) {
                    ImGui::SetDragDropPayload("reorder", &index, sizeof(index));
                    ImGui::EndDragDropSource();
                }
            }

            // Icon for deleting a row
            if (schema.operations->arrayEraseAt != nullptr) {
                float const availWidth = ImGui::GetContentRegionAvail().x;
                float const buttonWidth = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.x * 2.f;
                float const adjustWidth = availWidth - buttonWidth;
                ImGui::SetCursorPos({ImGui::GetCursorPosX() + adjustWidth, rowY});
                if (ImGui::IconButton("##remove", ICON_FA_TRASH)) {
                    eraseIndex = index;
                }
            }

            ImGui::EndGroup();

            ImGui::TableSetColumnIndex(1);
            ImGui::AlignTextToFramePadding();
            edit |= _editField(field, *schema.elementType, element);
            ImGui::PopID();
        }

        if (schema.operations->arrayInsertAt != nullptr) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            float const availWidth = ImGui::GetContentRegionAvail().x;
            float const buttonWidth = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.x * 2.f;
            float const adjustWidth = availWidth - buttonWidth;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + adjustWidth);
            if (ImGui::IconButton("##add", ICON_FA_PLUS)) {
                schema.operations->arrayInsertAt(object, size);
                edit = true;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::AlignTextToFramePadding();
            ImGui::TextDisabled("Add new row");
        }

        if (eraseIndex < size) {
            schema.operations->arrayEraseAt(object, eraseIndex);
            edit = true;
        }
        else if (swapFirst < size) {
            schema.operations->arraySwapIndices(object, swapFirst, swapSecond);
            edit = true;
        }

        return edit;
    }

    bool PropertyGrid::_editProperties(reflex::Schema const& schema, void* object) {
        UP_GUARD(schema.primitive == reflex::SchemaPrimitive::Object, false);

        bool edits = false;

        for (reflex::SchemaField const& field : schema.fields) {
            if (field.schema->primitive == reflex::SchemaPrimitive::Object &&
                queryAnnotation<schema::Flatten>(field) != nullptr) {
                void* const member = static_cast<char*>(object) + field.offset;
                edits |= _editProperties(*field.schema, member);
                continue;
            }

            if (queryAnnotation<schema::Hidden>(field) != nullptr) {
                continue;
            }

            ImGui::PushID(field.name.c_str());

            auto const* const displayNameAnnotation = queryAnnotation<schema::DisplayName>(field);
            zstring_view const displayName = displayNameAnnotation != nullptr && !displayNameAnnotation->name.empty()
                ? displayNameAnnotation->name
                : field.name;

            bool const expandable = field.schema->primitive == reflex::SchemaPrimitive::Object;

            bool open = true; // non-expandable fields are always "open"

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();

            PropertyInfo const info{
                .field = field,
                .schema = *field.schema,
                .object = static_cast<char*>(object) + field.offset};
            PropertyEditor* const propertyEditor = _selectEditor(info);

            if (expandable) {
                ImGuiTreeNodeFlags const flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                    ImGuiTreeNodeFlags_SpanAvailWidth;
                ImGui::PushID(field.name.c_str());
                open = ImGui::TreeNodeEx(displayName.c_str(), flags);
                ImGui::PopID();
            }
            else if (propertyEditor != nullptr) {
                propertyEditor->label(info);
            }
            else {
                ImGui::Text("%s", field.name.c_str());
            }

            if (open) {
                ImGui::TableSetColumnIndex(1);
                ImGui::AlignTextToFramePadding();

                if (propertyEditor != nullptr) {
                    edits |= propertyEditor->edit(info);
                }
                else {
                    edits |= _editField(field, *field.schema, info.object);
                }
            }

            if (open && expandable) {
                ImGui::TreePop();
            }

            ImGui::PopID();
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
