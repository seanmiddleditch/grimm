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

#include "potato/editor/imgui_fonts.h"
#include "potato/spud/numeric_util.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>
#include <imgui_internal.h>

static void DrawIcon(const char* icon, ImVec2 minPos, ImVec2 maxPos) {
    ImGui::RenderTextClipped(
        minPos,
        maxPos,
        reinterpret_cast<char const*>(icon),
        nullptr,
        nullptr,
        {0.5f, 0.5f},
        nullptr);
}

bool ImGui::Potato::IconButton(char const* label, char const* icon, ImVec2 size, ImGuiButtonFlags flags) {
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
        up::max(labelSize.y, iconSize.y) + style.FramePadding.y * 2.0f);

    ImRect const bb(pos, pos + buttonSize);
    ItemSize(buttonSize, style.FramePadding.y);

    ImGuiID const id = window->GetID(label);
    if (!ItemAdd(bb, id)) {
        return false;
    }

    if ((g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat) != 0) {
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

bool ImGui::Potato::BeginIconButtonDropdown(char const* label, char const* icon, ImVec2 size, ImGuiButtonFlags flags) {
    if (IconButton(label, icon, size, flags)) {
        ImGui::OpenPopup(label);
    }

    return ImGui::BeginPopup(label);
}

void ImGui::Potato::EndIconButtonDropdown() {
    ImGui::EndPopup();
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

bool ImGui::BeginContextPopup() {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiID const id = GetItemID();

    if (IsMouseReleased(ImGuiMouseButton_Right) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
        OpenPopupEx(id, 0);
    }
    return BeginPopupEx(
        id,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

ImVec2 ImGui::Potato::GetItemSpacing() {
    return GetStyle().ItemSpacing;
}

ImVec2 ImGui::Potato::GetItemInnerSpacing() {
    return GetStyle().ItemInnerSpacing;
}

bool ImGui::Potato::BeginIconGrid(char const* label, float iconWidth) {
    ImGuiStyle const& style = GetStyle();
    float const availWidth =
        up::max(ImGui::GetContentRegionAvailWidth() - style.WindowPadding.x - style.FramePadding.x * 2.f, 0.f);
    float const paddedIconWidth = iconWidth + style.ColumnsMinSpacing;
    int const columns = up::clamp(static_cast<int>(availWidth / paddedIconWidth), 1, 64);

    return ImGui::BeginTable(label, columns);
}

void ImGui::Potato::EndIconGrid() {
    ImGui::EndTable();
}

bool ImGui::Potato::IconGridItem(
    ImGuiID id,
    char const* label,
    char const* icon,
    bool selected,
    float width,
    float rounding) {
    UP_GUARD(label != nullptr, false);
    UP_GUARD(icon != nullptr, false);
    UP_GUARD(width > 0, false);
    UP_GUARD(rounding >= 0, false);

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

void ImGui::Potato::ApplyStyle() {
    auto& style = ImGui::GetStyle();

    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowRounding = 6.0f;
    style.PopupRounding = 2.0f;
    style.ChildRounding = 2.0f;

    style.WindowPadding = ImVec2(4.0f, 4.0f);
    style.FramePadding = ImVec2(4.0f, 4.0f);

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.GrabMinSize = 18.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f); // ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}
