pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XTST REQUIRED xtst)
pkg_check_modules(QGS REQUIRED gsettings-qt)
pkg_check_modules(KYSDKSYSTIME REQUIRED kysdk-systime)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/Common)

include_directories(
	${X11_INCLUDE_DIRS}
        ${XTST_INCLUDE_DIRS}
	${QGS_INCLUDE_DIRS}
        ${KYSDKSYSTIME_INCLUDE_DIRS}
	)

set(EXTRA_LIBS
	${EXTRA_LIBS}
	${X11_LIBRARIES}
        ${XTST_LIBRARIES}
        ${QGS_LIBRARIES}
        ${KYSDKSYSTIME_LIBRARIES}
        -ldl
	)
qt5_add_resources(screensaver_SRC
	default.qrc
	)

qt5_wrap_cpp(screensaver_SRC
    chinesedate.h
    screensaver.h
    mbackground.h
    cyclelabel.h
    scconfiguration.h
    sleeptime.h
    checkbutton.h
    weathermanager.h
    networkwatcher.h
    videoplayer.h
    graphicsview.h
    ../common/global_utils.h
    ../common/utils.h
    ../lock-dialog/backenddbushelper.h
    ../userinfo.h
    ../agreementinfo.h
    )
set(screensaver_SRC
    ${screensaver_SRC}
    main.cpp
    chinesedate.cpp
    mbackground.cpp
    screensaver.cpp
    cyclelabel.cpp
    scconfiguration.cpp
    sleeptime.cpp
    checkbutton.cpp
    weathermanager.cpp
    networkwatcher.cpp
    videoplayer.cpp
    graphicsview.cpp
    ../common/global_utils.cpp
    ../common/utils.cpp
    ../lock-dialog/backenddbushelper.cpp
    ../userinfo.cpp
    ../agreementinfo.cpp
    )
add_executable(ukui-screensaver-default ${screensaver_SRC})
target_link_libraries(ukui-screensaver-default Qt5::Core Qt5::Widgets Qt5::DBus Qt5::X11Extras Qt5::Xml Qt5::Network ukui-log4qt Qt5::Multimedia Qt5::MultimediaWidgets  avformat avcodec ${EXTRA_LIBS})

qt5_add_resources(screensaver_Plugin_SRC
        default.qrc
        )

qt5_wrap_cpp(screensaver_Plugin_SRC
    chinesedate.h
    screensaver.h
    mbackground.h
    cyclelabel.h
    scconfiguration.h
    sleeptime.h
    checkbutton.h
    weathermanager.h
    customplugin.h
    networkwatcher.h
    videoplayer.h
    graphicsview.h
    screensaverplugin.h
    ../common/global_utils.h
    ../common/utils.h
    ../lock-dialog/backenddbushelper.h
    ../userinfo.h
    ../agreementinfo.h
    )
set(screensaver_Plugin_SRC
    ${screensaver_Plugin_SRC}
    chinesedate.cpp
    mbackground.cpp
    screensaver.cpp
    cyclelabel.cpp
    scconfiguration.cpp
    sleeptime.cpp
    checkbutton.cpp
    weathermanager.cpp
    networkwatcher.cpp
    videoplayer.cpp
    graphicsview.cpp
    customplugin.cpp
    ../common/global_utils.cpp
    ../common/utils.cpp
    ../lock-dialog/backenddbushelper.cpp
    ../userinfo.cpp
    ../agreementinfo.cpp
    )

add_library(screensaver-default SHARED ${screensaver_Plugin_SRC})
target_link_libraries(screensaver-default Qt5::Core Qt5::Widgets Qt5::DBus Qt5::X11Extras Qt5::Xml Qt5::Network ukui-log4qt Qt5::Multimedia Qt5::MultimediaWidgets  avformat avcodec ${EXTRA_LIBS})

qt5_add_resources(Screensaver_SRC
        default.qrc
        )

qt5_wrap_cpp(Screensaver_SRC
    chinesedate.h
    screensaver.h
    mbackground.h
    cyclelabel.h
    scconfiguration.h
    sleeptime.h
    checkbutton.h
    weathermanager.h
    customplugin.h
    networkwatcher.h
    videoplayer.h
    graphicsview.h
    screensaverplugin.h
    ../common/global_utils.h
    ../common/utils.h
    ../lock-dialog/backenddbushelper.h
    ../userinfo.h
    ../agreementinfo.h
    )
set(Screensaver_SRC
     ${Screensaver_SRC}
    chinesedate.cpp
    mbackground.cpp
    screensaver.cpp
    cyclelabel.cpp
    scconfiguration.cpp
    sleeptime.cpp
    checkbutton.cpp
    weathermanager.cpp
    networkwatcher.cpp
    videoplayer.cpp
    graphicsview.cpp
    customplugin.cpp
    ../common/global_utils.cpp
    ../common/utils.cpp
    ../lock-dialog/backenddbushelper.cpp
    ../userinfo.cpp
    ../agreementinfo.cpp
    )

add_library(Screensaver STATIC ${Screensaver_SRC})
target_link_libraries(Screensaver Qt5::Core Qt5::Widgets Qt5::DBus Qt5::X11Extras Qt5::Xml Qt5::Network ukui-log4qt Qt5::Multimedia Qt5::MultimediaWidgets  avformat avcodec ${EXTRA_LIBS})

install(TARGETS
	ukui-screensaver-default
	screensaver-default
	DESTINATION lib/ukui-screensaver)

install(FILES
        language/screensaver-zh_CN.ini 
	language/screensaver-en_US.ini
        language/screensaver-bo_CN.ini
	language/screensaver-jd.ini
        language/screensaver-zh_CN-ok.ini
        DESTINATION share/ukui-screensaver/language)

install(FILES
	screensaverplugin.h
	DESTINATION include/ukui-screensaver/)
