// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/editor/editor.h"
#include "potato/game/space.h"
#include "potato/render/gpu_resource.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/spud/hash.h"

#include <glm/vec2.hpp>

namespace up {
    class AudioEngine;
    class Space;
    class GpuDevice;
} // namespace up

namespace up::shell {
    class GameEditor : public Editor<GameEditor> {
    public:
        static constexpr EditorTypeId editorTypeId{"potato.editor.game"};

        explicit GameEditor(EditorParams const& params, AudioEngine& audio, box<Space> space);

        static void addFactory(Workspace& workspace, AudioEngine& audio);
        static void addCommands(CommandManager& commands);

        zstring_view displayName() const override { return "Game"_zsv; }

        void togglePause() noexcept { _paused = !_paused; }

    protected:
        void content(CommandManager&) override;
        void tick(float deltaTime) override;
        void render(Renderer& renderer, float deltaTime) override;
        bool hasMenu() override { return true; }

    private:
        struct PlayPauseHandler;

        void _resize(GpuDevice& device, glm::ivec2 size);

        box<Space> _space;
        rc<GpuResource> _buffer;
        box<GpuResourceView> _bufferView;
        EntityId _cameraId = EntityId::None;
        glm::ivec2 _viewDimensions = {0, 0};
        bool _isInputBound = false;
        bool _paused = false;
    };
} // namespace up::shell
