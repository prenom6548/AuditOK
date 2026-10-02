# SPDX-FileCopyrightText: 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd. <zhangpengfei@kylinos.cn>

# SPDX-License-Identifier: BSD-3-Clause

include(${ECM_MODULE_DIR}ECMFindModuleHelpers.cmake)

ecm_find_package_version_check(WaylandScanner)

# Find wayland-scanner
find_program(WaylandScanner_EXECUTABLE NAMES wayland-scanner)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WaylandScanner
        FOUND_VAR
        WaylandScanner_FOUND
        REQUIRED_VARS
        WaylandScanner_EXECUTABLE
)

mark_as_advanced(WaylandScanner_EXECUTABLE)

if(NOT TARGET Wayland::Scanner AND WaylandScanner_FOUND)
    add_executable(Wayland::Scanner IMPORTED)
    set_target_properties(Wayland::Scanner PROPERTIES
            IMPORTED_LOCATION "${WaylandScanner_EXECUTABLE}"
    )
endif()

include(FeatureSummary)
set_package_properties(WaylandScanner PROPERTIES
        URL "https://wayland.freedesktop.org/"
        DESCRIPTION "Executable that converts XML protocol files to C code"
)

function(ukui_add_wayland_client_protocol target_or_sources_var)
    # Parse arguments
    set(options PRIVATE_CODE)
    set(oneValueArgs PROTOCOL BASENAME)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "" ${ARGN})

    if(ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown keywords given to ecm_add_wayland_client_protocol(): \"${ARGS_UNPARSED_ARGUMENTS}\"")
    endif()

    get_filename_component(_infile ${ARGS_PROTOCOL} ABSOLUTE)
    set(_client_header "${CMAKE_CURRENT_BINARY_DIR}/wayland-${ARGS_BASENAME}-client-protocol.h")
    set(_code "${CMAKE_CURRENT_BINARY_DIR}/wayland-${ARGS_BASENAME}-protocol.c")
    if(ARGS_PRIVATE_CODE)
        set(_code_type private-code)
    else()
        set(_code_type public-code)
    endif()

    set_source_files_properties(${_client_header} GENERATED)
    set_source_files_properties(${_code} GENERATED)
    set_property(SOURCE ${_client_header} ${_code} PROPERTY SKIP_AUTOMOC ON)

    add_custom_command(OUTPUT "${_client_header}"
            COMMAND ${WaylandScanner_EXECUTABLE} client-header ${_infile} ${_client_header}
            DEPENDS ${_infile} VERBATIM)

    add_custom_command(OUTPUT "${_code}"
            COMMAND ${WaylandScanner_EXECUTABLE} ${_code_type} ${_infile} ${_code}
            DEPENDS ${_infile} ${_client_header} VERBATIM)

    if (TARGET ${target_or_sources_var})
        target_sources(${target_or_sources_var} PRIVATE "${_client_header}" "${_code}")
    else()
        list(APPEND ${target_or_sources_var} "${_client_header}" "${_code}")
        set(${target_or_sources_var} ${${target_or_sources_var}} PARENT_SCOPE)
    endif()
endfunction()