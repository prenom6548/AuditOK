cmake_minimum_required(VERSION 3.16)

project(kylin-iputils-arping LANGUAGES CXX)

set(NETWORK_IPUTILS_ARPING_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)

add_executable(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)

find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

find_package(Qt5 REQUIRED COMPONENTS Sql)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Sql)

target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

find_package(PkgConfig)
pkg_check_modules(KYSDK_MODULE kysdk-alm kysdk-kabase kysdk-qtwidgets kysdk-diagnostics)
if (${KYSDK_MODULE_FOUND})
    target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARIES})
else()
    message("========================= Use Compatibility: ${PROJECT_NAME} =========================")
    set(COMPATIBILITY_NAME compatibility)
    set(COMPATIBILITY_DIR ${TOP_DIR}/${COMPATIBILITY_NAME})
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-alm/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-kabase/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-qtwidgets/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-diagnostics/)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_NAME})
endif()

set(SRCS
	${NETWORK_IPUTILS_ARPING_DIR}/main.cpp
	${NETWORK_IPUTILS_ARPING_DIR}/mainwindow.cpp)

set(TRANSLATIONS
	${NETWORK_IPUTILS_ARPING_DIR}/translations/komt-network-iputils-arping_zh_CN.ts
	${NETWORK_IPUTILS_ARPING_DIR}/translations/komt-network-iputils-arping_bo_CN.ts
	${NETWORK_IPUTILS_ARPING_DIR}/translations/komt-network-iputils-arping_mn.ts
    ${NETWORK_IPUTILS_ARPING_DIR}/translations/komt-network-iputils-arping_zh_HK.ts)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

target_include_directories(${PROJECT_NAME} PRIVATE ${NETWORK_IPUTILS_ARPING_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kylin-os-manager/bin/)
install(FILES ${QM_FILES} DESTINATION /usr/share/kylin-os-manager/treasure-chest/kylin-iputils-arping/translations/)
install(FILES kylin-iputils-arping.conf DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/plugins/)
