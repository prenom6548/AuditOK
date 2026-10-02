set(GENERATED_SOURCES
    displayControl_enc.cpp
    displayControl_entry.cpp
    displayControl_client_context.cpp)

add_custom_command(
    OUTPUT ${GENERATED_SOURCES}
    POST_BUILD
    COMMAND ${CMAKE_BINARY_DIR}/host/tools/emugen/emugen
            -i ${CMAKE_SOURCE_DIR}/host/libs/displayControl_dec
            -E ${CMAKE_CURRENT_SOURCE_DIR} displayControl
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS emugen)

