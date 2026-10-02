set(TEST_NAME tst_ukui-notebook)

# 找到所有需要的 Qt 组件
find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Widgets
    Test
    Network
    DBus
    Sql
    Concurrent
    REQUIRED
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 添加所有需要的源文件，包括字体相关组件
set(TEST_SOURCES
    tst_main.cpp
    tst_notemodel.cpp
    tst_dbmanager.cpp
    tst_notedata.cpp
    tst_adaptscreeninfo.cpp
    tst_editpage.cpp
    tst_iconviewmodedelegate.cpp
    tst_listviewmodedelegate.cpp
    tst_widget.cpp
)

set(MAIN_SOURCES
    ${CMAKE_SOURCE_DIR}/src/dbManager.cpp
    ${CMAKE_SOURCE_DIR}/src/noteModel.cpp
    ${CMAKE_SOURCE_DIR}/src/noteData.cpp
    ${CMAKE_SOURCE_DIR}/src/editPage.cpp
    ${CMAKE_SOURCE_DIR}/src/noteHeadMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/setFontSizePage.cpp
    ${CMAKE_SOURCE_DIR}/src/adaptscreeninfo.cpp
    ${CMAKE_SOURCE_DIR}/src/editPage.cpp
    ${CMAKE_SOURCE_DIR}/src/widget.cpp


    # 自定义部件
    ${CMAKE_SOURCE_DIR}/src/custom_ui/common_ui.cpp
    ${CMAKE_SOURCE_DIR}/src/custom_ui/custom_color_panel.cpp
    ${CMAKE_SOURCE_DIR}/src/custom_ui/custom_push_button_group.cpp
    ${CMAKE_SOURCE_DIR}/src/custom_ui/custom_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/custom_ui/resizable_push_button.cpp
    ${CMAKE_SOURCE_DIR}/src/custom_ui/resizable_tool_button.cpp
    # 字体相关组件
    ${CMAKE_SOURCE_DIR}/src/font_button/font_color_push_button.cpp
    ${CMAKE_SOURCE_DIR}/src/font_button/font_property_push_button.cpp
    ${CMAKE_SOURCE_DIR}/src/font_button/font_property_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/font_button/font_size_push_button.cpp
    ${CMAKE_SOURCE_DIR}/src/font_button/font_twin_button_group.cpp

    ${CMAKE_SOURCE_DIR}/src/noteExitWindow.cpp
    ${CMAKE_SOURCE_DIR}/src/emptyNotes.cpp
    ${CMAKE_SOURCE_DIR}/src/listViewModeDelegate.cpp
    ${CMAKE_SOURCE_DIR}/src/iconViewModeDelegate.cpp
    ${CMAKE_SOURCE_DIR}/src/noteView.cpp
    ${CMAKE_SOURCE_DIR}/src/about.cpp
    ${CMAKE_SOURCE_DIR}/src/paletteButton.cpp
    ${CMAKE_SOURCE_DIR}/src/noteHeadMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/noteHead.cpp
    ${CMAKE_SOURCE_DIR}/src/selectColorPage.cpp
    ${CMAKE_SOURCE_DIR}/src/setFontColorPage.cpp
    ${CMAKE_SOURCE_DIR}/src/mymenu.cpp
    ${CMAKE_SOURCE_DIR}/src/paletteWidget.cpp
)

# UI 文件
set(UI_FILES
    ${CMAKE_SOURCE_DIR}/src/noteHeadMenu.ui
    ${CMAKE_SOURCE_DIR}/src/editPage.ui
    ${CMAKE_SOURCE_DIR}/src/noteExitWindow.ui
    ${CMAKE_SOURCE_DIR}/src/setFontSizePage.ui
    ${CMAKE_SOURCE_DIR}/src/font_button/font_property_widget.ui  # 添加字体属性窗口的UI
)

# 其余部分保持不变
add_executable(${TEST_NAME}
    ${TEST_SOURCES}
    ${UI_FILES}
    ${MAIN_SOURCES}
)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})

target_include_directories(${TEST_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/src/custom_ui
    ${CMAKE_SOURCE_DIR}/src/font_button
    ${common_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TEST_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Concurrent
    common
    -lpthread
    -lz
)

target_link_kysdk_libraries(${TEST_NAME})
target_link_KWindowSystem_libraries(${TEST_NAME})
target_link_qgsettings_libraries(${TEST_NAME})


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

# 添加代码覆盖率选项（仅限非 Release 模式）
if(NOT CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "启用代码覆盖率功能")
    add_compile_options(--coverage)
    link_libraries(--coverage)
endif()

## 添加测试
#add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

# 设置 lcov 和覆盖率报告（可选）
find_program(LCOV_EXECUTABLE lcov)
find_program(GENHTML_EXECUTABLE genhtml)

if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
    add_custom_target(coverage
        COMMAND ${LCOV_EXECUTABLE} --capture --directory . --output-file coverage.info
        COMMAND ${LCOV_EXECUTABLE} --remove coverage.info '/usr/*' --output-file coverage.info
        COMMAND ${GENHTML_EXECUTABLE} coverage.info --output-directory coverage_report
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "生成覆盖率报告"
    )
endif()
