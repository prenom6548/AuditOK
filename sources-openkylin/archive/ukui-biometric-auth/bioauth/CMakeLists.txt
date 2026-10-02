pkg_check_modules(QGS REQUIRED gsettings-qt)

qt5_wrap_ui(BioAuthWidgets_SRC
	src/bioauthwidget.ui
	src/biodeviceswidget.ui
	)

qt5_wrap_cpp(BioAuth_SRC
        include/bioauth.h
        include/biodevices.h
        include/uniauthservice.h
    )

qt5_wrap_cpp(BioAuthWidgets_SRC
        include/bioauthwidget.h
	include/biodeviceswidget.h
	include/giodbus.h
        include/loginoptionswidget.h
	)

set(BioAuth_SRC
	${BioAuth_SRC}
	src/bioauth.cpp
	src/biodevices.cpp
	src/biotypes.cpp
        src/uniauthservice.cpp
    )

set(BioAuthWidgets_SRC
        ${BioAuthWidgets_SRC}
        src/bioauthwidget.cpp
	src/biodeviceswidget.cpp
	src/giodbus.cpp
        src/loginoptionswidget.cpp
	)

include_directories(
	${Qt5Core_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
	${Qt5DBus_INCLUDE_DIRS}
	${GLIB2_INCLUDE_DIRS}
        ${GIOUNIX2_INCLUDE_DIRS}
        ${QGS_INCLUDE_DIRS}
	include
        ../common
	)
add_library(BioAuth STATIC ${BioAuth_SRC})
target_link_libraries(BioAuth Qt5::Core Qt5::DBus Qt5::Svg)
add_library(BioAuthWidgets STATIC ${BioAuthWidgets_SRC})
target_link_libraries(BioAuthWidgets BioAuth Qt5::Widgets ${GIOUNIX2_LIBRARIES} ${QGS_LIBRARIES})
#add_library(BioAuth OBJECT ${BioAuth_SRC})
#add_library(BioAuthWidgets OBJECT ${BioAuth_SRC} ${BioAuthWidgets_SRC})

file(GLOB ts_files i18n_ts/*.ts)
qt5_add_translation(qm_files ${ts_files})
add_custom_target(bioauth_i18n DEPENDS ${qm_files} SOURCES ${ts_files})
add_dependencies(BioAuth bioauth_i18n)

install(FILES ${qm_files} DESTINATION ${UKUI_BIOMETRIC_DIR}/i18n_qm)
