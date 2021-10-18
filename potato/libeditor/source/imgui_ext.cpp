// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.
//
// This file includes code from dear imgui at https://github.com/ocornut/imgui, which is
// covered by the following license.
//
// The MIT License (MIT)
//
// Copyright(c) 2014 - 2020 Omar Cornut
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "potato/editor/imgui_ext.h"

#include "potato/editor/imgui_backend.h"
#include "potato/spud/numeric_util.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>
#include <imgui_internal.h>

static void DrawIcon(const char8_t* icon, ImVec2 minPos, ImVec2 maxPos) {
    ImGui::RenderTextClipped(
        minPos,
        maxPos,
        reinterpret_cast<char const*>(icon),
        nullptr,
        nullptr,
        {0.5f, 0.5f},
        nullptr);
}

bool ImGui::Potato::IconButton(char const* label, char8_t const* icon, ImVec2 size, ImGuiButtonFlags flags) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext& g = *GImGui;
    ImGuiStyle const& style = g.Style;
    ImFont const* font = ImGui::GetFont();

    float const iconWidth = font->FontSize;
    ImVec2 const iconSize{iconWidth, iconWidth};

    bool const hasLabel = FindRenderedTextEnd(label) != label;
    ImVec2 const labelSize = hasLabel ? CalcTextSize(label, nullptr, true) : ImVec2{};

    ImVec2 pos = window->DC.CursorPos;

    if (window != nullptr && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) {
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    }
    ImVec2 buttonSize = CalcItemSize(
        size,
        labelSize.x + iconWidth + style.FramePadding.x * 2.0f + (hasLabel ? style.ItemInnerSpacing.x : 0.f),
        std::max(labelSize.y, iconSize.y) + style.FramePadding.y * 2.0f);

    ImRect const bb(pos, pos + buttonSize);
    ItemSize(buttonSize, style.FramePadding.y);

    ImGuiID const id = window->GetID(label);
    if (!ItemAdd(bb, id)) {
        return false;
    }

    if ((window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) != 0) {
        flags |= ImGuiButtonFlags_Repeat;
    }

    bool hovered = false;
    bool held = false;
    bool const pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    ImU32 const col = GetColorU32(
        (held && hovered) ? ImGuiCol_ButtonActive
            : hovered     ? ImGuiCol_ButtonHovered
                          : ImGuiCol_Button);
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
    if (icon != nullptr && *icon != u8'\0') {
        DrawIcon(icon, bb.Min + style.FramePadding, bb.Min + style.FramePadding + iconSize);
    }
    RenderTextClipped(
        bb.Min + ImVec2{iconWidth + style.ItemInnerSpacing.x, 0} + style.FramePadding,
        bb.Max - style.FramePadding,
        label,
        nullptr,
        &labelSize,
        style.ButtonTextAlign,
        &bb);

    return pressed;
}

void ImGui::Potato::SetCaptureRelativeMouseMode(bool captured) {
    auto& io = ImGui::GetIO();
    auto* const state = static_cast<up::ImguiBackend*>(io.UserData);
    if (state != nullptr) {
        state->setCaptureRelativeMouseMode(captured);
    }
}

auto ImGui::Potato::IsCaptureRelativeMouseMode() -> bool {
    auto& io = ImGui::GetIO();
    auto* const state = static_cast<up::ImguiBackend*>(io.UserData);
    return state != nullptr ? state->isCaptureRelativeMouseMode() : false;
}

bool ImGui::Potato::IsModifierDown(ImGuiKeyModFlags modifiers) noexcept {
    auto& io = ImGui::GetIO();
    return (io.KeyMods & modifiers) == modifiers;
}

bool ImGui::Potato::InputVec3(char const* label, glm::vec3& value, char const* format, ImGuiInputTextFlags flags) {
    return ImGui::InputFloat3(label, &value.x, format, flags);
}

bool ImGui::Potato::InputQuat(char const* label, glm::quat& value, char const* format, ImGuiSliderFlags flags) {
    auto euler = glm::eulerAngles(value);
    auto eulerDegrees = glm::vec3(glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z));

    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::SliderFloat3(label, &eulerDegrees.x, 0.f, +359.f, format, flags)) {
        value = glm::vec3(glm::radians(eulerDegrees.x), glm::radians(eulerDegrees.y), glm::radians(eulerDegrees.z));
        return true;
    }

    return false;
}

bool ImGui::BeginIconMenuContextPopup() {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiID const id = window->DC.LastItemId;

    if (IsMouseReleased(ImGuiMouseButton_Right) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
        OpenPopupEx(id, 0);
    }
    return BeginPopupEx(
        id,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

bool ImGui::BeginIconMenu(const char* label, bool enabled) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal) {
        return BeginMenu(label, enabled);
    }

    char const* labelEnd = FindRenderedTextEnd(label);

    ImVec2 const pos = window->DC.CursorPos;
    ImVec2 const iconSize = ImGui::CalcTextSize(reinterpret_cast<char const*>(ICON_FA_INFO));
    ImVec2 const labelSize = ImGui::CalcTextSize(label, labelEnd);
    ImVec2 const spacing = GetStyle().ItemInnerSpacing;

    float const labelOffsetX = iconSize.x + spacing.x * 2.f;

    ImColor const color = GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    window->DC.MenuColumns.DeclColumns(labelSize.x + labelOffsetX, 0, 0);

    char menuId[32] = {
        0,
    };
    nanofmt::format_to(menuId, "##menu_{}", GetID(label));

    bool const open = BeginMenu(menuId, enabled);

    // we need to draw into our original window, so temporarily close the new sub-menu,
    // draw, and then we'll reopen it if appropriate.
    //
    ImGuiWindow* menuWindow = nullptr;
    if (open) {
        menuWindow = GetCurrentWindow();
        EndPopup();
        UP_ASSERT(window == GetCurrentWindow());
    }

    window->DrawList->AddText(ImVec2(pos.x + labelOffsetX, pos.y), color, label, labelEnd);

    if (open) {
        BeginPopup(menuId, menuWindow->Flags);
    }

    return open;
}

void ImGui::Potato::IconMenuSeparator() {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    if (window->DC.LayoutType == ImGuiLayoutType_Vertical) {
        ImVec2 const iconSize = ImGui::CalcTextSize(reinterpret_cast<char const*>(ICON_FA_INFO));
        ImVec2 const spacing = GetStyle().ItemInnerSpacing;

        window->DC.CursorPos.x += window->DC.Indent.x + iconSize.x + spacing.x * 2.f;

        ImRect const bounds(window->DC.CursorPos, window->DC.CursorPos + ImVec2(GetContentRegionAvailWidth(), 1.f));

        ItemSize(ImVec2(0.0f, 1.f));
        if (!ItemAdd(bounds, 0)) {
            return;
        }

        window->DrawList->AddLine(bounds.Min, ImVec2(bounds.Max.x, bounds.Min.y), GetColorU32(ImGuiCol_Separator));
    }
    else {
        Separator();
    }
}

bool ImGui::Potato::IconMenuItem(
    const char* label,
    const char8_t* icon,
    const char* shortcut,
    bool selected,
    bool enabled) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems && TableGetColumnCount() == 0) {
        return false;
    }

    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal) {
        return MenuItem(label, shortcut, selected, enabled);
    }

    char const* labelEnd = FindRenderedTextEnd(label);

    ImVec2 const pos = window->DC.CursorPos;
    ImVec2 const iconSize = ImGui::CalcTextSize(reinterpret_cast<char const*>(ICON_FA_INFO));
    ImVec2 const labelSize = ImGui::CalcTextSize(label, labelEnd);
    ImVec2 const spacing = GetStyle().ItemInnerSpacing;

    float const labelOffsetX = iconSize.x + spacing.x * 2.f;

    ImColor const color = GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    window->DC.MenuColumns.DeclColumns(labelSize.x + labelOffsetX, 0, 0);

    PushID(label);
    bool const clicked = MenuItem("##item", shortcut, &selected, enabled);
    PopID();

    if (icon != nullptr) {
        window->DrawList->AddText(ImVec2(pos.x + spacing.x, pos.y), color, reinterpret_cast<char const*>(icon));
    }

    window->DrawList->AddText(ImVec2(pos.x + labelOffsetX, pos.y), color, label, labelEnd);

    return clicked;
}

ImVec2 ImGui::Potato::GetItemSpacing() {
    return GetStyle().ItemSpacing;
}

ImVec2 ImGui::Potato::GetItemInnerSpacing() {
    return GetStyle().ItemInnerSpacing;
}

bool ImGui::Potato::BeginIconGrid(char const* label, float iconWidth) {
    float const availWidth = ImGui::GetContentRegionAvailWidth();
    int const columns = up::clamp(static_cast<int>(availWidth / iconWidth), 1, 64);

    return ImGui::BeginTable(label, columns);
}

void ImGui::Potato::EndIconGrid() {
    ImGui::EndTable();
}

bool ImGui::Potato::IconGridItem(
    ImGuiID id,
    char const* label,
    char8_t const* icon,
    bool selected,
    float width,
    float rounding) {
    UP_ASSERT(label != nullptr);
    UP_ASSERT(icon != nullptr);
    UP_ASSERT(width > 0);
    UP_ASSERT(rounding >= 0);

    ImGui::TableNextColumn();

    ImGuiWindow* const window = ImGui::GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImVec2 const size = ImGui::CalcItemSize({width, width}, 0.0f, 0.0f);
    ImRect const bounds{window->DC.CursorPos, window->DC.CursorPos + size};
    ImRect const innerBounds{bounds.Min + ImGui::GetItemSpacing(), bounds.Max - ImGui::GetItemSpacing()};

    bool clicked = false;

    ImGui::ItemSize(size);
    if (ImGui::ItemAdd(bounds, id)) {
        bool hovered = false;
        bool held = false;
        clicked = ButtonBehavior(
            bounds,
            id,
            &hovered,
            &held,
            ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_PressedOnDoubleClick | ImGuiButtonFlags_NoKeyModifiers);

        ImU32 const textColor = ImGui::GetColorU32(ImGuiCol_Text);
        ImU32 const bgColor = ImGui::GetColorU32(
            held           ? ImGuiCol_ButtonActive
                : hovered  ? ImGuiCol_ButtonHovered
                : selected ? ImGuiCol_Header
                           : ImGuiCol_Button);

        bool const showBg = hovered || held || selected;

        if (showBg) {
            window->DrawList->AddRectFilled(bounds.Min, bounds.Max, bgColor, rounding);
        }

        // must calculate this _before_ pushing the icon font, since we want to calcualte the size of the
        // label's height
        float const iconMaxHeight =
            innerBounds.GetHeight() - ImGui::GetTextLineHeightWithSpacing() - ImGui::GetItemInnerSpacing().y;

        ImGui::PushFont(ImGui::UpFont::FontAwesome_72);
        ImVec2 iconSize = ImGui::CalcTextSize(reinterpret_cast<char const*>(icon));
        float iconScale = 1.f;
        if (iconSize.y > iconMaxHeight) {
            iconScale = iconMaxHeight / iconSize.y;
        }
        ImGui::SetWindowFontScale(iconScale);
        ImVec2 const iconPos{
            innerBounds.Min.x + (innerBounds.GetWidth() - iconSize.x * iconScale) * 0.5f,
            innerBounds.Min.y};
        window->DrawList->AddText(iconPos, textColor, reinterpret_cast<char const*>(icon));
        ImGui::SetWindowFontScale(1.f);
        ImGui::PopFont();

        char const* const labelEnd = ImGui::FindRenderedTextEnd(label);
        float const textHeight = ImGui::GetTextLineHeight();
        ImRect const textBounds{
            ImVec2{bounds.Min.x, innerBounds.Max.y - ImGui::GetItemSpacing().y - textHeight},
            ImVec2{bounds.Max.x, innerBounds.Max.y - ImGui::GetItemSpacing().y}};

        if (showBg) {
            window->DrawList->AddRectFilled(textBounds.Min, textBounds.Max, ImGui::GetColorU32(ImGuiCol_Header));
        }

        ImFont const* const font = GetFont();
        ImVec2 const textSize = CalcTextSize(label, nullptr, true, 0.f);
        ImVec4 const textFineBounds{textBounds.Min.x, textBounds.Min.y, textBounds.Max.x, textBounds.Max.y};
        if (size.x > textBounds.GetWidth()) {
            window->DrawList
                ->AddText(font, font->FontSize, textBounds.Min, textColor, label, labelEnd, 0.f, &textFineBounds);
        }
        else {
            float const offsetX = (textBounds.GetWidth() - textSize.x) * 0.5f;
            ImVec2 const centerPos{textBounds.Min.x + offsetX, textBounds.Min.y};
            window->DrawList
                ->AddText(font, font->FontSize, centerPos, textColor, label, labelEnd, 0.f, &textFineBounds);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", label);
            ImGui::EndTooltip();
        }
    }

    return clicked;
}
