// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/editor/editor.h"
#include "potato/runtime/logger.h"
#include "potato/spud/hash.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up::shell {
    class LogHistory;

    class LogEditor : public Editor<LogEditor> {
    public:
        static constexpr EditorTypeId editorTypeId{"potato.editor.logs"};

        explicit LogEditor(EditorParams const& params, LogHistory& history) : Editor(params), _history(history) { }

        LogEditor(LogEditor const&) = delete;
        LogEditor& operator=(LogEditor const&) = delete;

        zstring_view displayName() const override { return "Logs"_zsv; }

        static void addFactory(Workspace& workspace, LogHistory& history);

        void content(CommandManager&) override;

    private:
        LogHistory& _history;
        LogSeverityMask _mask = LogSeverityMask::Everything;
        bool _stickyBottom = true;
        char _filter[128] = {
            0,
        };
    };
} // namespace up::shell
