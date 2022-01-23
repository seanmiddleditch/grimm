target_sources(potato_shell PRIVATE
    "_export.h"
    "editor.h"
    "log_history.h"
    "scene_doc.h"
    "selection.h"
)

add_subdirectory(scene)
add_subdirectory(ui)
