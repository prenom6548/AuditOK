project(kysdk-datacollect)

add_library(kysdk-datacollect  SHARED libkydatacollect.c)
set_target_properties(kysdk-datacollect PROPERTIES VERSION 3.0.0 SOVERSION 1)

find_package(PkgConfig REQUIRED)

pkg_check_modules(DBUS-1 dbus-1)
target_include_directories(${PROJECT_NAME} PRIVATE ${DBUS-1_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${DBUS-1_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DBUS-1_LIBRARIES} libcjson.so)

pkg_check_modules(OPENSSL openssl)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES})


pkg_check_modules(KYLOG kysdk-log)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYLOG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYLOG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYLOG_LIBRARIES})

execute_process(COMMAND uname -m
            OUTPUT_VARIABLE ARCH_NAME
            OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(PREFIX "")
set(HEADERS_INSTALL_DIR "${PREFIX}/usr/include/kysdk/diagnosetest/")

install(TARGETS kysdk-datacollect LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES libkydatacollect.h DESTINATION ${HEADERS_INSTALL_DIR})
