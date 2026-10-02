function(kylin_ai_generate_gdbus_proto_code RETURNED_GDBUS_GLUE_FILES) #REQUIRED_GLIB_MIN_VERSION #XMLFILENAMES)
    set(REQUIRED_GLIB_MIN_VERSION)
    set(XML_FILE_NAMES)
    set(XML_FILES)
    set(GLUE_FILES)

    foreach(ARG ${ARGN})
        if(ARG MATCHES "^--glib-min-required=([0-9]+)\\.([0-9]+)$")
            set(REQUIRED_GLIB_MIN_VERSION ${ARG})
            message(STATUS "gdbus-codegen: --glib-min-required=${ARG}")
        else()
            list(APPEND XML_FILE_NAMES ${ARG})
        endif()
    endforeach()

    foreach(XML_FILE_NAME ${XML_FILE_NAMES})
        set(XML_FILE /usr/share/kylin-ai/protocols/gdbus/${XML_FILE_NAME}.xml)
        if(NOT EXISTS ${XML_FILE})
            message(FATAL_ERROR "gdbus-codegen: Unable to find XML file: ${XML_FILE_NAME}")
        else()
            list(APPEND XML_FILES ${XML_FILE})
        endif()
    endforeach()

    foreach(PROTO_FILE ${XML_FILES})
        get_filename_component(PROTO_FILE_NAME ${PROTO_FILE} NAME)
        string(REPLACE ".xml" "" PROTO_FILE_NAME ${PROTO_FILE_NAME})
        execute_process(
            COMMAND /usr/bin/gdbus-codegen ${REQUIRED_GLIB_MIN_VERSION} --output-directory=${CMAKE_CURRENT_BINARY_DIR}
                --generate-c-code=${PROTO_FILE_NAME}glue --interface-prefix=com.kylin ${PROTO_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GDBUS_RESULT
        )

        if(NOT GDBUS_RESULT EQUAL 0)
            message(FATAL_ERROR "gdbus-codegen command failed with error code: ${GDBUS_RESULT}")
        else()
            list(APPEND GLUE_FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_FILE_NAME}glue.c")
            list(APPEND GLUE_FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_FILE_NAME}glue.h")
        endif()
    endforeach()

    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    set(${RETURNED_GDBUS_GLUE_FILES} ${GLUE_FILES} PARENT_SCOPE)
endfunction()