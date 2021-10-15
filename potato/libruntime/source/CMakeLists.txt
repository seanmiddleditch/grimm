target_sources(potato_libruntime PRIVATE
    # Callstack implementations
    #
    $<$<NOT:$<PLATFORM_ID:Windows>>:source/callstack.posix.cpp>
    $<$<PLATFORM_ID:Windows>:source/callstack.windows.cpp>

    # Debug handling implementations
    #
    $<$<NOT:$<PLATFORM_ID:Windows>>:source/debug.posix.cpp>
    $<$<PLATFORM_ID:Windows>:source/debug.windows.cpp>
    $<$<PLATFORM_ID:Windows>:source/debug.windows.h>
    $<$<PLATFORM_ID:Windows>:source/debug.windows.rc>

    # Threading backends
    #
    $<$<PLATFORM_ID:Darwin>:source/thread_util.darwin.cpp>
    $<$<PLATFORM_ID:Linux>:source/thread_util.linux.cpp>
    $<$<PLATFORM_ID:Windows>:source/thread_util.windows.cpp>

    # UUID generation
    #
    $<$<PLATFORM_ID:Darwin>:source/uuid.darwin.cpp>
    $<$<PLATFORM_ID:Linux>:source/uuid.linux.cpp>
    $<$<PLATFORM_ID:Windows>:source/uuid.windows.cpp>

    # General runtime code
    #
    "asset_loader.cpp"
    "filesystem.cpp"
    "io_loop.cpp"
    "resource_manifest.cpp"
    "debug.cpp"
    "json.cpp"
    "logger.cpp"
    "path.cpp"
    "stream.cpp"
    "task_worker.cpp"
    "thread_util.cpp"
    "uuid.cpp"
)
