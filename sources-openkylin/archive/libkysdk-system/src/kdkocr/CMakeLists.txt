cmake_minimum_required(VERSION 3.16)
project(kyocr)

set(CMAKE_CXX_STANDARD 11)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

pkg_check_modules(TESSERACT_PKG tesseract)
pkg_check_modules(LEPT_PKG lept)
link_directories(${TESSERACT_PKG_LIBRARY_DIRS} ${LEPT_PKG_LIBRARY_DIRS})
include_directories(${TESSERACT_PKG_INCLUDE_DIRS} ${LEPT_PKG_INCLUDE_DIRS}) 
message("kyocr_cflags is ${TESSERACT_PKG_LIBRARIES}")
message("lept libs: ${LEPT_PKG_LIBRARIES}, lept lib dirs: ${LEPT_PKG_LIBRARY_DIRS}")
list(APPEND kyocr_libraries ${TESSERACT_PKG_LIBRARIES} ${LEPT_PKG_LIBRARIES})
message("kyocr_cflags is ${kyocr_libraries}")

# 统一使用Tesseract OCR，移除所有Paddle相关代码
add_library(kyocr SHARED libkyocr.cpp)
target_link_libraries(kyocr ${kyocr_libraries})
target_compile_options(kyocr PUBLIC ${kyocr_cflags})
target_link_options(kyocr PRIVATE "-Wl,-z,now")

set_target_properties(kyocr PROPERTIES VERSION 3.0.0 SOVERSION 1)

# 包含 GNU 安装目录设置  
include(GNUInstallDirs)

# 设置 CMAKE_INSTALL_LIBDIR 变量  
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv8l")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/arm-linux-gnueabihf")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/mips64el-linux-gnuabi64")
    elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/i386-linux-gnu")
else()
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu")
endif()

# 安装库文件  
install(TARGETS kyocr
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${PROJECT_SOURCE_DIR}/libkyocr.hpp DESTINATION include/kysdk/kysdk-system)
