add_custom_target(Headers)
add_custom_command(
    TARGET Headers
    POST_BUILD
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/gen-emugl-headers.sh ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory(libDisplayControl)
add_subdirectory(displayControl_dec)
