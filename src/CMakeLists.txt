# Qt
uv_setup_Qt()

# ffmpeg
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg)
include_directories(${FFMPEG_DIR}/include)
link_directories(${FFMPEG_DIR}/lib)

# glew
set(GLEW_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glew)
include_directories(${GLEW_DIR}/include)
link_directories(${GLEW_DIR}/lib)

if (WIN32)
    set(FFMPEG_LIBS avcodec avdevice avfilter avformat avutil postproc swresample swscale)
endif ()

# logger
include_directories(logger)
# framelessMessageBox
include_directories(framelessMessageBox)

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
# custom
target_link_libraries(${PROJECT_NAME}
        logger
        framelessMessageBox
        uvstring
)


add_subdirectory(logger)
add_subdirectory(framelessMessageBox)
add_subdirectory(uvstring)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
)

# copy ffmpeg dlls to output directory
if (WIN32)
    file(GLOB FFMPEG_DLLS ${FFMPEG_DIR}/bin/*.dll)
    file(GLOB GLEW_DLLS ${GLEW_DIR}/bin/*.dll)

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
        ${CMAKE_SOURCE_DIR}/resource/uvplayer.rcc
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/bin/conf/uvplayer.conf.default
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/conf/uvplayer.conf.default
        COMMENT "Copying rcc file to output directory"
)
