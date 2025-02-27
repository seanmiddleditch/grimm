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

namespace ImGui::inline Potato {
    static void DrawIcon(char const* icon, ImVec2 minPos, ImVec2 maxPos) {
        ImGui::RenderTextClipped(
            minPos,
            maxPos,
            reinterpret_cast<char const*>(icon),
            nullptr,
            nullptr,
            {0.5f, 0.5f},
            nullptr);
    }

    bool ToggleHeader(char const* label, char const* icon) noexcept {
        ImGuiID const openId = ImGui::GetID("open");

        ImGuiStorage* const storage = ImGui::GetStateStorage();
        bool open = storage->GetBool(openId, true);

        char buf[256];
        char const* selectLabel = label;
        if (icon != nullptr) {
            nanofmt::format_to(buf, "{} {}", icon, label);
            selectLabel = buf;
        }

        if (ImGui::Selectable(selectLabel, open, ImGuiSelectableFlags_None)) {
            open = !open;
            storage->SetBool(openId, open);
        }

        return open;
    }

    bool ClickableText(char const* text, char const* textEnd) noexcept {
        char const* idStr = text;
        char const* const idStrEnd = textEnd;

        if (textEnd == nullptr) {
            textEnd = ImGui::FindRenderedTextEnd(text);
            if (*textEnd != '\0') {
                idStr = textEnd;
            }
        }

        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::TextUnformatted(text, textEnd);
        ImGui::SetCursorPos(pos);

        ImVec2 const textSize = ImGui::CalcTextSize(text, textEnd);
        ImGui::PushID(idStr, idStrEnd);
        bool const clicked = ImGui::InvisibleButton("##text", textSize);
        ImGui::PopID();
        return clicked;
    }

    void Interactive(char const* label, ImGuiInteractiveFlags_ flags) noexcept {
        ImGuiWindow* const window = ImGui::GetCurrentWindow();
        if (window->SkipItems) {
            return;
        }

        auto const& style = ImGui::GetStyle();

        auto const pos = window->DC.CursorPos;
        auto const maxPos = window->WorkRect.Max;
        float const frameHeight = ImGui::GetFontSize() + style.FramePadding.y * 2.f;

        auto size = CalcItemSize({maxPos.x - pos.x, frameHeight}, 0.f, 0.f);
        ItemSize(size);
        auto const id = ImGui::GetID(label);
        auto const bounds = ImRect(pos, {pos.x + size.x, pos.y + size.y});
        if (!ItemAdd(bounds, id)) {
            return;
        }

        int buttonFlags = ImGuiButtonFlags_MouseButtonLeft;
        if ((flags & ImGuiInteractiveFlags_AllowItemOverlap) != 0) {
            buttonFlags |= ImGuiButtonFlags_AllowItemOverlap;
        }

        bool hovered = false;
        bool held = false;
        ButtonBehavior(bounds, id, &hovered, &held, buttonFlags);

        if ((flags & ImGuiInteractiveFlags_AllowItemOverlap) != 0) {
            SetItemAllowOverlap();
        }

        if (hovered || held) {
            const ImU32 col = GetColorU32(held ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered);
            RenderFrame(bounds.Min, bounds.Max, col, false, 0.0f);
        }
        RenderNavHighlight(bounds, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

        ImGui::RenderText(pos, label, nullptr, true);

        window->DC.CursorPosPrevLine = pos;
        window->DC.CursorPos = {pos.x, pos.y + frameHeight};
    }

    bool BeginToolbar(char const* id) {
        auto const& style = ImGui::GetStyle();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.FramePadding.x);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_Header));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, style.FrameBorderSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);

        ImVec2 toolbarSize{
            ImGui::GetContentRegionAvail().x - style.FramePadding.x,
            ImGui::GetTextLineHeight() + style.FramePadding.y * 4.f};
        bool const open = ImGui::BeginChild(
            id,
            toolbarSize,
            false,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NavFlattened);

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(1);

        return open;
    }

    void EndToolbar() {
        // auto const& style = ImGui::GetStyle();

        ImGui::EndChild();

        // ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
    }

    bool IconButton(char const* label, char const* icon, ImVec2 size, ImGuiButtonFlags flags) {
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

    bool BeginIconButtonDropdown(char const* label, char const* icon, ImVec2 size, ImGuiButtonFlags flags) {
        if (IconButton(label, icon, size, flags)) {
            ImGui::OpenPopup(label);
        }

        return ImGui::BeginPopup(label);
    }

    void EndIconButtonDropdown() { ImGui::EndPopup(); }

    bool IsModifierDown(ImGuiKeyModFlags modifiers) noexcept {
        auto& io = ImGui::GetIO();
        return (io.KeyMods & modifiers) == modifiers;
    }

    bool BeginContextPopup() {
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

    ImVec2 GetItemSpacing() { return GetStyle().ItemSpacing; }

    ImVec2 GetItemInnerSpacing() { return GetStyle().ItemInnerSpacing; }

    bool BeginIconGrid(char const* label, float iconWidth) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.f);
        bool const open = ImGui::BeginChild(
            label,
            ImVec2{},
            false,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NavFlattened | ImGuiWindowFlags_AlwaysUseWindowPadding);
        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(1);

        ImGui::BeginGroup();

        return open;
    }

    void EndIconGrid() {
        ImGui::EndGroup();
        ImGui::EndChild();
    }

    bool IconGridItem(ImGuiID id, char const* label, char const* icon, bool selected, float width, float rounding) {
        UP_GUARD(label != nullptr, false);
        UP_GUARD(icon != nullptr, false);
        UP_GUARD(width > 0, false);
        UP_GUARD(rounding >= 0, false);

        auto const& style = ImGui::GetStyle();

        ImGuiWindow* const window = ImGui::GetCurrentWindow();
        if (window->SkipItems) {
            return false;
        }

        ImVec2 const size = ImGui::CalcItemSize({width, width}, 0.0f, 0.0f);

        {
            if (window->DC.CursorPosPrevLine.x > window->Pos.x + style.FramePadding.x) {
                ImGui::SameLine(0.f, style.ItemSpacing.x);
            }

            ImVec2 const availSize = ImGui::GetContentRegionAvail();
            if (availSize.x < width) {
                ImGui::Spacing();
            }
        }

        ImRect const bounds{window->DC.CursorPos, window->DC.CursorPos + size};
        ImRect const innerBounds{bounds.Min + ImGui::GetItemSpacing(), bounds.Max - ImGui::GetItemSpacing()};

        ImGui::ItemSize(size);
        if (!ImGui::ItemAdd(bounds, id)) {
            return false;
        }

        bool hovered = false;
        bool held = false;
        bool const clicked = ButtonBehavior(
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

        return clicked;
    }

    static float CalcItemWidth(float width) {
        if (width < 0.f) {
            ImGuiWindow const* const window = ImGui::GetCurrentWindowRead();
            float region_max_x = GetContentRegionMaxAbs().x;
            width = ImMax(1.0f, region_max_x - window->DC.CursorPos.x + width);
        }
        return IM_FLOOR(width);
    }

    bool BeginInlineFrame(char const* label, float width) {
        ImGuiStyle const& style = ImGui::GetStyle();

        ImVec2 const framePadding = style.FramePadding;
        ImVec2 const innerItemSpacing = style.ItemInnerSpacing;
        ImVec2 const size{CalcItemWidth(width), ImGui::GetTextLineHeight() + framePadding.y * 2};

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, style.FrameRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, style.FrameBorderSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.f, 0.f});
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_Header));

        bool const visible = ImGui::BeginChild(
            ImGui::GetID(label),
            size,
            true,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NavFlattened);

        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar(3);

        if (visible) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);

            char const* const labelEnd = ImGui::FindRenderedTextEnd(label);
            if (label != labelEnd) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + framePadding.x);

                ImGui::AlignTextToFramePadding();
                if (ClickableText(label, labelEnd)) {
                    ImGui::SetKeyboardFocusHere();
                }
                ImGui::SameLine(0.f, innerItemSpacing.x);
            }
            ImGui::SetNextItemWidth(-1.f);
        }

        return visible;
    }

    void EndInlineFrame() {
        ImGui::PopStyleVar(1);
        ImGui::EndChild();
    }

    bool BeginTitlebarPopup(char const* title, ImGuiWindowFlags flags) {
        if (!ImGui::IsPopupOpen(title, ImGuiPopupFlags_None)) {
            ImGui::GetCurrentContext()->NextWindowData.ClearFlags();
            return false;
        }

        flags |= ImGuiWindowFlags_Popup | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
        bool const open = ImGui::Begin(title, nullptr, flags);
        if (!open) {
            EndPopup();
        }
        return open;
    }

    void ApplyStyle() {
        auto& style = ImGui::GetStyle();

        style.FrameRounding = 0.0f;
        style.GrabRounding = 4.0f;
        style.WindowRounding = 6.0f;
        style.PopupRounding = 0.0f;
        style.ChildRounding = 0.0f;

        style.WindowPadding = ImVec2(4.0f, 4.0f);
        style.FramePadding = ImVec2(4.0f, 4.0f);

        style.WindowBorderSize = 2.0f;
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
        colors[ImGuiCol_Border] = ImVec4(0.10f, 0.14f, 0.16f, 1.00f); // ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
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
} // namespace ImGui::inline Potato
