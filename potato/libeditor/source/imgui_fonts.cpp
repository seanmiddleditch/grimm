// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "imgui_backend.h"
#include "imgui_fonts.h"

#include <imgui.h>

void ImGui::Potato::PushFont(UpFont font) {
    auto const& backend = *static_cast<up::ImguiBackend const*>(GetIO().UserData);

    ImGui::PushFont(backend.getFont(static_cast<int>(font)));
}
