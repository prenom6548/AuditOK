add_subdirectory(device_auth)
add_subdirectory(hilog)
add_subdirectory(ipc)

add_library(
        system_ability_fwk
        SHARED
        main.cpp
)

add_library(
        samgr_proxy
        SHARED
        main.cpp
)

add_library(
        libpermissionsdk_standard
        SHARED
        main.cpp
)

add_library(
        cesfwk_innerkits
        SHARED
        main.cpp
)

add_library(
        want
        SHARED
        main.cpp
)

add_library(
        appexecfwk_core
        SHARED
        main.cpp
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)