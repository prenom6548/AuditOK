find_package(Qt5LinguistTools)

file(GLOB ts_files *.ts)
qt5_add_translation(qm_files ${ts_files})

add_custom_target(ukccplugins_i18n
	DEPENDS ${qm_files}
	SOURCES ${ts_files}
	)

install(FILES ${ts_files} DESTINATION /usr/share/ukui-screensaver/ukccplugins/i18n/)
install(FILES ${qm_files} DESTINATION /usr/share/ukui-screensaver/ukccplugins/i18n_qm/)
