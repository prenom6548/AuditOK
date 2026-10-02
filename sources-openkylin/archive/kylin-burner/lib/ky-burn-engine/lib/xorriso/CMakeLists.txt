cmake_minimum_required(VERSION 3.10)
project(xorriso)

# 设置C标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

# Setup the environment
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
# 包含目录
include_directories(
    ./libburn
    ./libisofs
    ./libisoburn
    ./xorriso
    ./include
)

# 定义源文件
set(XORRISO_SOURCES
#    main.cpp
    xorriso/xorriso.h
    xorriso/xorriso_private.h
#   xorriso/xorriso_main.c
    xorriso/sfile.h
    xorriso/sfile.c
    xorriso/aux_objects.h
    xorriso/aux_objects.c
    xorriso/findjob.h
    xorriso/findjob.c
    xorriso/check_media.h
    xorriso/check_media.c
    xorriso/misc_funct.h
    xorriso/misc_funct.c
    xorriso/text_io.h
    xorriso/text_io.c
    xorriso/match.h
    xorriso/match.c
    xorriso/emulators.h
    xorriso/emulators.c
    xorriso/disk_ops.h
    xorriso/disk_ops.c
    xorriso/cmp_update.h
    xorriso/cmp_update.c
    xorriso/parse_exec.h
    xorriso/parse_exec.c
    xorriso/opts_a_c.c
    xorriso/opts_d_h.c
    xorriso/opts_i_o.c
    xorriso/opts_p_z.c
    xorriso/xorrisoburn.h
    xorriso/base_obj.h
    xorriso/base_obj.c
    xorriso/lib_mgt.h
    xorriso/lib_mgt.c
    xorriso/sort_cmp.h
    xorriso/sort_cmp.c
    xorriso/drive_mgt.h
    xorriso/drive_mgt.c
    xorriso/iso_img.h
    xorriso/iso_img.c
    xorriso/iso_tree.h
    xorriso/iso_tree.c
    xorriso/iso_manip.h
    xorriso/iso_manip.c
    xorriso/write_run.h
    xorriso/write_run.c
    xorriso/read_run.h
    xorriso/read_run.c
    xorriso/filters.h
    xorriso/filters.c
    xorriso/xorriso_timestamp.h
    xorriso/xorriso_buildstamp.h

    libisoburn/libisoburn.h
    libisoburn/isoburn.h
    libisoburn/isoburn.c
    libisoburn/isofs_wrap.c
    libisoburn/burn_wrap.c
    libisoburn/data_source.c
    
    libisofs/libisofs.h
    libisofs/builder.h
    libisofs/builder.c
    libisofs/node.h
    libisofs/node.c
    libisofs/tree.h
    libisofs/tree.c
    libisofs/image.h
    libisofs/image.c
    libisofs/iso1999.h
    libisofs/iso1999.c
    libisofs/fsource.h
    libisofs/fsource.c
    libisofs/fs_local.c
    libisofs/fs_image.c
    libisofs/messages.h
    libisofs/messages.c
    libisofs/libiso_msgs.h
    libisofs/libiso_msgs.c
    libisofs/stream.h
    libisofs/stream.c
    libisofs/util.h
    libisofs/util.c
    libisofs/util_rbtree.c
    libisofs/util_htable.c
    libisofs/filesrc.h
    libisofs/filesrc.c
    libisofs/ecma119.h
    libisofs/ecma119.c
    libisofs/ecma119_tree.h
    libisofs/ecma119_tree.c
    libisofs/writer.h
    libisofs/buffer.h
    libisofs/buffer.c
    libisofs/rockridge.h
    libisofs/rockridge.c
    libisofs/rockridge_read.c
    libisofs/joliet.h
    libisofs/joliet.c
    libisofs/hfsplus.h
    libisofs/hfsplus.c
    libisofs/hfsplus_decompose.c
    libisofs/hfsplus_classes.c
    libisofs/hfsplus_case.c
    libisofs/eltorito.h
    libisofs/eltorito.c
    libisofs/data_source.c
    libisofs/find.c
    libisofs/filter.h
    libisofs/filter.c
    libisofs/filters/external.c
    libisofs/filters/zisofs.c
    libisofs/filters/gzip.c
    libisofs/system_area.h
    libisofs/system_area.c
    libisofs/make_isohybrid_mbr.c
    libisofs/aaip_0_2.h
    libisofs/aaip_0_2.c
    libisofs/md5.h
    libisofs/md5.c

    libburn/async.c
    libburn/async.h
    libburn/back_hacks.h
    libburn/cdtext.c
    libburn/cleanup.c
    libburn/cleanup.h
    libburn/crc.h
    libburn/debug.c
    libburn/debug.h
    libburn/drive.c
    libburn/drive.h
    libburn/ecma130ab.c
    libburn/ecma130ab.h
    libburn/error.h
    libburn/file.c
    libburn/file.h
    libburn/init.c
    libburn/init.h
    libburn/libburn.h
    libburn/libdax_audioxtr.h
    libburn/libdax_audioxtr.c
    libburn/libdax_msgs.h
    libburn/libdax_msgs.c
    libburn/mmc.c
    libburn/mmc.h
    libburn/null.c
    libburn/null.h
    libburn/options.c
    libburn/options.h
    libburn/os.h
    libburn/read.c
    libburn/read.h
    libburn/sbc.c
    libburn/sbc.h
    libburn/sector.c
    libburn/sector.h
    libburn/sg.c
    libburn/sg.h
    libburn/source.h
    libburn/source.c
    libburn/spc.c
    libburn/spc.h
    libburn/structure.c
    libburn/structure.h
    libburn/toc.c
    libburn/toc.h
    libburn/transport.h
    libburn/util.c
    libburn/util.h
    libburn/write.c
    libburn/write.h

    libjte/libjte.h  
    libjte/libjte_private.h
    libjte/libjte.c
    libjte/jte.h
    libjte/jte.c
    libjte/checksum.h
    libjte/checksum.c
    libjte/endian.c
    libjte/endianconv.h
    libjte/md5.h
    libjte/md5.c
    libjte/rsync.h
    libjte/rsync.c
    libjte/sha1.h
    libjte/sha1.c
    libjte/sha256.h
    libjte/sha256.c
    libjte/sha512.h
    libjte/sha512.c
)

# public include
file(GLOB_RECURSE PUBLIC_INCLUDES  CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*"
    )

# 定义可执行文件

add_library(xorriso STATIC
    ${PUBLIC_INCLUDES}
    ${XORRISO_SOURCES}
    )

target_link_directories(xorriso
PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# 链接库
target_link_libraries(xorriso 
    Qt5::Core
    readline
    ncurses
    pthread
    acl
    ZLIB::ZLIB
    bz2
)

# 安装目标
install(TARGETS xorriso DESTINATION /usr/lib/kylin-burner/)
