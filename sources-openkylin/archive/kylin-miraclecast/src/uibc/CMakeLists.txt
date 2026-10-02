find_package(PkgConfig)
pkg_check_modules (GLIB2 REQUIRED glib-2.0)
pkg_check_modules (GIO2 REQUIRED gio-2.0)
include_directories( ${GLIB2_INCLUDE_DIRS})

set(miracle-uibcctl_SRCS miracle-uibcctl.h
        miracle-uibcctl.c)

add_executable(miracle-uibcctl ${miracle-uibcctl_SRCS})
target_link_libraries(miracle-uibcctl miracle-shared)
target_link_libraries(miracle-uibcctl m)
target_link_libraries(miracle-uibcctl ${GLIB2_LIBRARIES} ${GIO2_LIBRARIES})

install(TARGETS miracle-uibcctl DESTINATION bin)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/shared)
