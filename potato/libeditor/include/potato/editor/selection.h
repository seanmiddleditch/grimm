// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/find.h"
#include "potato/spud/int_types.h"
#include "potato/spud/vector.h"

namespace up {
    using SelectionId = uint64;
    constexpr SelectionId SelectionIdNone = ~uint64{0};

    class SelectionState {
    public:
        bool select(SelectionId id, bool state = true) {
            auto it = find(_selected, id);
            bool const selected = it != _selected.end();
            if (state && !selected) {
                _selected.push_back(id);
                return true;
            }
            if (!state && selected) {
                _selected.erase(it);
                return true;
            }
            return false;
        }

        bool click(SelectionId id, bool multiselect = false, bool context = false) {
            auto it = find(_selected, id);
            bool const wasSelected = it != _selected.end();

            // context behavior: right-click a selected item never deselects other items
            if (context && wasSelected) {
                return true;
            }

            // multi-select behavior: add or remove on ctrl-click
            if (multiselect) {
                if (wasSelected) {
                    _selected.erase(it);
                    return false;
                }

                _selected.push_back(id);
                return true;
            }

            // single-select behavior: select on click, clear others
            _selected.clear();
            _selected.push_back(id);
            return true;
        }

        [[nodiscard]] bool empty() const noexcept { return _selected.empty(); }
        [[nodiscard]] auto size() const noexcept { return _selected.size(); }

        [[nodiscard]] bool selected(SelectionId id) const noexcept { return contains(_selected, id); }
        [[nodiscard]] view<SelectionId> selected() const noexcept { return _selected; }

        void clear() { _selected.clear(); }

    private:
        vector<SelectionId> _selected;
    };
} // namespace up
