// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "icons.h"

#include <glm/fwd.hpp>
#include <imgui.h>

inline namespace Potato {
    enum ImGuiInteractiveFlags_ { ImGuiInteractiveFlags_None = 0, ImGuiInteractiveFlags_AllowItemOverlap };
}

namespace ImGui::inline Potato {

    UP_EDITOR_API bool BeginContextPopup();

    UP_EDITOR_API bool ToggleHeader(char const* label, char const* icon = nullptr) noexcept;

    UP_EDITOR_API void Interactive(
        char const* label,
        ImGuiInteractiveFlags_ flags = ImGuiInteractiveFlags_None) noexcept;

    UP_EDITOR_API bool BeginToolbar(char const* id);
    UP_EDITOR_API void EndToolbar();

    UP_EDITOR_API bool IconButton(
        char const* label,
        char const* icon,
        ImVec2 size = {},
        ImGuiButtonFlags flags = ImGuiButtonFlags_None);
    UP_EDITOR_API bool BeginIconButtonDropdown(
        char const* label,
        char const* icon,
        ImVec2 size = {},
        ImGuiButtonFlags flags = ImGuiButtonFlags_None);
    UP_EDITOR_API void EndIconButtonDropdown();

    UP_EDITOR_API bool IsModifierDown(ImGuiKeyModFlags modifiers) noexcept;

    UP_EDITOR_API ImVec2 GetItemSpacing();
    UP_EDITOR_API ImVec2 GetItemInnerSpacing();

    UP_EDITOR_API bool BeginIconGrid(char const* label, float iconWidth = 96.f);
    UP_EDITOR_API bool IconGridItem(
        ImGuiID id,
        char const* label,
        char const* icon,
        bool selected = false,
        float width = 96.f,
        float rounding = 8.f);
    UP_EDITOR_API void EndIconGrid();

    UP_EDITOR_API bool InlineSliderScalar(
        const char* label,
        ImGuiDataType dataType,
        void* pData,
        const void* pMin = nullptr,
        const void* pMax = nullptr,
        const char* format = nullptr,
        ImGuiSliderFlags flags = ImGuiSliderFlags_None);

    UP_EDITOR_API bool InlineInputScalar(
        const char* label,
        ImGuiDataType dataType,
        void* pData,
        const void* pStep = nullptr,
        const void* pStepFast = nullptr,
        const char* format = nullptr,
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);

    UP_EDITOR_API void ApplyStyle();
} // namespace ImGui::inline Potato
