#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
# 
# See the LICENSE file for more details.
#

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

pkg_search_module(elf_static REQUIRED IMPORTED_TARGET libelf)
# pkg_search_module(deflate_static REQUIRED IMPORTED_TARGET libdeflate)
pkg_search_module(lz4_static REQUIRED IMPORTED_TARGET liblz4)
pkg_search_module(lzma_static REQUIRED IMPORTED_TARGET liblzma)
pkg_search_module(zlib_static REQUIRED IMPORTED_TARGET zlib)
pkg_search_module(zstd_static REQUIRED IMPORTED_TARGET libzstd)
pkg_search_module(uuid_static REQUIRED IMPORTED_TARGET uuid)
pkg_search_module(fuse_static REQUIRED IMPORTED_TARGET fuse)
pkg_search_module(lzo2_static REQUIRED IMPORTED_TARGET lzo2)

# 查找库文件，V10 SP1中libdeflate-dev中没有.pc文件，不能用pkgconfig查找
find_library(FOUND_deflate_static NAMES deflate)
if("${FOUND_deflate_static}" STREQUAL "")
  message(FATAL_ERROR "libdeflate.a not fount")
endif()
get_filename_component(LIBDEFLATE_ABS_FILE ${FOUND_deflate_static} ABSOLUTE)
message(STATUS "found libdeflate.a: ${LIBDEFLATE_ABS_FILE}")

# 查找头文件路径
find_path(deflate_static_INCLUDE_DIRS NAMES libdeflate.h)
if (NOT deflate_static_INCLUDE_DIRS)
    message(FATAL_ERROR "Could not find libdeflate.h")
endif()
message(STATUS "Found libdeflate.h at: ${deflate_static_INCLUDE_DIRS}")

# 查找库文件，V10 SP1中libdl.a不自动加载
find_library(FOUND_dl_static NAMES dl)
if("${FOUND_dl_static}" STREQUAL "")
  message(FATAL_ERROR "libdl.a not fount")
endif()
get_filename_component(LIBDL_ABS_FILE ${FOUND_dl_static} ABSOLUTE)
message(STATUS "found libdl.a: ${LIBDL_ABS_FILE}")

# 查找头文件路径
find_path(dl_static_INCLUDE_DIRS NAMES dlfcn.h)
if (NOT dl_static_INCLUDE_DIRS)
    message(FATAL_ERROR "Could not find dlfcn.h")
endif()
message(STATUS "Found dlfcn.h at: ${dl_static_INCLUDE_DIRS}")

# add_link_options(-static -static-libgcc -static-libstdc++)

set(ONLY_TO_TERMINAL 1)
configure_file(config.h.in config.h)

aux_source_directory(squashfuse SRC_SQUASHFUSE)

add_executable(ok-engine
    main.cpp
    KMOABApplication.h
    KMOABApplication.cpp
    KMOABContext.h
    KMOABElf.h
    KMOABElf.cpp
    KMOABExtract.h
    KMOABExtract.cpp
    KMOABPrintMeta.h
    KMOABPrintMeta.cpp
    KMOABUtils.h
    KMOABUtils.cpp
    KMOABRepackage.h
    KMOABRepackage.cpp
    KMOABPrintDataDir.h
    KMOABPrintDataDir.cpp
    KMOABMount.h
    KMOABMount.cpp
    ../../common/KMBuildinOptions.h
    ../../common/KMBuildinOptions.cpp
    ../../common/KMException.h
    ../../common/KMException.cpp
    ../../common/KMObject.h
    ../../common/KMObject.cpp
    ../../common/KMSingleton.h
    ../../common/KMSubCommand.h
    ../../common/KMSubCommand.cpp
    ../../common/KMDynamicCreator.h
    ../../common/KMDynamicCreator.cpp
    ../../common/KMOABMetadata.h
    ../../common/KMOABMetadata.cpp
    ../../common/KMStringUtils.h
    ../../common/KMStringUtils.cpp
    ../../common/KMJsonHelper.h
    ../../common/KMJsonHelper.cpp
    ../../common/KMInfoJson.h
    ../../common/KMInfoJson.cpp
    ../../common/KMLogger.h
    ../../common/KMLogger.cpp
    ../../common/KMConfigData.h
    ../../common/KMCrun.h
    ../../common/KMCrun.cpp
    ../../common/KMContainer.h
    ../../common/KMContainer.cpp
    ../../common/KMOverlay.h
    ../../common/KMOverlay.cpp
    ${SRC_SQUASHFUSE}
)

target_include_directories(ok-engine PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${elf_static_INCLUDE_DIRS}
    ${fuse_static_INCLUDE_DIRS}
    ${dl_static_INCLUDE_DIRS}
    ${lzo2_static_INCLUDE_DIRS}
    ${deflate_static_INCLUDE_DIRS}
    ${lz4_static_INCLUDE_DIRS}
    ${lzma_static_INCLUDE_DIRS}
    ${zlib_static_INCLUDE_DIRS}
    ${zstd_static_INCLUDE_DIRS}
    ${uuid_static_INCLUDE_DIRS}
)

# v10sp1中libfuse.a链接不到libdl.a; openkylin2.0中ld版本为2.41，此为规避方案
if(CMAKE_CXX_COMPILER_LINKER_VERSION VERSION_LESS "2.41")
    target_link_libraries(ok-engine PRIVATE
        -Wl,-Bstatic
        # ${elf_static_LIBRARIES}
        PkgConfig::elf_static
        PkgConfig::fuse_static
        # PkgConfig::deflate_static
        ${LIBDEFLATE_ABS_FILE}
        PkgConfig::lz4_static
        PkgConfig::lzma_static
        PkgConfig::zlib_static
        PkgConfig::zstd_static
        PkgConfig::lzo2_static
        PkgConfig::uuid_static
        # stdc++fs
        -Wl,-Bdynamic
        fuse
    )

    target_link_options(ok-engine PRIVATE -static-libgcc -static-libstdc++)
else()
    target_link_libraries(ok-engine PRIVATE
        # ${elf_static_LIBRARIES}
        PkgConfig::elf_static
        PkgConfig::fuse_static
        # PkgConfig::deflate_static
        ${LIBDEFLATE_ABS_FILE}
        PkgConfig::lz4_static
        PkgConfig::lzma_static
        PkgConfig::zlib_static
        PkgConfig::zstd_static
        PkgConfig::lzo2_static
        PkgConfig::uuid_static
        # stdc++fs
    )

    target_link_options(ok-engine PRIVATE -static -static-libgcc -static-libstdc++)
endif()

install( TARGETS ok-engine
    RUNTIME DESTINATION  /opt/kaiming-tools/bin
)
