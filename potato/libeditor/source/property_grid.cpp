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

bool up::editor::PropertyGrid::beginItem(char const* label) {
    ImGuiID const openId = ImGui::GetID("open");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);

    ImGuiStorage* const storage = ImGui::GetStateStorage();
    bool open = storage->GetBool(openId, true);

    ImGuiSelectableFlags const flags = ImGuiSelectableFlags_SpanAllColumns;

    if (ImGui::Selectable(label, open, flags)) {
        open = !open;
        storage->SetBool(openId, open);
    }

    return open;
}

void up::editor::PropertyGrid::endItem() { }

bool up::editor::PropertyGrid::_editField(
    reflex::SchemaField const& field,
    reflex::Schema const& schema,
    void* object) {
    ImGui::PushID(object);

    bool edit = false;

    switch (schema.primitive) {
        case reflex::SchemaPrimitive::Int16:
            edit = _editIntegerField(field, *static_cast<int16*>(object));
            break;
        case reflex::SchemaPrimitive::Int32:
            edit = _editIntegerField(field, *static_cast<int32*>(object));
            break;
        case reflex::SchemaPrimitive::Int64:
            edit = _editIntegerField(field, *static_cast<int64*>(object));
            break;
        case reflex::SchemaPrimitive::Float:
            edit = _editFloatField(field, *static_cast<float*>(object));
            break;
        case reflex::SchemaPrimitive::Double:
            edit = _editFloatField(field, *static_cast<double*>(object));
            break;
        case reflex::SchemaPrimitive::Vec3:
            edit = _editVec3Field(field, *static_cast<glm::vec3*>(object));
            break;
        case reflex::SchemaPrimitive::Mat4x4:
            edit = _editMat4x4Field(field, *static_cast<glm::mat4x4*>(object));
            break;
        case reflex::SchemaPrimitive::Quat:
            edit = _editQuatField(field, *static_cast<glm::quat*>(object));
            break;
        case reflex::SchemaPrimitive::String:
            edit = _editStringField(field, *static_cast<string*>(object));
            break;
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
        case reflex::SchemaPrimitive::AssetRef:
            edit = _editAssetField(field, schema, object);
            break;
        case reflex::SchemaPrimitive::Uuid:
            edit = _editUuidField(field, *static_cast<UUID*>(object));
            break;
        default:
            ImGui::Text("Unsupported primitive type for schema `%s`", schema.name.c_str());
            break;
    }

    ImGui::PopID();

    return edit;
}

bool up::editor::PropertyGrid::_drawObjectEditor(reflex::Schema const& schema, void* object) {
    ImGui::TextDisabled("%s", schema.name.c_str());

    return _editProperties(schema, object);
}

bool up::editor::PropertyGrid::_editArrayField(
    reflex::SchemaField const& field,
    reflex::Schema const& schema,
    void* object) {
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

bool up::editor::PropertyGrid::_beginProperty(reflex::SchemaField const& field, void* object) {
    if (queryAnnotation<schema::Hidden>(field) != nullptr) {
        return false;
    }

    auto const* const displayNameAnnotation = queryAnnotation<schema::DisplayName>(field);
    zstring_view const displayName = displayNameAnnotation != nullptr && !displayNameAnnotation->name.empty()
        ? displayNameAnnotation->name
        : field.name;

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
    bool const expandable =
        (field.schema->primitive == reflex::SchemaPrimitive::Object ||
         field.schema->primitive == reflex::SchemaPrimitive::Mat4x4);
    if (!expandable) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();

    void* const member = static_cast<char*>(object) + field.offset;

    ImGui::PushID(member);
    bool const open = ImGui::TreeNodeEx(displayName.c_str(), flags);
    ImGui::PopID();

    auto const* const tooltipAnnotation = queryAnnotation<schema::Tooltip>(field);
    if (tooltipAnnotation != nullptr && ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", tooltipAnnotation->text.c_str());
        ImGui::EndTooltip();
    }

    if (open) {
        ImGui::TreePush(member);
    }
    return open;
}

void up::editor::PropertyGrid::_endProperty() {
    ImGui::TreePop();
}

bool up::editor::PropertyGrid::_editProperties(reflex::Schema const& schema, void* object) {
    UP_GUARD(schema.primitive == reflex::SchemaPrimitive::Object, false);

    bool edits = false;

    for (reflex::SchemaField const& field : schema.fields) {
        if (field.schema->primitive == reflex::SchemaPrimitive::Object &&
            queryAnnotation<schema::Flatten>(field) != nullptr) {
            void* const member = static_cast<char*>(object) + field.offset;
            edits |= _editProperties(*field.schema, member);
            continue;
        }

        if (_beginProperty(field, object)) {
            edits |= _editProperty(field, object);
            _endProperty();
        }
    }

    return edits;
}

bool up::editor::PropertyGrid::_editProperty(reflex::SchemaField const& field, void* object) {
    void* const member = static_cast<char*>(object) + field.offset;

    ImGui::TableSetColumnIndex(1);
    ImGui::AlignTextToFramePadding();

    return _editField(field, *field.schema, member);
}

bool up::editor::PropertyGrid::_editIntegerField(reflex::SchemaField const& field, int& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);

    auto const* const rangeAnnotation = queryAnnotation<up::schema::IntRange>(field);
    if (rangeAnnotation != nullptr) {
        return ImGui::SliderInt("##int", &value, rangeAnnotation->min, rangeAnnotation->max);
    }

    return ImGui::InputInt("##int", &value);
}

bool up::editor::PropertyGrid::_editFloatField(reflex::SchemaField const& field, float& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);

    auto const* const rangeAnnotation = queryAnnotation<up::schema::FloatRange>(field);
    if (rangeAnnotation != nullptr) {
        return ImGui::SliderFloat("##float", &value, rangeAnnotation->min, rangeAnnotation->max);
    }

    return ImGui::InputFloat("##float", &value);
}

bool up::editor::PropertyGrid::_editFloatField(
    [[maybe_unused]] reflex::SchemaField const& field,
    double& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);

    return ImGui::InputDouble("##double", &value);
}

bool up::editor::PropertyGrid::_editVec3Field(
    [[maybe_unused]] reflex::SchemaField const& field,
    glm::vec3& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    return ImGui::InputVec3("##vec3", value);
}

bool up::editor::PropertyGrid::_editMat4x4Field(
    [[maybe_unused]] reflex::SchemaField const& field,
    glm::mat4x4& value) noexcept {
    bool edit = false;
    ImGui::SetNextItemWidth(-1.f);
    edit |= ImGui::InputFloat4("##a", &value[0].x);
    ImGui::SetNextItemWidth(-1.f);
    edit |= ImGui::InputFloat4("##b", &value[1].x);
    ImGui::SetNextItemWidth(-1.f);
    edit |= ImGui::InputFloat4("##c", &value[2].x);
    ImGui::SetNextItemWidth(-1.f);
    edit |= ImGui::InputFloat4("##d", &value[3].x);
    return edit;
}

bool up::editor::PropertyGrid::_editQuatField(
    [[maybe_unused]] reflex::SchemaField const& field,
    glm::quat& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    return ImGui::InputQuat("##quat", value);
}

bool up::editor::PropertyGrid::_editStringField(
    [[maybe_unused]] reflex::SchemaField const& field,
    string& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);

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

bool up::editor::PropertyGrid::_editAssetField(
    reflex::SchemaField const& field,
    reflex::Schema const& schema,
    void* object) {
    ImGui::BeginGroup();
    UP_GUARD(schema.primitive == reflex::SchemaPrimitive::AssetRef, false);

    zstring_view assetType{};
    if (auto const* const assetTypeAnno = queryAnnotation<schema::AssetType>(schema); assetTypeAnno != nullptr) {
        assetType = assetTypeAnno->assetType;
    }

    auto* const handle = static_cast<UntypedAssetHandle*>(object);
    AssetId const assetId = handle->assetId();
    zstring_view displayName = "<empty>"_zsv;

    if (handle->isSet()) {
        displayName = _assetLoader->debugName(assetId);
    }

    bool edit = false;

    ImGui::Text("%s", displayName.c_str());
    ImGui::SameLine();
    if (ImGui::IconButton("##clear", ICON_FA_TRASH) && schema.operations->pointerAssign != nullptr) {
        schema.operations->pointerAssign(object, nullptr);
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

    if (_assetLoader != nullptr && schema.operations != nullptr && schema.operations->pointerAssign != nullptr) {
        AssetId targetAssetId = assetId;
        if (up::assetBrowserPopup(browserId, targetAssetId, assetType, *_assetLoader) && targetAssetId != assetId) {
            *handle = _assetLoader->loadAssetSync(targetAssetId, assetType);
            edit = true;
        }
    }

    ImGui::EndGroup();

    return edit;
}

bool up::editor::PropertyGrid::_editUuidField([[maybe_unused]] reflex::SchemaField const& field, UUID& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);

    // FIXME:
    // up::string is an immutable string type, which isn't easy to make editable
    // in a well-performing way. we ideally want to know when a string is being
    // edited, make a temporary copy into a cheaply-resizable buffer, then post-
    // edit copy that back into a new up::string. For now... just this.
    char buffer[UUID::strLength];
    nanofmt::format_to(buffer, "{}", value);

    if (ImGui::InputText(
            "##uuid",
            buffer,
            sizeof(buffer),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal |
                ImGuiInputTextFlags_CharsDecimal /* for dash */)) {
        value = UUID::fromString(buffer);
        return true;
    }

    return false;
}
