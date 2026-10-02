cmake_minimum_required(VERSION 3.16)
project(kylin-ki18n-cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

option(ENABLE_COVERAGE "Enable gcov coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -fPIC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

# 启用 Qt 自动 moc，生成元对象代码
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Qml)

# 设置安装路径 - 使用 GNUInstallDirs 提供的变量
# CMAKE_INSTALL_LIBDIR 默认会是 lib/aarch64-linux-gnu (通过 GNUInstallDirs)
message(STATUS "CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "CMAKE_INSTALL_FULL_LIBDIR: ${CMAKE_INSTALL_FULL_LIBDIR}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(SOVERSION 5)
set(VERSION 5.92.0)

# 头文件目录（供 moc 使用）
set(HEADERS
    ${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/klocalizedcontext.h
    ${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/klocalizedtranslator.h
)

set(SOURCES
    klocalizedstring.cpp
    kcatalog.cpp
    kuitsetup.cpp
    kuit_default_markup.cpp
    klocalizedcontext.cpp
    klocalizedtranslator.cpp
    kuitmarkup.cpp
    main.cpp
)

# 头文件必须列入目标，AUTOMOC 才会为 Q_OBJECT（KLocalizedContext / KLocalizedTranslator）生成 moc，
# 否则 vtable 符号缺失，与上游链接的测试二进制无法运行时加载本库。
add_library(KF5I18n SHARED ${SOURCES} ${HEADERS})

set_target_properties(KF5I18n PROPERTIES
    OUTPUT_NAME KF5I18n
    VERSION ${VERSION}
    SOVERSION ${SOVERSION}
    EXPORT_NAME I18n
)

# 默认隐藏非 API 符号
target_compile_options(KF5I18n PRIVATE
    -fvisibility=hidden
    -fvisibility-inlines-hidden
)

# Qt5::Core 对依赖方可见（与上游 KF5I18n 一致）；Qml 仅本库 / 插件内部使用
target_link_libraries(KF5I18n PUBLIC Qt5::Core PRIVATE
    Qt5::Qml
    ${CMAKE_SOURCE_DIR}/target/release/libki18n_rs.a
)

# 丢弃未引用 section；尽量不把 Rust 静态库中的符号全部导出到 DSO（仍会有 ki18n_rs_* / boa 等，见 nm -D）。
target_link_options(KF5I18n PRIVATE
    "LINKER:--gc-sections"
    "LINKER:--exclude-libs,libki18n_rs.a"
)

target_include_directories(KF5I18n PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp/include
    ${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString
)
# 与上游 ki18n/src/i18n/CMakeLists.txt 对齐：向消费者传播已安装头文件路径
target_include_directories(KF5I18n
    INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/KF5/KI18n>"
)

# 安装规则见根目录 cmake/kf5_install_targets.cmake

# 与上游一致：独立 Qt 插件，供 KLocalizedString 通过 QPluginLoader 加载
add_library(ktranscript MODULE ktranscript_plugin.cpp)
set_target_properties(ktranscript PROPERTIES PREFIX "")
target_compile_options(ktranscript PRIVATE
    -fvisibility=hidden
    -fvisibility-inlines-hidden
)
target_link_libraries(ktranscript PRIVATE
    Qt5::Core
    Qt5::Qml
    ${CMAKE_SOURCE_DIR}/target/release/libki18n_rs.a
)
target_link_options(ktranscript PRIVATE
    "LINKER:--gc-sections"
    "LINKER:--exclude-libs,libki18n_rs.a"
)
target_include_directories(ktranscript PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp/include
    ${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString
)
add_dependencies(KF5I18n ki18n_rs_build)
add_dependencies(ktranscript ki18n_rs_build)
