# Model模块测试的CMake配置
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

# 包含目录
include_directories(
    ${PROJECT_ROOT_DIR}/src
    ${PROJECT_ROOT_DIR}/src/common
    ${PROJECT_ROOT_DIR}/src/data
    ${PROJECT_ROOT_DIR}/src/frame
    ${PROJECT_ROOT_DIR}/src/model
    ${PROJECT_ROOT_DIR}/src/process
    ${PROJECT_ROOT_DIR}/src/status-fix
    ${PROJECT_ROOT_DIR}/lib
    ${Qt5Test_INCLUDE_DIRS}
    ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${KYSDKSYSINFO_PKG_INCLUDE_DIRS}
    ${KYSDKBASE_PKG_INCLUDE_DIRS}
)

# 链接目录
link_directories(
    ${PROJECT_ROOT_DIR}/build/src
    ${PROJECT_ROOT_DIR}/build/lib/ky-burn-engine
    ${PROJECT_ROOT_DIR}/build/lib/ky-disc-info
    ${PROJECT_ROOT_DIR}/build/lib/ky-udf-image
    ${KYSDKQTWIDGETS_PKG_LIBRARY_DIRS}
    ${KYSDKSYSINFO_PKG_LIBRARY_DIRS}
    ${KYSDKBASE_PKG_LIBRARY_DIRS}
)

# 统一测试：FileFilterModel + LogicBurnImage + LogicCopyDisc + LogicMain（一个可执行文件运行四个测试）
add_executable(model_unified_test
    ${SOURCES}
    ${COMMON_TEST_FILE}
    model_unified_test.cpp
    )
# 为被 model_unified_test.cpp #include 进来的测试 .cpp 生成 .moc 文件
set(MODEL_UNIFIED_INCLUDED_TESTS filefiltermodel_test logicburnimage_test logiccopydisc_test logicmain_test menufactory_test model_test)
set(MODEL_MOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/model_unified_included_moc)
file(MAKE_DIRECTORY ${MODEL_MOC_OUT_DIR})
set(MODEL_MOC_OUTPUTS "")
foreach(_t ${MODEL_UNIFIED_INCLUDED_TESTS})
    add_custom_command(OUTPUT ${MODEL_MOC_OUT_DIR}/${_t}.moc
        COMMAND ${Qt5Core_MOC_EXECUTABLE}
            -I${PROJECT_ROOT_DIR}
            -I${PROJECT_ROOT_DIR}/tests
            -I${PROJECT_ROOT_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/${_t}.cpp
            -o ${MODEL_MOC_OUT_DIR}/${_t}.moc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_t}.cpp
        COMMENT "Generating ${_t}.moc for model_unified_test"
    )
    list(APPEND MODEL_MOC_OUTPUTS ${MODEL_MOC_OUT_DIR}/${_t}.moc)
endforeach()
add_custom_target(model_unified_test_moc ALL DEPENDS ${MODEL_MOC_OUTPUTS})
add_dependencies(model_unified_test model_unified_test_moc)
target_include_directories(model_unified_test PRIVATE ${MODEL_MOC_OUT_DIR})
set_source_files_properties(model_unified_test.cpp PROPERTIES OBJECT_DEPENDS "${MODEL_MOC_OUTPUTS}")
target_link_libraries(model_unified_test
    ${COMMON_DEPEND_LIB}
    ky-burn-engine
    ky-disc-info
    ky-udf-tools
)

add_test(NAME model_test COMMAND model_test)
add_test(NAME menufactory_test COMMAND menufactory_test)
add_test(NAME model_unified_test COMMAND model_unified_test)
