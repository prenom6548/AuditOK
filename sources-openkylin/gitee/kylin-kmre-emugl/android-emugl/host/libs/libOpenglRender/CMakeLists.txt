set(SOURCES
    ../../opengles/opengles.cpp
    NativeSubWindow_x11.cpp
    ChannelStream.cpp
    UnixStream.cpp
    ColorBuffer.cpp
    FbConfig.cpp
    FenceSync.cpp
    FrameBuffer.cpp
    GLESVersionDetector.cpp
    GrabberHelper.cpp
    PostWorker.cpp
    ReadbackWorker.cpp
    ReadBuffer.cpp
    RenderChannelImpl.cpp
    RenderContext.cpp
    RenderControl.cpp
    RendererImpl.cpp
    RenderLibImpl.cpp
    RenderServer.cpp
    RenderThread.cpp
    RingStream.cpp
    RenderThreadInfo.cpp
    render_api.cpp
    RenderWindow.cpp
    SyncThread.cpp
    TextureDraw.cpp
    TextureResize.cpp
    WindowSurface.cpp
    YUVConverter.cpp)
    
    
add_library(OpenglRender SHARED ${SOURCES})

if (USE_EMUGEN)
add_dependencies(OpenglRender GLHeaders)
endif (USE_EMUGEN)

target_link_libraries(
    OpenglRender
    GLESv1_dec
    GLESv2_dec
    emugl_common
    android-emu-base
    GLSnapshot
    renderControl_dec
    OpenglCodecCommon    
    OpenGLESDispatch
    pciaccess
    xcb
    xcb-dri3
    drm
    ${X11_X11_LIB}    
    rt)

if (NEED_INSTALL)
install(
  TARGETS OpenglRender
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif (NEED_INSTALL)


