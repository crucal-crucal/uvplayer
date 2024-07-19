# Qt
uv_setup_Qt()

# ffmpeg
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg-5.0.1)
include_directories(${FFMPEG_DIR}/include)
if (WIN32)
    link_directories(${FFMPEG_DIR}/lib)
endif ()

option(FFMPEG_CHECK_VERSION "Check ffmpeg version" ON)

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
    set(FFMPEG_LIBS avcodec avdevice avfilter avformat avutil postproc swresample swscale)
endif ()

# sdl
set(SDL_DIR ${CMAKE_SOURCE_DIR}/3rdparty/sdl)
include_directories(${SDL_DIR}/include)
if (WIN32)
    link_directories(${SDL_DIR}/lib)
    set(SDL_LIBS SDL2)
endif ()

# logger
include_directories(logger)
# framelessMessageBox
include_directories(framelessMessageBox)
# app
include_directories(.)
include_directories(def)
include_directories(codec)
include_directories(frame)
include_directories(gl)

# src
set(CONF_SRC
        conf/uvconf.hpp
        conf/uvfile.hpp
        conf/uviniparser.cpp
        conf/uviniparser.hpp
)

set(DEF_SRC
        def/appdef.hpp
        def/avdef.hpp
        def/uvdef.hpp
        def/uverr.hpp
        def/auvexport.hpp
)

set(GL_SRC
        gl/uvglwidget.cpp
        gl/uvglwidget.hpp
        gl/uvglwnd.cpp
        gl/uvglwnd.hpp
)

set(GLOBAL_SRC
        global/uvmedia.hpp
        global/uvscope.hpp
        global/uvbase.hpp
        global/uvdevice.hpp
        global/uvfunctions.hpp
)

set(INTERFACE_SRC
        interface/uvcenterwidget.cpp
        interface/uvcenterwidget.hpp
        interface/uvmultiview.cpp
        interface/uvmultiview.hpp
        interface/uvmultiview_p.hpp
        interface/uvmainwindow.cpp
        interface/uvmainwindow.hpp
        interface/uvmainwindow_p.hpp
        interface/uvtable.cpp
        interface/uvtable.hpp
        interface/uvcustomeventtype.hpp
        interface/uvopenmediadlg.cpp
        interface/uvopenmediadlg.hpp
        interface/uvvideoplayer.hpp
        interface/uvvideotitlebar.cpp
        interface/uvvideotitlebar.hpp
        interface/uvvideotoolbar.cpp
        interface/uvvideotoolbar.hpp
        interface/uvvideowidget.cpp
        interface/uvvideowidget.hpp
        interface/uvvideownd.cpp
        interface/uvvideownd.hpp
        interface/uvvideowndfactory.hpp
)

set(SDL2_SRC
        sdl/uvsdl2Wnd.cpp
        sdl/uvsdl2Wnd.hpp
)

set(UTIL_SRC
        util/uvsdl_util.hpp
        util/uvbuf.hpp
        util/uvframe.hpp
        util/uvframe.cpp
        util/uvgl.hpp
        util/uvgui.hpp
        util/uvffmpeg_util.hpp
)

set(VIDEO_SRC
        video/uvthread.hpp
        video/uvffplayer.cpp
        video/uvffplayer.hpp
        #        video/uvcodec.cpp
        #        video/uvcodec.hpp
)

add_executable(${PROJECT_NAME}
        main.cpp ${CONF_SRC} ${DEF_SRC} ${GL_SRC} ${GLOBAL_SRC} ${INTERFACE_SRC} ${SDL2_SRC} ${UTIL_SRC} ${VIDEO_SRC}
)
# Qt
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Multimedia)
# 3rdparty, OpenGL & SDL & FFmpeg & GLEW
target_compile_definitions(${PROJECT_NAME} PRIVATE -DGLEW_STATIC)
target_link_libraries(${PROJECT_NAME}
        ${FFMPEG_LIBS}
        ${SDL_LIBS}
        glew32s
        opengl32
)
# app
target_link_libraries(${PROJECT_NAME}
        uvmaterialslider
        uvslider
        logger
        framelessMessageBox
        uvstring
)

# logger => Qt Release版本中如果需要定位到日志文件，需要添加此宏
add_definitions(-DQT_MESSAGELOGCONTEXT)

# subdirectories
add_subdirectory(logger)
add_subdirectory(framelessMessageBox)
add_subdirectory(uvstring)
add_subdirectory(uvslider)
add_subdirectory(uvshared)
add_subdirectory(uvmaterialslider)

# dependencies required for installation and runtime
file(GLOB FFMPEG_DLLS ${FFMPEG_DIR}/bin/*.dll)
file(GLOB GLEW_DLLS ${GLEW_DIR}/bin/*.dll)
file(GLOB SDL_DLLS ${SDL_DIR}/bin/*.dll)
set(RCCFILE ${CMAKE_SOURCE_DIR}/resource/mediaplayer.rcc)
set(CONF_FILE_NAME mediaplayer.conf.default)
set(CONF_FILE_PATH ${CMAKE_SOURCE_DIR}/bin/conf/${CONF_FILE_NAME})

# install application and dependencies, except for Qt
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${RCCFILE} DESTINATION bin)
install(FILES ${CONF_FILE_PATH} DESTINATION bin/conf)
install(FILES ${FFMPEG_DLLS} DESTINATION bin)
install(FILES ${GLEW_DLLS} DESTINATION bin)
install(FILES ${SDL_DLLS} DESTINATION bin)

# copy ffmpeg dlls, glew dlls, sdl2 dlls to output directory
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DLLS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${GLEW_DLLS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SDL_DLLS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying ffmpeg dlls, glew dlls, sdl2 dlls to output directory"
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
