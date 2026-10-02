# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)
project(kylin-ki18n)

enable_testing()

option(KI18N_RS_FULL_INSTALL "安装 KF5 开发布局：CMake 包、头文件、pri、qlogging、可选 QCH" OFF)
option(KI18N_RS_BUILD_QCH "生成 KF5I18n.qch（需 qtbase5-doc、Doxygen、Qt5Core_QCH）" OFF)

# 本地数据根目录（归类存放）；亦可 -DKI18N_DATA_DIR= 指向上游 ki18n 源码根。
set(_KI18N_DATA_DEFAULT "${CMAKE_CURRENT_LIST_DIR}/ki18n-data")
if(NOT EXISTS "${_KI18N_DATA_DEFAULT}/cmake/KF5I18nMacros.cmake.in")
    set(_KI18N_DATA_DEFAULT "${CMAKE_CURRENT_LIST_DIR}/../ki18n")
endif()
set(KI18N_DATA_DIR "${_KI18N_DATA_DEFAULT}" CACHE PATH
    "KI18n 静态数据根：本地 ki18n-data/ 或完整上游 ki18n 树")

if(NOT EXISTS "${KI18N_DATA_DIR}/cmake/KF5I18nMacros.cmake.in")
    message(FATAL_ERROR "KI18N_DATA_DIR=${KI18N_DATA_DIR}: missing cmake/KF5I18nMacros.cmake.in")
endif()
if(NOT IS_DIRECTORY "${KI18N_DATA_DIR}/po")
    message(FATAL_ERROR "KI18N_DATA_DIR=${KI18N_DATA_DIR}: missing po/")
endif()

# LocaleData 公共头：本地为 headers/localedata/，上游树为 src/localedata/
if(EXISTS "${KI18N_DATA_DIR}/headers/localedata/kcountry.h")
    set(KI18N_LOCALEDATA_HEADERS_DIR "${KI18N_DATA_DIR}/headers/localedata")
elseif(EXISTS "${KI18N_DATA_DIR}/src/localedata/kcountry.h")
    set(KI18N_LOCALEDATA_HEADERS_DIR "${KI18N_DATA_DIR}/src/localedata")
else()
    message(FATAL_ERROR "KI18N_DATA_DIR=${KI18N_DATA_DIR}: missing headers/localedata/ or src/localedata/ API headers")
endif()

set(KI18N_DATA_DOCS_DIR "${KI18N_DATA_DIR}/docs")
set(KI18N_QLOGGING_DIR "${KI18N_DATA_DIR}/qlogging-categories")

# 获取 multiarch 路径
execute_process(COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
    OUTPUT_VARIABLE ARCH_TRIPLET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CMAKE_INSTALL_LIBDIR "/usr/lib/${ARCH_TRIPLET}" CACHE STRING "library installation directory")
set(CMAKE_INSTALL_FULL_LIBDIR "/usr/lib/${ARCH_TRIPLET}" CACHE STRING "full library installation directory")

message(STATUS "ARCH_TRIPLET: ${ARCH_TRIPLET}")
message(STATUS "CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "KI18N_DATA_DIR: ${KI18N_DATA_DIR}")
message(STATUS "KI18N_LOCALEDATA_HEADERS_DIR: ${KI18N_LOCALEDATA_HEADERS_DIR}")

add_custom_target(ki18n_rs_build ALL
    COMMAND cargo build --release --manifest-path ${CMAKE_SOURCE_DIR}/Cargo.toml
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

find_package(Gettext REQUIRED)
find_package(PythonInterp REQUIRED)

file(COPY ${KI18N_DATA_DIR}/cmake/build-pofiles.cmake DESTINATION ${CMAKE_BINARY_DIR}/cmake)
file(COPY ${KI18N_DATA_DIR}/cmake/build-tsfiles.cmake DESTINATION ${CMAKE_BINARY_DIR}/cmake)
file(COPY ${KI18N_DATA_DIR}/cmake/ts-pmap-compile.py DESTINATION ${CMAKE_BINARY_DIR}/cmake)

file(COPY ${KI18N_DATA_DIR}/po DESTINATION ${CMAKE_BINARY_DIR})

set(_KI18N_PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE})
configure_file(
    ${KI18N_DATA_DIR}/cmake/KF5I18nMacros.cmake.in
    ${CMAKE_BINARY_DIR}/cmake/KF5I18nMacros.cmake
    @ONLY
)

include(${CMAKE_BINARY_DIR}/cmake/KF5I18nMacros.cmake)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/share/locale)
KI18N_INSTALL(po)

add_subdirectory(cpp/src)
add_subdirectory(cpp/localedata)
add_subdirectory(cpp/localedata-qml)
add_subdirectory(tests)

include(${CMAKE_SOURCE_DIR}/cmake/kf5_install_targets.cmake)
if(KI18N_RS_FULL_INSTALL)
    include(${CMAKE_SOURCE_DIR}/cmake/kf5_devel_install.cmake)
endif()
