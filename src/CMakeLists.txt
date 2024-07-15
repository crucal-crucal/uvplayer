# Qt
uv_setup_Qt()

# ffmpeg
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg)
include_directories(${FFMPEG_DIR}/include)
if (WIN32)
    link_directories(${FFMPEG_DIR}/lib)
endif ()

option(FFMPEG_CHECK_VERSION "Check ffmpeg version" OFF)

if (FFMPEG_CHECK_VERSION)
    # Function to extract FFmpeg version
    function(get_ffmpeg_version)
        file(READ "${FFMPEG_DIR}/include/libavformat/version.h" VERSION_HEADER)
        string(REGEX MATCH "#define[ \t]+LIBAVFORMAT_VERSION_MAJOR[ \t]+([0-9]+)" _ ${VERSION_HEADER})
        set(FFMPEG_VERSION_MAJOR ${CMAKE_MATCH_1})

        string(REGEX MATCH "#define[ \t]+LIBAVFORMAT_VERSION_MINOR[ \t]+([0-9]+)" _ ${VERSION_HEADER})
        set(FFMPEG_VERSION_MINOR ${CMAKE_MATCH_1})

        string(REGEX MATCH "#define[ \t]+LIBAVFORMAT_VERSION_MICRO[ \t]+([0-9]+)" _ ${VERSION_HEADER})
        set(FFMPEG_VERSION_MICRO ${CMAKE_MATCH_1})

        set(FFMPEG_VERSION "${FFMPEG_VERSION_MAJOR}.${FFMPEG_VERSION_MINOR}.${FFMPEG_VERSION_MICRO}")
        message(STATUS "FFmpeg version: ${FFMPEG_VERSION}")

        set(FFMPEG_VERSION_MAJOR_INT ${FFMPEG_VERSION_MAJOR} PARENT_SCOPE)
    endfunction()

    # Get FFmpeg version
    get_ffmpeg_version()

    # 根据版本号设置预处理宏
    if (FFMPEG_VERSION_MAJOR_INT GREATER_EQUAL 59)
        add_definitions(-DFFMPEG_VERSION_GTE_5_0_0)
        message(STATUS "FFmpeg version >= 5.0.0")
    else ()
        message(STATUS "FFmpeg version < 5.0.0")
    endif ()
endif (FFMPEG_CHECK_VERSION)

# glew
set(GLEW_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glew)
include_directories(${GLEW_DIR}/include)
if (WIN32)
    link_directories(${GLEW_DIR}/lib)
endif ()

if (WIN32)
    set(FFMPEG_LIBS avcodec avdevice avfilter avformat avutil postproc swresample swscale)
endif ()

# logger
include_directories(logger)
# framelessMessageBox
include_directories(framelessMessageBox)
# app
include_directories(.)

add_executable(${PROJECT_NAME} main.cpp
        appdef.hpp
        uvmainwindow.cpp
        uvmainwindow.hpp
        uvmainwindow_p.hpp
        uvcenterwidget.cpp
        uvcenterwidget.hpp
        uvmultiview.cpp
        uvmultiview.hpp
        uvmultiview_p.hpp
        uvtable.cpp
        uvtable.hpp
        uvthread.hpp
        uvffplayer.cpp
        uvffplayer.hpp
        uvvideoplayer.hpp
        avdef.hpp
        uvmedia.hpp
        uvbuf.hpp
        uvframe.hpp
        uvframe.cpp
        uvscope.hpp
        uvvideowidget.cpp
        uvvideowidget.hpp
        uvvideotitlebar.cpp
        uvvideotitlebar.hpp
        uvvideotoolbar.cpp
        uvvideotoolbar.hpp
        uvvideownd.cpp
        uvvideownd.hpp
        uvopenmediadlg.cpp
        uvopenmediadlg.hpp
        uvcustomeventtype.hpp
        uvgl.hpp
        uvglwidget.cpp
        uvglwidget.hpp
        uvgui.hpp
        uvglwnd.cpp
        uvglwnd.hpp
        uvdef.hpp
        uviniparser.cpp
        uviniparser.hpp
        uvfile.hpp
        uverr.hpp
        uvstring/uvstring.cpp
        uvstring/uvstring.hpp
        auvexport.hpp
        uvconf.hpp
        uvbase.hpp
        uvdevice.hpp
        uvfunctions.hpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src/GL)
# Qt
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Multimedia)
# OpenGL & GLEW
target_compile_definitions(${PROJECT_NAME} PRIVATE -DGLEW_STATIC)
target_link_libraries(${PROJECT_NAME}
        glew32s
        opengl32
)
# 3rdparty
target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBS})
# app
target_link_libraries(${PROJECT_NAME}
        logger
        framelessMessageBox
        uvstring
)

# subdirectories
add_subdirectory(logger)
add_subdirectory(framelessMessageBox)
add_subdirectory(uvstring)

# dependencies required for installation and runtime
file(GLOB FFMPEG_DLLS ${FFMPEG_DIR}/bin/*.dll)
file(GLOB GLEW_DLLS ${GLEW_DIR}/bin/*.dll)
set(RCCFILE ${CMAKE_SOURCE_DIR}/resource/uvplayer.rcc)
set(CONF_FILE_NAME uvplayer.conf.default)
set(CONF_FILE_PATH ${CMAKE_SOURCE_DIR}/bin/conf/${CONF_FILE_NAME})

# install application and dependencies, except for Qt
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
)
install(FILES ${RCCFILE} DESTINATION bin)
install(FILES ${CONF_FILE_PATH} DESTINATION bin/conf)
install(FILES ${FFMPEG_DLLS} DESTINATION bin)
install(FILES ${GLEW_DLLS} DESTINATION bin)

# copy ffmpeg dlls to output directory
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DLLS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${GLEW_DLLS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying ffmpeg dlls, glew dlls to output directory"
    )
endif ()

# copy rcc, config files to output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${RCCFILE}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CONF_FILE_PATH}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/conf/${CONF_FILE_NAME}
        COMMENT "Copying rcc, config file to output directory"
)
