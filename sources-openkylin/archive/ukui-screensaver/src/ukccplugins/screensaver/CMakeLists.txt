pkg_check_modules(QGS REQUIRED gsettings-qt)
pkg_check_modules(GIO2.0 REQUIRED gio-2.0)
pkg_check_modules(GIOUNIX2.0 REQUIRED gio-unix-2.0)
pkg_check_modules(KYSDKQTWIDGETS REQUIRED kysdk-qtwidgets)
pkg_check_modules(KYSDKDIAGNOSTICS REQUIRED kysdk-diagnostics)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/Common)

include_directories(
        ${QGS_INCLUDE_DIRS}
        ${GIO2.0_INCLUDE_DIRS}
        ${GIOUNIX2.0_INCLUDE_DIRS}
        ${KYSDKQTWIDGETS_INCLUDE_DIRS}
        ${KYSDKDIAGNOSTICS_INCLUDE_DIRS}
        ${KYSDKSYSTIME_INCLUDE_DIRS}
        /usr/include/ukcc
        )

set(EXTRA_LIBS
        ${EXTRA_LIBS}
        ${QGS_LIBRARIES}
        ${GIO2.0_LIBRARIES}
        ${GIOUNIX2.0_LIBRARIES}
        ${KYSDKQTWIDGETS_LIBRARIES}
        ${KYSDKDIAGNOSTICS_LIBRARIES}
        ${KYSDKSYSTIME_LIBRARIES}
        -lukcc
        )

qt5_add_resources(screensaver_Plugin_SRC
        )

qt5_wrap_cpp(screensaver_Plugin_SRC
    previewwidget.h
    screensaver.h
    screensaverpluginso.h
    screensaverui.h
    photocorver.h
    ../../screensaver/albumthumbnailwidget.h
    ../../screensaver/saverthumbnailwidget.h
    ../../screensaver/weatherthumbnailwidget.h
    ../../screensaver/musicthumbnailwidget.h
    ../../screensaver/customalbumviewer.h
    ../../screensaver/masklable.h
    ../../screensaver/timethumbnailwidget.h
    ../../screensaver/weatherwidget.h
    ../../screensaver/adaptivelabel.h
    ../../common/commonfunc.h
    ../../widgets/customlabel.h
    )

set(screensaver_Plugin_SRC
    ${screensaver_Plugin_SRC}
    previewwidget.cpp
    screensaver.cpp
    screensaverui.cpp
    photocorver.cpp
    ../../screensaver/albumthumbnailwidget.cpp
    ../../screensaver/saverthumbnailwidget.cpp
    ../../screensaver/weatherthumbnailwidget.cpp
    ../../screensaver/musicthumbnailwidget.cpp
    ../../screensaver/customalbumviewer.cpp
    ../../screensaver/masklable.cpp
    ../../screensaver/timethumbnailwidget.cpp
    ../../screensaver/weatherwidget.cpp
    ../../screensaver/adaptivelabel.cpp
    ../../common/commonfunc.cpp
    ../../widgets/customlabel.cpp
    )

add_library(screensaver SHARED ${screensaver_Plugin_SRC})
target_link_libraries(screensaver Qt5::Widgets Qt5::DBus Qt5::Svg ${EXTRA_LIBS})

install(TARGETS
        screensaver
        DESTINATION lib/${CMAKE_LIBRARY_ARCHITECTURE}/ukui-control-center)
