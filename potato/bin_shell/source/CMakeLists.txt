target_sources(potato_shell PRIVATE
    "camera.cpp"
    "camera_controller.cpp"
    "edit_components.h"
    "edit_components.cpp"
    "editor.cpp"
    "log_history.cpp"
    "main.cpp"
    "scene_doc.cpp"
    "settings.cpp"
    "settings.h"
    "shell_app.cpp"
 )

 add_subdirectory(editors)
 add_subdirectory(ui)
