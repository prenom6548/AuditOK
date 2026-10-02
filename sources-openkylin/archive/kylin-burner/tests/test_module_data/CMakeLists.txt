set(CMAKE_CXX_STANDARD 14)

set(SOURCES
    ${PROJECT_ROOT_DIR}/rc/kylin-burner.qrc
    ${PROJECT_ROOT_DIR}/src/common/utils.h
    ${PROJECT_ROOT_DIR}/src/common/utils.cpp
    ${PROJECT_ROOT_DIR}/src/common/dbuscaller.h
    ${PROJECT_ROOT_DIR}/src/common/dbuscaller.cpp
    ${PROJECT_ROOT_DIR}/src/common/trackingmanager.h
    ${PROJECT_ROOT_DIR}/src/common/trackingmanager.cpp
    ${PROJECT_ROOT_DIR}/src/common/encodingdetector.cpp
    ${PROJECT_ROOT_DIR}/src/common/encodingdetector.h
    ${PROJECT_ROOT_DIR}/src/frame/xatom-helper.cpp
    ${PROJECT_ROOT_DIR}/src/frame/xatom-helper.h
    ${PROJECT_ROOT_DIR}/src/frame/logger.cpp
    ${PROJECT_ROOT_DIR}/src/frame/logger.h
    ${PROJECT_ROOT_DIR}/src/frame/configuration.cpp
    ${PROJECT_ROOT_DIR}/src/frame/configuration.h
    ${PROJECT_ROOT_DIR}/src/frame/signals.cpp
    ${PROJECT_ROOT_DIR}/src/frame/signals.h
    ${PROJECT_ROOT_DIR}/src/frame/auditlog.h
    ${PROJECT_ROOT_DIR}/src/frame/auditlog.cpp
    ${PROJECT_ROOT_DIR}/src/po/translation.h
    ${PROJECT_ROOT_DIR}/src/view/mainwindow.cpp
    ${PROJECT_ROOT_DIR}/src/view/mainwindow.h
    ${PROJECT_ROOT_DIR}/src/view/mainwindow.ui
    ${PROJECT_ROOT_DIR}/src/view/windowtitle.cpp
    ${PROJECT_ROOT_DIR}/src/view/windowtitle.h
    ${PROJECT_ROOT_DIR}/src/view/windowtitle.ui
    ${PROJECT_ROOT_DIR}/src/view/burndata.cpp
    ${PROJECT_ROOT_DIR}/src/view/burndata.h
    ${PROJECT_ROOT_DIR}/src/view/burndata.ui
    ${PROJECT_ROOT_DIR}/src/view/burnimage.cpp
    ${PROJECT_ROOT_DIR}/src/view/burnimage.h
    ${PROJECT_ROOT_DIR}/src/view/burnimage.ui
    ${PROJECT_ROOT_DIR}/src/view/components/clicklabel.h
    ${PROJECT_ROOT_DIR}/src/view/components/clicklabel.cpp
    ${PROJECT_ROOT_DIR}/src/view/components/iconlabel.h
    ${PROJECT_ROOT_DIR}/src/view/components/iconlabel.cpp
    ${PROJECT_ROOT_DIR}/src/view/copydisc.cpp
    ${PROJECT_ROOT_DIR}/src/view/copydisc.h
    ${PROJECT_ROOT_DIR}/src/view/copydisc.ui
    ${PROJECT_ROOT_DIR}/src/view/modaldialog.cpp
    ${PROJECT_ROOT_DIR}/src/view/modaldialog.h
    ${PROJECT_ROOT_DIR}/src/view/modaldialog.ui
    ${PROJECT_ROOT_DIR}/src/view/execdialog.cpp
    ${PROJECT_ROOT_DIR}/src/view/execdialog.h
    ${PROJECT_ROOT_DIR}/src/view/execdialog.ui
    ${PROJECT_ROOT_DIR}/src/view/input.cpp
    ${PROJECT_ROOT_DIR}/src/view/input.h
    ${PROJECT_ROOT_DIR}/src/view/input.ui
    ${PROJECT_ROOT_DIR}/src/view/inputchange.cpp
    ${PROJECT_ROOT_DIR}/src/view/inputchange.h
    ${PROJECT_ROOT_DIR}/src/view/inputchange.ui
    ${PROJECT_ROOT_DIR}/src/view/progress.cpp
    ${PROJECT_ROOT_DIR}/src/view/progress.h
    ${PROJECT_ROOT_DIR}/src/view/progress.ui
    ${PROJECT_ROOT_DIR}/src/view/tips.cpp
    ${PROJECT_ROOT_DIR}/src/view/tips.h
    ${PROJECT_ROOT_DIR}/src/view/tips.ui
    ${PROJECT_ROOT_DIR}/src/view/tipschange.cpp
    ${PROJECT_ROOT_DIR}/src/view/tipschange.h
    ${PROJECT_ROOT_DIR}/src/view/tipschange.ui
    ${PROJECT_ROOT_DIR}/src/view/tip_burnresult.cpp
    ${PROJECT_ROOT_DIR}/src/view/tip_burnresult.h
    ${PROJECT_ROOT_DIR}/src/view/tip_burnresult.ui
    ${PROJECT_ROOT_DIR}/src/view/busnesssetting.cpp
    ${PROJECT_ROOT_DIR}/src/view/busnesssetting.h
    ${PROJECT_ROOT_DIR}/src/view/busnesssetting.ui
    ${PROJECT_ROOT_DIR}/src/view/settingdatas.cpp
    ${PROJECT_ROOT_DIR}/src/view/settingdatas.h
    ${PROJECT_ROOT_DIR}/src/view/settingdatas.ui
    ${PROJECT_ROOT_DIR}/src/view/filefilterrules.cpp
    ${PROJECT_ROOT_DIR}/src/view/filefilterrules.h
    ${PROJECT_ROOT_DIR}/src/view/filefilterrules.ui
    ${PROJECT_ROOT_DIR}/src/view/viewfilterfiles.cpp
    ${PROJECT_ROOT_DIR}/src/view/viewfilterfiles.h
    ${PROJECT_ROOT_DIR}/src/view/mymenu.cpp
    ${PROJECT_ROOT_DIR}/src/view/mymenu.h
    ${PROJECT_ROOT_DIR}/src/view/multiselectionfiledialog.cpp
    ${PROJECT_ROOT_DIR}/src/view/multiselectionfiledialog.h
    ${PROJECT_ROOT_DIR}/src/view/aboutdialog.cpp
    ${PROJECT_ROOT_DIR}/src/view/aboutdialog.h
    ${PROJECT_ROOT_DIR}/src/view/diskcheck.cpp
    ${PROJECT_ROOT_DIR}/src/view/diskcheck.h
    ${PROJECT_ROOT_DIR}/src/view/diskcheck.ui
    ${PROJECT_ROOT_DIR}/src/view/myframe.cpp
    ${PROJECT_ROOT_DIR}/src/view/myframe.h
    ${PROJECT_ROOT_DIR}/src/view/v11sdk.h
    ${PROJECT_ROOT_DIR}/src/view/splashwindow.h
    ${PROJECT_ROOT_DIR}/src/view/splashwindow.cpp
    ${PROJECT_ROOT_DIR}/src/view/customcombobox.h
    ${PROJECT_ROOT_DIR}/src/view/customcombobox.cpp
    ${PROJECT_ROOT_DIR}/src/model/logicmain.cpp
    ${PROJECT_ROOT_DIR}/src/model/logicmain.h
    ${PROJECT_ROOT_DIR}/src/model/logicburndata.cpp
    ${PROJECT_ROOT_DIR}/src/model/logicburndata.h
    ${PROJECT_ROOT_DIR}/src/model/filefiltermodel.cpp
    ${PROJECT_ROOT_DIR}/src/model/filefiltermodel.h
    ${PROJECT_ROOT_DIR}/src/model/logicburnimage.cpp
    ${PROJECT_ROOT_DIR}/src/model/logicburnimage.h
    ${PROJECT_ROOT_DIR}/src/model/logiccopydisc.cpp
    ${PROJECT_ROOT_DIR}/src/model/logiccopydisc.h
    ${PROJECT_ROOT_DIR}/src/model/menufactory.cpp
    ${PROJECT_ROOT_DIR}/src/model/menufactory.h
    ${PROJECT_ROOT_DIR}/src/model/diroptutils.cpp
    ${PROJECT_ROOT_DIR}/src/model/diroptutils.h
    ${PROJECT_ROOT_DIR}/src/data/datacdrom.cpp
    ${PROJECT_ROOT_DIR}/src/data/datacdrom.h
    ${PROJECT_ROOT_DIR}/src/data/datafile.cpp
    ${PROJECT_ROOT_DIR}/src/data/datafile.h
    ${PROJECT_ROOT_DIR}/src/data/datadirectory.cpp
    ${PROJECT_ROOT_DIR}/src/data/datadirectory.h
    ${PROJECT_ROOT_DIR}/src/data/datasetting.cpp
    ${PROJECT_ROOT_DIR}/src/data/datasetting.h
    ${PROJECT_ROOT_DIR}/src/data/iso9660.cpp
    ${PROJECT_ROOT_DIR}/src/data/iso9660.h
    ${PROJECT_ROOT_DIR}/src/data/iso9660backend.cpp
    ${PROJECT_ROOT_DIR}/src/data/iso9660backend.h
    ${PROJECT_ROOT_DIR}/src/data/newdevicetype.cpp
    ${PROJECT_ROOT_DIR}/src/data/newdevicetype.h
    ${PROJECT_ROOT_DIR}/src/data/msf.cpp
    ${PROJECT_ROOT_DIR}/src/data/msf.h
    ${PROJECT_ROOT_DIR}/src/data/mmc.h
    ${PROJECT_ROOT_DIR}/src/data/toc.cpp
    ${PROJECT_ROOT_DIR}/src/data/toc.h
    ${PROJECT_ROOT_DIR}/src/data/devicetypes.h
    ${PROJECT_ROOT_DIR}/src/data/track.cpp
    ${PROJECT_ROOT_DIR}/src/data/track.h
    ${PROJECT_ROOT_DIR}/src/data/deviceglobals.cpp
    ${PROJECT_ROOT_DIR}/src/data/deviceglobals.h
    ${PROJECT_ROOT_DIR}/src/process/processbasic.cpp
    ${PROJECT_ROOT_DIR}/src/process/processbasic.h
    ${PROJECT_ROOT_DIR}/src/process/processopenfile.cpp
    ${PROJECT_ROOT_DIR}/src/process/processopenfile.h
    ${PROJECT_ROOT_DIR}/src/process/processmd5sum.cpp
    ${PROJECT_ROOT_DIR}/src/process/processmd5sum.h
    ${PROJECT_ROOT_DIR}/src/process/processlsof.cpp
    ${PROJECT_ROOT_DIR}/src/process/processlsof.h
    ${PROJECT_ROOT_DIR}/src/status-fix/discscsi.h
    ${PROJECT_ROOT_DIR}/src/status-fix/discscsi.cpp
    ${PROJECT_ROOT_DIR}/src/status-fix/discread.h
    ${PROJECT_ROOT_DIR}/src/status-fix/discread.cpp
    ${PROJECT_ROOT_DIR}/src/status-fix/discmonitor.h
    ${PROJECT_ROOT_DIR}/src/status-fix/discmonitor.cpp
    ${PROJECT_ROOT_DIR}/src/libisofs/rock.h
    ${PROJECT_ROOT_DIR}/src/libisofs/bswap.h
    ${PROJECT_ROOT_DIR}/src/libisofs/el_torito.h
    ${PROJECT_ROOT_DIR}/src/libisofs/isofs.cpp
    ${PROJECT_ROOT_DIR}/src/libisofs/isofs.h
    ${PROJECT_ROOT_DIR}/src/libisofs/iso_fs.h
    )


# CS0Encode头文件测试
add_executable(cs0encode_test cs0encode_test.cpp)
target_link_libraries(cs0encode_test
    Qt5::Test
    Qt5::Widgets
    ${KYSDKBASE_PKG_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

# ECMA167头文件测试
add_executable(ecma167_test ecma167_test.cpp)
target_link_libraries(ecma167_test
    Qt5::Test
    Qt5::Widgets
    ${KYSDKBASE_PKG_LIBRARIES}
    ${GLIB2_LIBRARIES}
)


# UDFReader头文件测试
add_executable(udfreader_test udfreader_test.cpp)
target_link_libraries(udfreader_test
    Qt5::Test
    Qt5::Widgets
    ${KYSDKBASE_PKG_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

add_executable(datafile_test
    datafile_test.cpp
    ${SOURCES})
target_link_libraries(datafile_test
    ${COMMON_DEPEND_LIB}
    ky-burn-engine
    ky-disc-info
    ky-udf-tools
)


# DeviceTypes头文件测试
add_executable(devicetypes_test devicetypes_test.cpp)
target_link_libraries(devicetypes_test
    Qt5::Test
    Qt5::Widgets
    ${KYSDKBASE_PKG_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

# BurnJobManager头文件测试
# public include
file(GLOB_RECURSE PUBLIC_INCLUDES  CONFIGURE_DEPENDS
    "${PROJECT_ROOT_DIR}/lib/ky-udf-image/include/*")

# src
FILE (GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "${PROJECT_ROOT_DIR}/lib/ky-udf-image/src/*"
)

# public include
file(GLOB_RECURSE ENGINE_INCLUDES  CONFIGURE_DEPENDS
    "${PROJECT_ROOT_DIR}/lib/ky-burn-engine/include/*")

# src
FILE (GLOB_RECURSE ENGINE_SRCS CONFIGURE_DEPENDS
    "${PROJECT_ROOT_DIR}/lib/ky-burn-engine/src/*"
)

# public include
file(GLOB_RECURSE DISC_INCLUDES  CONFIGURE_DEPENDS
    "${PROJECT_ROOT_DIR}/lib/ky-disc-info/include/*")

# src
FILE (GLOB_RECURSE DISC_SRCS CONFIGURE_DEPENDS
    "${PROJECT_ROOT_DIR}/lib/ky-disc-info/src/*"
)

add_executable(burnjobmanager_test
    ${COMMON_TEST_FILE}
    ${PUBLIC_INCLUDES}
    ${SRCS}
    ${ENGINE_INCLUDES}
    ${ENGINE_SRCS}
    burnjobmanager_test.cpp
    )

target_link_libraries(burnjobmanager_test
    xorriso
    ${COMMON_DEPEND_LIB}
)

target_include_directories(burnjobmanager_test PRIVATE
    ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${KYSDKSYSINFO_PKG_INCLUDE_DIRS}
    ${KYSDKAPPLICATION_PKG_INCLUDE_DIRS}
    ${KYSDKBASE_PKG_INCLUDE_DIRS}
    ${KYSDKALM_PKG_INCLUDE_DIRS}
    ${KYSDKSOUNDEFFECTS_PKG_INCLUDE_DIRS}
    ${DATACOLLECT_INCLUDE_DIRS}
    ${KYSDKWAYLANDHELPER_PKG_INCLUDE_DIRS}
    ${PROJECT_ROOT_DIR}/lib/burner-utils-lib/include
    ${PROJECT_ROOT_DIR}/lib/ky-udf-image/include
    ${PROJECT_ROOT_DIR}/lib/ky-burn-engine/include
    ${PROJECT_ROOT_DIR}/lib/ky-disc-info/include
)

add_executable(datacdrom_test
    ${COMMON_TEST_FILE}
    ${PUBLIC_INCLUDES}
    ${SRCS}
    ${ENGINE_INCLUDES}
    ${ENGINE_SRCS}
    ${SOURCES}
    datacdrom_test.cpp
)
target_link_libraries(datacdrom_test
    xorriso
    ${COMMON_DEPEND_LIB}
    ky-disc-info
)

target_include_directories(datacdrom_test PRIVATE
    ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${KYSDKSYSINFO_PKG_INCLUDE_DIRS}
    ${KYSDKAPPLICATION_PKG_INCLUDE_DIRS}
    ${KYSDKBASE_PKG_INCLUDE_DIRS}
    ${KYSDKALM_PKG_INCLUDE_DIRS}
    ${KYSDKSOUNDEFFECTS_PKG_INCLUDE_DIRS}
    ${DATACOLLECT_INCLUDE_DIRS}
    ${KYSDKWAYLANDHELPER_PKG_INCLUDE_DIRS}
    ${PROJECT_ROOT_DIR}/lib/burner-utils-lib/include
    ${PROJECT_ROOT_DIR}/lib/ky-udf-image/include
    ${PROJECT_ROOT_DIR}/lib/ky-burn-engine/include
)

# NewDeviceType类测试
add_executable(newdevicetype_test
    ${COMMON_TEST_FILE}
    ${PUBLIC_INCLUDES}
    ${SRCS}
    ${ENGINE_INCLUDES}
    ${ENGINE_SRCS}
    ${SOURCES}
    newdevicetype_test.cpp
    )
target_link_libraries(newdevicetype_test
    xorriso
    ${COMMON_DEPEND_LIB}
    ky-disc-info
)
target_include_directories(newdevicetype_test PRIVATE
    ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${KYSDKSYSINFO_PKG_INCLUDE_DIRS}
    ${KYSDKAPPLICATION_PKG_INCLUDE_DIRS}
    ${KYSDKBASE_PKG_INCLUDE_DIRS}
    ${KYSDKALM_PKG_INCLUDE_DIRS}
    ${KYSDKSOUNDEFFECTS_PKG_INCLUDE_DIRS}
    ${DATACOLLECT_INCLUDE_DIRS}
    ${KYSDKWAYLANDHELPER_PKG_INCLUDE_DIRS}
    ${PROJECT_ROOT_DIR}/lib/burner-utils-lib/include
    ${PROJECT_ROOT_DIR}/lib/ky-udf-image/include
    ${PROJECT_ROOT_DIR}/lib/ky-burn-engine/include
    ${PROJECT_ROOT_DIR}/lib/ky-disc-info/include
)

# DataSetting类测试
add_executable(datasetting_test
    ${PROJECT_ROOT_DIR}/tests/test_common/test_helpers.h
    ${PROJECT_ROOT_DIR}/tests/test_common/test_helpers.cpp
    ${PROJECT_ROOT_DIR}/src/data/datasetting.h
    ${PROJECT_ROOT_DIR}/src/data/datasetting.cpp
    datasetting_test.cpp
    )
target_link_libraries(datasetting_test
    ${COMMON_DEPEND_LIB}
)

# DataDirectory类测试
add_executable(datadirectory_test
    ${COMMON_TEST_FILE}
    ${PUBLIC_INCLUDES}
    ${SRCS}
    ${ENGINE_INCLUDES}
    ${ENGINE_SRCS}
    ${SOURCES}
    datadirectory_test.cpp
    )
target_link_libraries(datadirectory_test
     xorriso
     ${COMMON_DEPEND_LIB}
     ky-disc-info
)
target_include_directories(datadirectory_test PRIVATE
    ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${KYSDKSYSINFO_PKG_INCLUDE_DIRS}
    ${KYSDKAPPLICATION_PKG_INCLUDE_DIRS}
    ${KYSDKBASE_PKG_INCLUDE_DIRS}
    ${KYSDKALM_PKG_INCLUDE_DIRS}
    ${KYSDKSOUNDEFFECTS_PKG_INCLUDE_DIRS}
    ${DATACOLLECT_INCLUDE_DIRS}
    ${KYSDKWAYLANDHELPER_PKG_INCLUDE_DIRS}
    ${PROJECT_ROOT_DIR}/lib/burner-utils-lib/include
    ${PROJECT_ROOT_DIR}/lib/ky-udf-image/include
    ${PROJECT_ROOT_DIR}/lib/ky-burn-engine/include
    ${PROJECT_ROOT_DIR}/lib/ky-disc-info/include
)

# DeviceGlobals类测试
add_executable(deviceglobals_test deviceglobals_test.cpp ${PROJECT_ROOT_DIR}/src/data/deviceglobals.cpp)
target_link_libraries(deviceglobals_test
     ${COMMON_DEPEND_LIB}
)


# 添加CTest测试
add_test(NAME datacdrom_test COMMAND datacdrom_test)
add_test(NAME datadirectory_test COMMAND datadirectory_test)
add_test(NAME datafile_test COMMAND datafile_test)
add_test(NAME device_test COMMAND device_test)
add_test(NAME devicetypes_test COMMAND devicetypes_test)
add_test(NAME iso9660backend_test COMMAND iso9660backend_test)
add_test(NAME newdevicetype_test COMMAND newdevicetype_test)
add_test(NAME toc_test COMMAND toc_test)
add_test(NAME mmc_test COMMAND mmc_test)
add_test(NAME xorriso_test COMMAND xorriso_test)
add_test(NAME burnjob_test COMMAND burnjob_test)
add_test(NAME burnjobmanager_test COMMAND burnjobmanager_test)
add_test(NAME dburn_global_test COMMAND dburn_global_test)
add_test(NAME dopticaldiscinfo_test COMMAND dopticaldiscinfo_test)
add_test(NAME dopticaldiscmanager_test COMMAND dopticaldiscmanager_test)
add_test(NAME blockinput_test COMMAND blockinput_test)
add_test(NAME cs0encode_test COMMAND cs0encode_test)
add_test(NAME ecma167_test COMMAND ecma167_test)
add_test(NAME interface_test COMMAND interface_test)
add_test(NAME udfappender_test COMMAND udfappender_test)
add_test(NAME udfread_test COMMAND udfread_test)
add_test(NAME udfreader_test COMMAND udfreader_test)

