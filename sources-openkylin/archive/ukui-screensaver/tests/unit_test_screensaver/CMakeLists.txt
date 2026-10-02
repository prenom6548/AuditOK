# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

find_package(Qt5 COMPONENTS Core DBus Network Test REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)

# 包含 GTest 库和 pthread 库
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(GIOUNIX2 REQUIRED gio-unix-2.0)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)
pkg_check_modules(QGS REQUIRED gsettings-qt)
pkg_check_modules(LIGHTDM-QT5-3 REQUIRED liblightdm-qt5-3)
pkg_check_modules(LIBSYSTEMD REQUIRED libsystemd)
# 查找pam动态库全路径并缓存到PAM_LIBRARIES变量
find_library(PAM_LIBRARIES pam)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

 
# 开启代码覆盖率相关编译选项（对应QMAKE_LFLAGS和QMAKE_CXXFLAGS中代码覆盖率相关设置）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage --coverage -fno-inline -fno-access-control")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

# 定义源文件列表，对应原来的SOURCES变量
set(SOURCES
    ../../src/lock-backend/authpamthread.cpp
    ../../src/lock-backend/pamauthenticate.cpp
    ../../src/lock-backend/lightdmhelper.cpp
    ../../src/lock-backend/dbusupperinterface.cpp
    ../../src/lock-backend/gsettingshelper.cpp
    ../../src/lock-backend/bioauthenticate.cpp
    ../../src/lock-backend/switchuserutils.cpp
    ../../src/dbusifs/accountshelper.cpp
    ../../src/userinfo.cpp
    ../../src/common/global_utils.cpp
    ../../src/common/configuration.cpp
    ../../src/common/kyrsac.cpp
    ../../src/dbusifs/giodbus.cpp
    ../../src/dbusifs/uniauthservice.cpp
    ../../src/lock-backend/securityuser.cpp
    ../../src/dbusifs/freedesktophelper.cpp
    ../../src/dbusifs/login1helper.cpp
    ../../src/dbusifs/usdhelper.cpp
    ../../src/dbusifs/upowerhelper.cpp
    ../../src/dbusifs/sessionhelper.cpp
    ../../src/dbusifs/dbusservermanager.cpp
    ../../src/dbusifs/systemupgradehelper.cpp
    ../../src/lock-backend/sessionwatcher.cpp
    ../../src/dbusifs/kglobalaccelhelper.cpp
    ../../src/dbusifs/libinputswitchevent.cpp
    ../../src/lock-backend/personalizeddata.cpp
    ../../src/dbusifs/biometrichelper.cpp
    ../../src/dbusifs/device.cpp
    ../../src/dbusifs/machinemodel.cpp
    ../../src/dbusifs/enginedevice.cpp
    ../../src/common/biodefines.cpp
    ../../src/QtSingleApplication/qtlocalpeer.cpp
    ../kt-test-utils/cpp-stub-ext/stub-shadow.cpp
    ../../src/ukccplugins/sessiondbus/screensaverinterface.cpp
    unit_test_screensaver.cpp
    main.cpp
)

# 定义头文件列表，对应原来的HEADERS变量
set(HEADERS
    ../../src/lock-backend/authpamthread.h
    ../../src/lock-backend/pamauthenticate.h
    ../../src/lock-backend/lightdmhelper.h
    ../../src/lock-backend/dbusupperinterface.h
    ../../src/lock-backend/gsettingshelper.h
    ../../src/lock-backend/bioauthenticate.h
    ../../src/lock-backend/switchuserutils.h
    ../../src/common/definetypes.h
    ../../src/userinfo.h
    ../../src/common/global_utils.h
    ../../src/common/configuration.h
    ../../src/common/kyrsac.h
    ../../src/dbusifs/giodbus.h
    ../../src/dbusifs/accountshelper.h
    ../../src/common/configuration.h
    ../../src/dbusifs/uniauthservice.h
    ../../src/lock-backend/securityuser.h
    ../../src/dbusifs/freedesktophelper.h
    ../../src/dbusifs/login1helper.h
    ../../src/dbusifs/usdhelper.h
    ../../src/dbusifs/upowerhelper.h
    ../../src/dbusifs/dbusservermanager.h
    ../../src/dbusifs/sessionhelper.h
    ../../src/dbusifs/systemupgradehelper.h
    ../../src/lock-backend/sessionwatcher.h
    ../../src/dbusifs/kglobalaccelhelper.h
    ../../src/dbusifs/libinputswitchevent.h
    ../../src/lock-backend/personalizeddata.h
    ../../src/dbusifs/biometrichelper.h
    ../../src/dbusifs/device.h
    ../../src/dbusifs/enginedevice.h
    ../../src/dbusifs/machinemodel.h
    ../../src/common/biodefines.h
    ../kt-test-utils/cpp-stub-ext/stubext.h
    ../kt-test-utils/cpp-stub-ext/stub-shadow.h
    ../../src/QtSingleApplication/qtlocalpeer.h
    ../../src/ukccplugins/sessiondbus/screensaverinterface.h
)

# 包含头文件的路径设置，对应原来的INCLUDEPATH变量
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../kt-test-utils/cpp-stub
    ${CMAKE_CURRENT_SOURCE_DIR}/../kt-test-utils/cpp-stub-ext
    ${QGS_INCLUDE_DIRS}
    ${Qt5Test_INCLUDE_DIRS}
    ${LIGHTDM-QT5-3_INCLUDE_DIRS}
    ${LIBSYSTEMD_INCLUDE_DIRS}
    ${GIOUNIX2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}../../src/QtSingleApplication/
)

# 使用qt5_wrap_cpp生成元对象代码相关的源文件
qt5_wrap_cpp(MOC_SOURCES ${HEADERS})

# 添加可执行文件或库目标，将元对象代码源文件一起添加进去
add_executable(unit_test_screensaver ${SOURCES} ${MOC_SOURCES})

# 链接Qt相关的库
target_link_libraries(unit_test_screensaver
    Qt5::Core
    Qt5::DBus
    Qt5::Test
    Qt5::Network
    ${QGS_LIBRARIES}
    ${PAM_LIBRARIES}
    ${LIGHTDM-QT5-3_LIBRARIES}
    ${LIBSYSTEMD_LIBRARIES}
    OpenSSL::Crypto
    -lukuiinputgatherclient
    ${GIOUNIX2_LIBRARIES}
)

# 链接 GTest 库
target_link_libraries(unit_test_screensaver
    GTest::GTest
    GTest::Main
    Threads::Threads
)

