remove_definitions(-DQT_NO_CAST_FROM_ASCII)
add_executable(ukuiwaylandscanner ukuiwaylandscanner.cpp)
target_link_libraries(ukuiwaylandscanner Qt::Core)

find_program(WaylandScanner_EXECUTABLE NAMES wayland-scanner)

function(add_wayland_client_protocol target_or_sources_var)
    # Parse arguments
    set(oneValueArgs PROTOCOL BASENAME)
    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "" ${ARGN})

    get_filename_component(_infile ${ARGS_PROTOCOL} ABSOLUTE)
    set(_client_header "${CMAKE_CURRENT_BINARY_DIR}/wayland-${ARGS_BASENAME}-client-protocol.h")
    set(_code "${CMAKE_CURRENT_BINARY_DIR}/wayland-${ARGS_BASENAME}-protocol.c")

    set_source_files_properties(${_client_header} GENERATED)
    set_source_files_properties(${_code} GENERATED)
    set_property(SOURCE ${_client_header} ${_code} PROPERTY SKIP_AUTOMOC ON)

    add_custom_command(OUTPUT "${_client_header}"
        COMMAND ${WaylandScanner_EXECUTABLE} client-header ${_infile} ${_client_header}
        DEPENDS ${_infile} VERBATIM)

    add_custom_command(OUTPUT "${_code}"
        COMMAND ${WaylandScanner_EXECUTABLE} private-code ${_infile} ${_code}
        DEPENDS ${_infile} ${_client_header} VERBATIM)

    if (TARGET ${target_or_sources_var})
        target_sources(${target_or_sources_var} PRIVATE "${_client_header}" "${_code}")
    else()
        list(APPEND ${target_or_sources_var} "${_client_header}" "${_code}")
        set(${target_or_sources_var} ${${target_or_sources_var}} PARENT_SCOPE)
    endif()
endfunction()

function(ukui_add_wayland_client_protocol out_var)
    # Parse arguments
    set(oneValueArgs PROTOCOL BASENAME PREFIX)
    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "" ${ARGN})

    set(_prefix "${ARGS_PREFIX}")

    add_wayland_client_protocol(${out_var}
                                     PROTOCOL ${ARGS_PROTOCOL}
                                     BASENAME ${ARGS_BASENAME})

    get_filename_component(_infile ${ARGS_PROTOCOL} ABSOLUTE)
    set(_header "${CMAKE_CURRENT_BINARY_DIR}/ukui-${ARGS_BASENAME}.h")
    set(_code "${CMAKE_CURRENT_BINARY_DIR}/ukui-${ARGS_BASENAME}.cpp")

    set_source_files_properties(${_header} ${_code} GENERATED)

    add_custom_command(OUTPUT "${_header}"
        COMMAND ukuiwaylandscanner client-header ${_infile} "" ${_prefix} > ${_header}
        DEPENDS ${_infile} ukuiwaylandscanner VERBATIM)

    add_custom_command(OUTPUT "${_code}"
        COMMAND ukuiwaylandscanner client-code ${_infile} "" ${_prefix} > ${_code}
        DEPENDS ${_infile} ${_header} ukuiwaylandscanner VERBATIM)

    set_property(SOURCE ${_header} ${_code} PROPERTY SKIP_AUTOMOC ON)

    list(APPEND ${out_var} "${_code}")
    set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()
