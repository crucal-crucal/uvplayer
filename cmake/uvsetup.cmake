# uvsetup.cmake

# 从环境变量中获取 Qt 路径，查找 qmake
macro(uv_setup_Qt)
    if (WIN32 OR UNIX)
        set(QT_DIR $ENV{QTDIR})
        message(STATUS "QT_DIR=${QT_DIR}")
        find_program(QMAKE_EXECUTABLENAMES qmake HINTS $ENV{QT_DIR} PATH_SUFFIXES bin)
        set(CMAKE_PREFIX_PATH ${QT_DIR})
    endif ()

    # 查询 Qt 版本 并存储再 QT_VERSION 中
    execute_process(COMMAND ${QMAKE_EXECUTABLENAMES} -query QT_VERSION OUTPUT_VARIABLE QT_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (QT_VERSION)
        message(STATUS "Qt version: " ${QT_VERSION})
        string(REPLACE ".." ";" QT_VERSION_LIST ${QT_VERSION})
        list(GET QT_VERSION_LIST 0 QT_VERSION_MAJOR)
    else ()
        message(FATAL_ERROR "Qt version not found. Please check if the QTDIR environment variable is set correctly.")
    endif ()

    include_directories(${QT_DIR}/include)

    if (QT_VERSION_MAJOR VERSION_GREATER_EQUAL 5)
        message(STATUS "Qt5 found.")
        find_package(Qt5
                COMPONENTS
                Core
                Gui
                Widgets
                OpenGL
                REQUIRED
        )
    else ()
        message(FATAL_ERROR "this project requires Qt5. Please install Qt5 and try again.")
    endif ()

endmacro()

# 安装 DLL
macro(uv_install_dll)
    install(TARGETS ${TARGET_NAME}
            RUNTIME DESTINATION bin
    )

    install(TARGETS ${TARGET_NAME}
            ARCHIVE DESTINATION lib OPTIONAL
    )

    if (WIN32)
        install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}> DESTINATION pdb OPTIONAL)
    endif ()
endmacro()

# 复制 dll 到可执行文件目录
macro(uv_copy_dll)
endmacro()