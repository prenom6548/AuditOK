pkg_check_modules(QGS REQUIRED gsettings-qt)
pkg_check_modules(GIO2.0 REQUIRED gio-2.0)
pkg_check_modules(KYSDKQTWIDGETS REQUIRED kysdk-qtwidgets)
pkg_check_modules(KYSDKDIAGNOSTICS REQUIRED kysdk-diagnostics)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/Common)

include_directories(
	${QGS_INCLUDE_DIRS}
        ${GIO2.0_INCLUDE_DIRS}
        ${KYSDKQTWIDGETS_INCLUDE_DIRS}
        ${KYSDKDIAGNOSTICS_INCLUDE_DIRS}
        /usr/include/ukcc
	)

set(EXTRA_LIBS
        ${EXTRA_LIBS}
        ${QGS_LIBRARIES}
        ${GIO2.0_LIBRARIES}
        ${KYSDKQTWIDGETS_LIBRARIES}
        ${KYSDKDIAGNOSTICS_LIBRARIES}
        -lukcc
	)

qt5_add_resources(lockscreen_Plugin_SRC
        )

qt5_wrap_cpp(lockscreen_Plugin_SRC
    pictureunit.h
    screenlock.h
    screenlockui.h
    ../../screensaver/masklable.h
    ../screensaver/photocorver.h
    ../../common/commonfunc.h
    )

set(lockscreen_Plugin_SRC
    ${lockscreen_Plugin_SRC}
    pictureunit.cpp
    screenlock.cpp
    screenlockui.cpp
    ../../screensaver/masklable.cpp
    ../screensaver/photocorver.cpp
    ../../common/commonfunc.cpp
    )

add_library(screenlock SHARED ${lockscreen_Plugin_SRC})
target_link_libraries(screenlock Qt5::Widgets Qt5::DBus Qt5::Xml Qt5::Svg ${EXTRA_LIBS})

install(TARGETS
        screenlock
        DESTINATION lib/${CMAKE_LIBRARY_ARCHITECTURE}/ukui-control-center)
