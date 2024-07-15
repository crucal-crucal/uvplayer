set(TARGET_NAME uvstring)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

include_directories(..)
aux_source_directory(. SRC_LIST)

add_library(${TARGET_NAME} SHARED ${SRC_LIST})

target_compile_definitions(${TARGET_NAME} PRIVATE UVSTRING_EXPORTS)

uv_install_dll()