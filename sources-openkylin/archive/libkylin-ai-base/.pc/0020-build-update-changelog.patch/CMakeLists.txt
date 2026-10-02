cmake_minimum_required(VERSION 3.5)

project(kylin-ai-base LANGUAGES CXX C)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#######################################################################
# Options
#######################################################################
option(ENABLE_TEST "Build Test" OFF)

include_directories(include)
include_directories(src)

find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
pkg_check_modules(GIO REQUIRED gio-unix-2.0)

find_package(jsoncpp REQUIRED)

include_directories(${GIO_INCLUDE_DIRS})
pkg_check_modules(JSON REQUIRED json-glib-1.0)
include_directories(${GIO_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS})
include_directories(include/kylin-ai/ai-base)
include_directories(include/kylin-ai)

set(SERVICE_SOURCES
    src/services/textprocessorproxy.cpp
    src/services/textprocessorglue.c
    src/services/speechprocessorproxy.cpp
    src/services/speechprocessorglue.c
    src/services/modelmanagerserviceproxy.cpp
    src/services/modelmanagerserviceglue.c
    src/services/serverproxy.cpp
    src/services/visionprocessorproxy.cpp
    src/services/visionprocessorglue.c
    src/services/datamanagementprocessorglue.c
    src/services/datamanagementprocessorglue.h
    src/services/datamanagementprocessorproxy.cpp
    src/services/datamanagementprocessorproxy.h
)

add_library(kylin-ai-base SHARED
    include/kylin-ai/ai-base/modelconfig.h
    include/kylin-ai/ai-base/remotemodelvendor.h
    include/kylin-ai/ai-base/vision.h
    include/kylin-ai/ai-base/nlp.h
    include/kylin-ai/ai-base/speech.h
    include/kylin-ai/ai-base/ocr.h
    include/kylin-ai/ai-base/private/datamanagement.h
    include/kylin-ai/config.h
    src/modelconfig.cpp
    src/nlp.cpp
    src/vision.cpp
    src/visionsession.h
    src/visionsession.cpp
    src/nlpsession.h
    src/nlpsession.cpp
    src/speech.cpp
    src/config.cpp
    ${SERVICE_SOURCES}
    src/speechsession.h src/speechsession.cpp
    src/datamanagementsession.h src/datamanagementsession.cpp
    include/kylin-ai/ai-base/datamanagement.h src/datamanagement.cpp
    src/settings/settings.h src/settings/settings.cpp
    src/settings/globalsettings.h src/settings/globalsettings.cpp
    src/settings/childsettings.h src/settings/childsettings.cpp
    include/kylin-ai/base.h
)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

set_target_properties(kylin-ai-base PROPERTIES VERSION 2.0.0 SOVERSION 2)

target_link_libraries(
    kylin-ai-base
    ${GIO_LIBRARIES}
    jsoncpp
    ${JSON_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
install(TARGETS kylin-ai-base
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/kylin-ai
    DESTINATION include)

install(FILES pkg-configs/kylin-ai-base.pc DESTINATION share/pkgconfig/)
install(FILES data/org.openkylin.aisdk.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
