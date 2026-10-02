include(FeatureSummary)
set_package_properties(XCB PROPERTIES
        URL "https://xcb.freedesktop.org/"
        DESCRIPTION "X protocol C-language Binding")

find_package(PkgConfig)
pkg_check_modules(PC_XCB xcb ${XCB_FIND_COMPONENTS})

find_library(XCB_LIBRARIES xcb HINTS ${PC_XCB_LIBRARY_DIRS})
find_path(XCB_INCLUDE_DIRS xcb/xcb.h PATH_SUFFIXES xcb HINTS ${PC_XCB_INCLUDE_DIRS})

foreach(COMPONENT ${XCB_FIND_COMPONENTS})
    find_library(XCB_LIBRARIES_${COMPONENT} ${COMPONENT} HINTS ${PC_XCB_LIBRARY_DIRS})
    list(APPEND XCB_LIBRARIES ${XCB_LIBRARIES_${COMPONENT}})
    mark_as_advanced(XCB_LIBRARIES_${COMPONENT})
endforeach(COMPONENT ${XCB_FIND_COMPONENTS})

set(XCB_DEFINITIONS ${PC_XCB_CFLAGS_OTHER})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XCB DEFAULT_MSG XCB_LIBRARIES XCB_INCLUDE_DIRS)
mark_as_advanced(XCB_INCLUDE_DIRS XCB_LIBRARIES XCB_DEFINITIONS)