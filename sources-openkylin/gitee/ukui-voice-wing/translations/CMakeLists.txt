find_program(LRELEASE_EXECUTABLE NAMES lrelease lrelease-qt5)

if(NOT LRELEASE_EXECUTABLE)
    message(FATAL_ERROR "lrelease not found; install qt5-tools or qttools5-dev-tools")
endif()

file(GLOB TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ukui-voice-wing_*.ts")
set(QM_FILES "")

foreach(TS_FILE ${TS_FILES})
    get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
    set(QM_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TS_NAME}.qm)
    list(APPEND QM_FILES ${QM_FILE})

    add_custom_command(
        OUTPUT ${QM_FILE}
        COMMAND ${LRELEASE_EXECUTABLE} ${TS_FILE} -qm ${QM_FILE}
        DEPENDS ${TS_FILE}
        COMMENT "Generating ${QM_FILE}"
        VERBATIM
    )
endforeach()

add_custom_target(ukui-voice-wing-translations ALL DEPENDS ${QM_FILES})

install(FILES ${QM_FILES}
    DESTINATION "${CMAKE_INSTALL_DATADIR}/ukui-voice-wing/translations"
)
