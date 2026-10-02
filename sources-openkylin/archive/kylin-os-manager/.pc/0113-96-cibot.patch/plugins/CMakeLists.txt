cmake_minimum_required(VERSION 3.16)

project(plugins)

if (ICBC_OPTION)
    add_subdirectory(backup_restore)
endif ()
add_subdirectory(fault_diagnosis)
add_subdirectory(junk_clean)
add_subdirectory(tool_box)
add_subdirectory(service-support)
add_subdirectory(device-controller)
#add_subdirectory(plugin-template)
