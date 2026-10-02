set(CMAKE_AUTOMOC on)
set(CMAKE_AUTORCC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Test Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Widgets)

include_directories(
    ..
    ../../libsearch
    ../../libsearch/plugininterface
    ../../libsearch/pluginmanage
    ../control
    ../control/flow-layout
    ../control/list-labels
    ../control/stack-pages
    ../model
    ../view
)

add_executable(searchLineEditTest
    search-line-edit-test.cpp
    ../control/search-line-edit.cpp
)
target_link_libraries(searchLineEditTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ukui-search
)
add_test(searchLineEditTest ${CMAKE_BINARY_DIR}/frontend/autotest/searchLineEditTest)

add_executable(createIndexAskDialogTest
    create-index-ask-dialog-test.cpp
    ../control/create-index-ask-dialog.cpp
)
target_link_libraries(createIndexAskDialogTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ukui-search
    ukui-quick::platform
)
add_test(createIndexAskDialogTest ${CMAKE_BINARY_DIR}/frontend/autotest/createIndexAskDialogTest)

add_executable(oneResultTest
    one-result-test.cpp
    ../model/one-result.cpp
)
target_link_libraries(oneResultTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ukui-search
)
add_test(oneResultTest ${CMAKE_BINARY_DIR}/frontend/autotest/oneResultTest)

add_executable(resultViewTest
    result-view-test.cpp
    ../view/result-view.cpp
    ../view/result-view-delegate.cpp
    ../model/search-result-model.cpp
    ../model/search-result-manager.cpp
    ../model/one-result.cpp
    ../control/list-labels/title-label.cpp
    ../control/list-labels/show-more-label.cpp
    ../resource.qrc
)
target_link_libraries(resultViewTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ukui-search
)
add_test(resultViewTest ${CMAKE_BINARY_DIR}/frontend/autotest/resultViewTest)

add_executable(resultWidgetWhiteboxTest
    result-widget-whitebox-test.cpp
    ../../libsearch/pluginmanage/search-plugin-manager.h
    ../../libsearch/global-settings.h
    ../view/result-view.cpp
    ../view/result-view-delegate.cpp
    ../model/search-result-model.cpp
    ../model/search-result-manager.cpp
    ../model/one-result.cpp
    ../control/list-labels/title-label.cpp
    ../control/list-labels/show-more-label.cpp
    ../resource.qrc
)
target_link_libraries(resultWidgetWhiteboxTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(resultWidgetWhiteboxTest ${CMAKE_BINARY_DIR}/frontend/autotest/resultWidgetWhiteboxTest)

add_executable(bestListViewWhiteboxTest
    best-list-view-whitebox-test.cpp
    ../../libsearch/pluginmanage/search-plugin-manager.h
    ../../libsearch/global-settings.h
    ../view/best-list-view.cpp
    ../view/result-view-delegate.cpp
    ../model/best-list-model.cpp
    ../model/one-result.cpp
    ../control/list-labels/title-label.cpp
    ../control/list-labels/show-more-label.cpp
    ../resource.qrc
)
target_link_libraries(bestListViewWhiteboxTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(bestListViewWhiteboxTest ${CMAKE_BINARY_DIR}/frontend/autotest/bestListViewWhiteboxTest)

add_executable(stackPagesWhiteboxTest
    stack-pages-whitebox-test.cpp
    ../../libsearch/pluginmanage/search-plugin-manager.h
    ../../libsearch/global-settings.h
    ../control/stack-pages/search-page-section.cpp
    ../control/stack-pages/search-result-page.cpp
    ../view/result-view.cpp
    ../view/result-view-delegate.cpp
    ../view/best-list-view.cpp
    ../model/search-result-model.cpp
    ../model/search-result-manager.cpp
    ../model/best-list-model.cpp
    ../model/one-result.cpp
    ../control/list-labels/title-label.cpp
    ../control/list-labels/show-more-label.cpp
    ../resource.qrc
)
target_link_libraries(stackPagesWhiteboxTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(stackPagesWhiteboxTest ${CMAKE_BINARY_DIR}/frontend/autotest/stackPagesWhiteboxTest)

add_executable(searchResultModelTest
    search-result-model-test.cpp
    ../model/search-result-model.cpp
    ../model/search-result-manager.cpp
    ../model/one-result.cpp
)
target_link_libraries(searchResultModelTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ukui-search
)
add_test(searchResultModelTest ${CMAKE_BINARY_DIR}/frontend/autotest/searchResultModelTest)

add_executable(searchResultManagerTest
    search-result-manager-test.cpp
    ../model/search-result-manager.cpp
)
target_link_libraries(searchResultManagerTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ukui-search
)
add_test(searchResultManagerTest ${CMAKE_BINARY_DIR}/frontend/autotest/searchResultManagerTest)

add_executable(bestListModelTest
    best-list-model-test.cpp
    ../model/best-list-model.cpp
    ../model/one-result.cpp
)
target_link_libraries(bestListModelTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ukui-search
)
add_test(bestListModelTest ${CMAKE_BINARY_DIR}/frontend/autotest/bestListModelTest)

add_executable(flowLayoutTest
    flow-layout-test.cpp
    ../control/flow-layout/flow-layout.cpp
)
target_link_libraries(flowLayoutTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(flowLayoutTest ${CMAKE_BINARY_DIR}/frontend/autotest/flowLayoutTest)

add_executable(showMoreLabelTest
    show-more-label-test.cpp
    ../control/list-labels/show-more-label.cpp
    ../resource.qrc
)
target_link_libraries(showMoreLabelTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ukui-search
)
add_test(showMoreLabelTest ${CMAKE_BINARY_DIR}/frontend/autotest/showMoreLabelTest)

add_executable(titleLabelTest
    title-label-test.cpp
    ../control/list-labels/title-label.cpp
    ../control/list-labels/show-more-label.cpp
    ../resource.qrc
)
target_link_libraries(titleLabelTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ukui-search
)
add_test(titleLabelTest ${CMAKE_BINARY_DIR}/frontend/autotest/titleLabelTest)
