# R05: KF5I18nLocaleData Rust backend bridge
# Builds libKF5I18nLocaleData.so that wraps ki18n_rs_* C API.
# Requires KF5I18n (cpp/src) to be built first for header paths.

cmake_minimum_required(VERSION 3.16)
project(kylin-ki18n-localedata)

set(CMAKE_CXX_STANDARD 17)

option(ENABLE_COVERAGE "Enable gcov coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -fPIC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

find_package(Qt5 REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)

# C API declarations (must match kylin-ki18n/src/capi/locale.rs)
set(LOCALEDATA_C_API
    "extern \"C\" {
        struct Ki18nRsCountryHandle;
        struct Ki18nRsSubdivisionHandle;
        int ki18n_rs_country_from_alpha2(const char*, void**);
        int ki18n_rs_country_from_alpha3(const char*, void**);
        int ki18n_rs_country_from_name(const char*, void**);
        int ki18n_rs_country_from_location(float, float, void**);
        int ki18n_rs_country_all(void**, size_t*);
        int ki18n_rs_country_index(const void*);
        int ki18n_rs_country_handle_from_index(int, void**);
        void ki18n_rs_country_destroy(void*);
        int ki18n_rs_country_alpha2(const void*, char**);
        int ki18n_rs_country_alpha3(const void*, char**);
        int ki18n_rs_country_name(const void*, char**);
        int ki18n_rs_country_emoji_flag(const void*, char**);
        int ki18n_rs_country_currency_code(const void*, char**);
        void ki18n_rs_string_free(char*);
        int ki18n_rs_timezone_from_location(float, float, char**);
        int ki18n_rs_timezone_country(const char*, void**);
    }"
)

set(LOCALEDATA_MOC_HEADERS
    ${KI18N_LOCALEDATA_HEADERS_DIR}/kcountry.h
    ${KI18N_LOCALEDATA_HEADERS_DIR}/kcountrysubdivision.h
)

add_library(KF5I18nLocaleData SHARED
    kcountry.cpp
    kcountrysubdivision.cpp
    ktimezone.cpp
    ${LOCALEDATA_MOC_HEADERS}
)

target_compile_definitions(KF5I18nLocaleData PRIVATE
    KI18NLOCALEDATA_BUILDING_LIBRARY
)

target_include_directories(KF5I18nLocaleData PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KI18N_LOCALEDATA_HEADERS_DIR}
)
target_include_directories(KF5I18nLocaleData
    INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/KF5/KI18nLocaleData>"
)

add_dependencies(KF5I18nLocaleData ki18n_rs_build)
target_link_libraries(KF5I18nLocaleData PUBLIC Qt5::Core PRIVATE
    ${CMAKE_SOURCE_DIR}/target/release/libki18n_rs.a
)
if (TARGET Qt5::CorePrivate)
    target_link_libraries(KF5I18nLocaleData PRIVATE Qt5::CorePrivate)
endif()
target_link_options(KF5I18nLocaleData PRIVATE
    "LINKER:--gc-sections"
    "LINKER:--exclude-libs,libki18n_rs.a"
)

set_target_properties(KF5I18nLocaleData PROPERTIES
    VERSION 5.92.0
    SOVERSION 5
    EXPORT_NAME I18nLocaleData
)
