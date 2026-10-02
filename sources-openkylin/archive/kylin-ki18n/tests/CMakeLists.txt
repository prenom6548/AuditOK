# SPDX-FileCopyrightText: 2024 KI18n Rust Backend
# SPDX-License-Identifier: LGPL-2.0-or-later

cmake_minimum_required(VERSION 3.16)

option(ENABLE_COVERAGE "Enable gcov coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fPIC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Test)

set(CMAKE_AUTOMOC ON)

add_executable(cpp_localedata_test
    cpp_localedata_test.cpp
)

target_include_directories(cpp_localedata_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/include
    ${KI18N_LOCALEDATA_HEADERS_DIR}
)

target_link_libraries(cpp_localedata_test PRIVATE
    Qt5::Core
    Qt5::Test
    KF5I18nLocaleData
)

add_test(NAME cpp_localedata_test COMMAND cpp_localedata_test)

add_executable(cpp_kf5i18n_test
    cpp_kf5i18n_test.cpp
)

target_include_directories(cpp_kf5i18n_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/include/KLocalizedString
    ${KI18N_LOCALEDATA_HEADERS_DIR}
)

target_link_libraries(cpp_kf5i18n_test PRIVATE
    Qt5::Core
    Qt5::Test
    KF5I18n
)

add_test(NAME cpp_kf5i18n_test COMMAND cpp_kf5i18n_test)