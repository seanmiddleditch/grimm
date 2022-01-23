// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/imgui_fonts.h"

#include "potato/data/fontawesome_font.h"
#include "potato/data/roboto_font.h"
#include "potato/spud/utility.h"

#include <imgui.h>

void ImGui::Potato::PushFont(UpFont font) {
    auto& io = ImGui::GetIO();

    auto const index = up::to_underlying(font);
    UP_GUARD_VOID(index < io.Fonts->Fonts.Size);

    ImFont** const fonts = io.Fonts->Fonts.Data;
    ImGui::PushFont(fonts[index]);
}

void ImGui::Potato::LoadFonts() {
    auto& io = ImGui::GetIO();

    ImFontConfig config;
    config.MergeMode = false;
    config.PixelSnapH = false;
    config.FontDataOwnedByAtlas = false;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(roboto_font_data), roboto_font_size, 16.f, &config);

    config.MergeMode = true;
    config.GlyphMinAdvanceX = 14.f;
    config.PixelSnapH = true;
    config.FontDataOwnedByAtlas = false;

    static constexpr auto fontawesomeMinGlyph = 0xf000;
    static constexpr auto fontawesomeMaxGlyph = 0xf897;
    static constexpr ImWchar s_ranges[] = {fontawesomeMinGlyph, fontawesomeMaxGlyph, 0};

    io.Fonts->AddFontFromMemoryTTF(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        const_cast<unsigned char*>(fontawesome_font_data),
        fontawesome_font_size,
        12.f,
        &config,
        s_ranges);

    config.MergeMode = false;
    config.GlyphMinAdvanceX = 72.f;

    io.Fonts->AddFontFromMemoryTTF(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        const_cast<unsigned char*>(fontawesome_font_data),
        fontawesome_font_size,
        72.f,
        &config,
        s_ranges);
}
