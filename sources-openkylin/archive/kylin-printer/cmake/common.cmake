function(find_kysdk_package prj_name kysdk_package_name)
    pkg_check_modules(${kysdk_package_name}_PKG ${kysdk_package_name})
    target_include_directories(${prj_name} PUBLIC ${${kysdk_package_name}_PKG_INCLUDE_DIRS})
    target_link_directories(${prj_name} PUBLIC ${${kysdk_package_name}_PKG_LIBRARY_DIRS})
    target_link_libraries(${prj_name} PUBLIC ${${kysdk_package_name}_PKG_LIBRARIES})
endfunction()


function(target_link_kysdk_libraries prj_name)
    find_package(PkgConfig REQUIRED)

    find_kysdk_package(${prj_name} kysdk-qtwidgets)
    find_kysdk_package(${prj_name} kysdk-datacollect)
    find_kysdk_package(${prj_name} kysdk-log)
    find_kysdk_package(${prj_name} kysdk-utils)
    find_kysdk_package(${prj_name} kysdk-config)
    find_kysdk_package(${prj_name} kysdk-waylandhelper)
    find_kysdk_package(${prj_name} kysdk-alm)
    find_kysdk_package(${prj_name} kysdk-sysinfo)
    find_kysdk_package(${prj_name} kysdk-systime)
    find_kysdk_package(${prj_name} kysdk-package)
endfunction()

function(target_link_glib_libraries prj_name)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLIB REQUIRED glib-2.0)
    include_directories(${GLIB_INCLUDE_DIRS})
    target_link_libraries(${prj_name} PUBLIC "-lglib-2.0")
endfunction()

function(target_link_qgsettings_libraries prj_name)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
    target_link_libraries(${prj_name} PUBLIC PkgConfig::QGSETTINGS)
endfunction()

function(target_link_X11_libraries prj_name)
    target_link_libraries(${prj_name} PUBLIC -lX11)
endfunction()

function(target_link_KWindowSystem_libraries prj_name)
    find_package(KF5WindowSystem REQUIRED)
    target_link_libraries(${prj_name} PUBLIC KF5::WindowSystem)
endfunction()

function(target_link_smbclient_libraries prj_name)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SMBCLIENT_PKG smbclient)
    target_include_directories(${prj_name} PUBLIC ${SMBCLIENT_PKG_INCLUDE_DIRS})
    target_link_directories(${prj_name} PUBLIC ${SMBCLIENT_PKG_LIBRARY_DIRS})
    target_link_libraries(${prj_name} PUBLIC ${SMBCLIENT_PKG_LIBRARIES})
endfunction()

function(get_recursive_include_dirs out_var root_dir)
    file(GLOB_RECURSE headers 
         LIST_DIRECTORIES true 
         "${root_dir}/*.h" "${root_dir}/*.hpp")
    
    set(dirs "")
    foreach(header ${headers})
        get_filename_component(dir ${header} DIRECTORY)
        list(APPEND dirs ${dir})
    endforeach()
    
    list(REMOVE_DUPLICATES dirs)
    set(${out_var} ${dirs} PARENT_SCOPE)
endfunction()

