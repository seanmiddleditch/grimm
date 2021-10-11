// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "editor.h"

#include "potato/runtime/logger.h"
#include "potato/spud/hash.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up::shell {
    class LogHistory;

    class LogWindow : public Editor {
    public:
        static constexpr zstring_view editorName = "potato.editor.logs"_zsv;

        explicit LogWindow(LogHistory& history) : Editor("LogWindow"_zsv), _history(history) {}

        LogWindow(LogWindow const&) = delete;
        LogWindow& operator=(LogWindow const&) = delete;

        zstring_view displayName() const override { return "Logs"; }
        zstring_view editorClass() const override { return editorName; }
        EditorId uniqueId() const override { return hash_value(this); }

        static box<EditorFactory> createFactory(LogHistory& history);

        void configure() override {}
        void content() override;

    private:
        LogHistory& _history;
        LogSeverityMask _mask = LogSeverityMask::Everything;
        bool _stickyBottom = true;
        char _filter[128] = {
            0,
        };
    };
} // namespace up::shell
