// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/shell/ui/action.h"
#include "potato/spud/box.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <imgui.h>

namespace up {
    class Renderer;
}

namespace up::shell {
    class Editor;

    class Menu;

    class EditorGroup {
    public:
        explicit EditorGroup(Actions& actions);
        ~EditorGroup();

        void update(Renderer& renderer, float deltaTime);

        void closeAll() noexcept;
        void closeActive() noexcept;

        // shut it all down.  this needs to be called before renderer shutdown to ensure that
        // any graphics resources are freed up before the device is killed.
        void shutdown() noexcept; 

        bool canCloseActive() const noexcept;

        void open(box<Editor> editor);

    private:
        void _setActive(Editor* editor);

        vector<box<Editor>> _editors;
        Actions& _actions;
        ImGuiWindowClass _documentWindowClass;
        Editor* _active = nullptr;
    };
} // namespace up::shell
