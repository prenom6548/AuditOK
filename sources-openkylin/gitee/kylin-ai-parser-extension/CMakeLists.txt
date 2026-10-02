cmake_minimum_required(VERSION 3.12)  # 建议使用 3.12 或更高版本，以获得更好的 Python 模块支持

project(parser_extension LANGUAGES CXX C)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Python 3.x 解释器和开发库
find_package(Python3 REQUIRED COMPONENTS Development Interpreter)
if (NOT Python3_FOUND)
    message(FATAL_ERROR "Python 3.x or higher is required but not found.")
endif()
message(STATUS "Found Python: ${Python3_VERSION} (${Python3_EXECUTABLE})")

set(PYBIND11_PYTHON_VERSION ${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR} CACHE STRING "")
# 查找 pybind11
find_package(pybind11 REQUIRED)
# 覆盖 pybind11 的默认包含路径
set_property(TARGET pybind11::module PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${Python3_INCLUDE_DIRS}
)
message(STATUS "pybind11 INCLUDE_DIRS: ${pybind11_INCLUDE_DIRS}")
message(STATUS "pybind11 LIBRARIES: ${pybind11_LIBRARIES}")

# 查找 Qt5Core
find_package(Qt5Core REQUIRED)

# 查找 ukui-file-metadata
find_package(PkgConfig REQUIRED)
pkg_check_modules(filemetadata REQUIRED IMPORTED_TARGET ukui-file-metadata)

# 添加模块源文件
add_library(parser_extension MODULE
    parserextension.cpp
)

# 包含目录
target_include_directories(parser_extension PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${filemetadata_INCLUDE_DIRS}
    ${pybind11_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}  # 确保包含 Python 3.x 的头文件路径
)

# 链接库
target_link_libraries(parser_extension PRIVATE
    PkgConfig::filemetadata
    pybind11::module
    Qt5::Core
    ${Python3_LIBRARIES}  # 确保链接 Python 3.x 的库
)

# 设置输出路径
set_target_properties(parser_extension PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 设置输出文件名
set_target_properties(parser_extension PROPERTIES
    OUTPUT_NAME "parser_extension"
    SUFFIX ".so"
)

# 安装到架构特定的库路径
install(TARGETS parser_extension DESTINATION ${CMAKE_INSTALL_LIBDIR}/kylin-ai-parser-extention/)