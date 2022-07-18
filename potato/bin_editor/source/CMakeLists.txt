target_sources(potato_editor PRIVATE
    "commands.h"
    "edit_components.h"
    "edit_components.cpp"
    "log_history.cpp"
    "log_history.h"
    "main.cpp"
    "scene_doc.h"
    "scene_doc.cpp"
    "settings.cpp"
    "settings.h"
    "shell_app.cpp"
 )

 add_subdirectory(editors)
