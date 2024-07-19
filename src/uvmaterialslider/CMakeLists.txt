set(TARGET_NAME uvmaterialslider)

if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

include_directories(..)
aux_source_directory(. SRC_LIST)

add_library(${TARGET_NAME} SHARED ${SRC_LIST})
target_link_libraries(${TARGET_NAME} Qt5::Widgets)
target_link_libraries(${TARGET_NAME} uvmaterialshared)

target_compile_definitions(${TARGET_NAME} PRIVATE CUVMATERIALSLIDER_LIB)

uv_install_dll()