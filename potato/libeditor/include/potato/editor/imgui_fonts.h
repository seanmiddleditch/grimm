// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

namespace ImGui::inline Potato {
    enum class UpFont { Default, Roboto_16 = Default, FontAwesome_72, Count_ };

    UP_EDITOR_API void PushFont(UpFont font);
} // namespace ImGui::inline Potato
