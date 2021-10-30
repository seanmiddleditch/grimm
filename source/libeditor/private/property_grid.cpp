// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "property_grid.h"
#include "asset_browser_popup.h"
#include "common_schema.h"
#include "constraint_schema.h"
#include "tools_schema.h"

#include "potato/editor/icons.h"
#include "potato/editor/imgui_ext.h"
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

void up::editor::PropertyGrid::endItem() {}

void up::editor::PropertyGrid::_editField(
    reflex::SchemaField const& field,
    reflex::Schema const& schema,
    void* object) {
    ImGui::PushID(object);

    switch (schema.primitive) {
        case reflex::SchemaPrimitive::Int16:
            _editIntegerField(field, *static_cast<int16*>(object));
            break;
        case reflex::SchemaPrimitive::Int32:
            _editIntegerField(field, *static_cast<int32*>(object));
            break;
        case reflex::SchemaPrimitive::Int64:
            _editIntegerField(field, *static_cast<int64*>(object));
            break;
        case reflex::SchemaPrimitive::Float:
            _editFloatField(field, *static_cast<float*>(object));
            break;
        case reflex::SchemaPrimitive::Double:
            _editFloatField(field, *static_cast<double*>(object));
            break;
        case reflex::SchemaPrimitive::Vec3:
            _editVec3Field(field, *static_cast<glm::vec3*>(object));
            break;
        case reflex::SchemaPrimitive::Mat4x4:
            _editMat4x4Field(field, *static_cast<glm::mat4x4*>(object));
            break;
        case reflex::SchemaPrimitive::Quat:
            _editQuatField(field, *static_cast<glm::quat*>(object));
            break;
        case reflex::SchemaPrimitive::String:
            _editStringField(field, *static_cast<string*>(object));
            break;
        case reflex::SchemaPrimitive::Pointer:
            if (schema.operations->pointerMutableDeref != nullptr) {
                if (void* pointee = schema.operations->pointerMutableDeref(object)) {
                    _editField(field, *schema.elementType, pointee);
                }
            }
            break;
        case reflex::SchemaPrimitive::Array:
            _editArrayField(field, schema, object);
            break;
        case reflex::SchemaPrimitive::Object:
            _drawObjectEditor(schema, object);
            break;
        case reflex::SchemaPrimitive::AssetRef:
            _editAssetField(field, schema, object);
            break;
        case reflex::SchemaPrimitive::Uuid:
            _editUuidField(field, *static_cast<UUID*>(object));
            break;
        default:
            ImGui::Text("Unsupported primitive type for schema `%s`", schema.name.c_str());
            break;
    }

    ImGui::PopID();
}

void up::editor::PropertyGrid::_drawObjectEditor(reflex::Schema const& schema, void* object) {
    ImGui::TextDisabled("%s", schema.name.c_str());

    _editProperties(schema, object);
}

void up::editor::PropertyGrid::_editArrayField(
    reflex::SchemaField const& field,
    reflex::Schema const& schema,
    void* object) {
    if (schema.operations == nullptr) {
        ImGui::TextDisabled("Unsupported type");
        return;
    }

    if (schema.operations->arrayGetSize == nullptr || schema.operations->arrayElementAt == nullptr) {
        ImGui::TextDisabled("Unsupported type");
        return;
    }

    size_t const size = schema.operations->arrayGetSize(object);

    ImGui::Text("%d items :: %s", static_cast<int>(size), schema.name.c_str());

    size_t eraseIndex = size;
    size_t swapFirst = size;
    size_t swapSecond = size;

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
            float const availWidth = ImGui::GetContentRegionAvailWidth();
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
        _editField(field, *schema.elementType, element);
        ImGui::PopID();
    }

    if (schema.operations->arrayInsertAt != nullptr) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        float const availWidth = ImGui::GetContentRegionAvailWidth();
        float const buttonWidth = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.x * 2.f;
        float const adjustWidth = availWidth - buttonWidth;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + adjustWidth);
        if (ImGui::IconButton("##add", ICON_FA_PLUS)) {
            schema.operations->arrayInsertAt(object, size);
        }

        ImGui::TableSetColumnIndex(1);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Add new row");
    }

    if (eraseIndex < size) {
        schema.operations->arrayEraseAt(object, eraseIndex);
    }
    else if (swapFirst < size) {
        schema.operations->arraySwapIndices(object, swapFirst, swapSecond);
    }
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

void up::editor::PropertyGrid::_editProperties(reflex::Schema const& schema, void* object) {
    UP_ASSERT(schema.primitive == reflex::SchemaPrimitive::Object);

    for (reflex::SchemaField const& field : schema.fields) {
        if (field.schema->primitive == reflex::SchemaPrimitive::Object &&
            queryAnnotation<schema::Flatten>(field) != nullptr) {
            void* const member = static_cast<char*>(object) + field.offset;
            _editProperties(*field.schema, member);
            continue;
        }

        if (_beginProperty(field, object)) {
            _editProperty(field, object);
            _endProperty();
        }
    }
}

void up::editor::PropertyGrid::_editProperty(reflex::SchemaField const& field, void* object) {
    void* const member = static_cast<char*>(object) + field.offset;

    ImGui::TableSetColumnIndex(1);
    ImGui::AlignTextToFramePadding();

    _editField(field, *field.schema, member);
}

void up::editor::PropertyGrid::_editIntegerField(reflex::SchemaField const& field, int& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);

    auto const* const rangeAnnotation = queryAnnotation<up::schema::IntRange>(field);
    if (rangeAnnotation != nullptr) {
        ImGui::SliderInt("##int", &value, rangeAnnotation->min, rangeAnnotation->max);
    }
    else {
        ImGui::InputInt("##int", &value);
    }
}

void up::editor::PropertyGrid::_editFloatField(reflex::SchemaField const& field, float& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);

    auto const* const rangeAnnotation = queryAnnotation<up::schema::FloatRange>(field);
    if (rangeAnnotation != nullptr) {
        ImGui::SliderFloat("##float", &value, rangeAnnotation->min, rangeAnnotation->max);
    }
    else {
        ImGui::InputFloat("##float", &value);
    }
}

void up::editor::PropertyGrid::_editFloatField(
    [[maybe_unused]] reflex::SchemaField const& field,
    double& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);

    ImGui::InputDouble("##double", &value);
}

void up::editor::PropertyGrid::_editVec3Field(
    [[maybe_unused]] reflex::SchemaField const& field,
    glm::vec3& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputVec3("##vec3", value);
}

void up::editor::PropertyGrid::_editMat4x4Field(
    [[maybe_unused]] reflex::SchemaField const& field,
    glm::mat4x4& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##a", &value[0].x);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##b", &value[1].x);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##c", &value[2].x);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##d", &value[3].x);
}

void up::editor::PropertyGrid::_editQuatField(
    [[maybe_unused]] reflex::SchemaField const& field,
    glm::quat& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputQuat("##quat", value);
}

void up::editor::PropertyGrid::_editStringField(
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
    }
}

void up::editor::PropertyGrid::_editAssetField(
    reflex::SchemaField const& field,
    reflex::Schema const& schema,
    void* object) {
    ImGui::BeginGroup();
    UP_ASSERT(schema.primitive == reflex::SchemaPrimitive::AssetRef);

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

    ImGui::Text("%s", displayName.c_str());
    ImGui::SameLine();
    if (ImGui::IconButton("##clear", ICON_FA_TRASH) && schema.operations->pointerAssign != nullptr) {
        schema.operations->pointerAssign(object, nullptr);
    }
    ImGui::SameLine();

    char browserId[32] = {
        0,
    };
    nanofmt::format_to(browserId, "##assets{}", ImGui::GetID("popup"));

    if (ImGui::IconButton("##select", ICON_FA_FOLDER)) {
        ImGui::OpenPopup(browserId);
    }

    if (_assetLoader != nullptr && schema.operations->pointerAssign != nullptr) {
        AssetId targetAssetId = assetId;
        if (up::assetBrowserPopup(browserId, targetAssetId, assetType, *_assetLoader) && targetAssetId != assetId) {
            *handle = _assetLoader->loadAssetSync(targetAssetId, assetType);
        }
    }

    ImGui::EndGroup();
}

void up::editor::PropertyGrid::_editUuidField([[maybe_unused]] reflex::SchemaField const& field, UUID& value) noexcept {
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
    }
}
