cmake_minimum_required(VERSION 3.5)

project(kylin-ai-business-framework-service LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kylin-ai-business-framework-service src/main.cpp
    src/thirdparty/threadpool/async.cpp src/thirdparty/threadpool/async.h
    src/thirdparty/threadpool/singleton.h
    src/thirdparty/threadpool/threadpool.cpp src/thirdparty/threadpool/threadpool.h
    src/server.cpp src/server.h
    src/servicemanager.cpp src/servicemanager.h
    src/datamanagement/datamanagementprocessorglue.c src/datamanagement/datamanagementprocessorglue.h
    src/datamanagement/datamanagementprocessor.cpp src/datamanagement/datamanagementprocessor.h
    src/utils/python/autotokenizer.py
    src/utils/parser/parser.h src/utils/parser/parser.cpp
    src/utils/parser/txtparser.h src/utils/parser/txtparser.cpp
    src/utils/parser/pdfparser.h src/utils/parser/pdfparser.cpp
    src/utils/pythonutil.h src/utils/pythonutil.cpp
    src/utils/parser/officepyparserwrapper.h src/utils/parser/officepyparserwrapper.cpp
    src/utils/python/officeparser.py
    src/utils/parser/docxparser.h src/utils/parser/docxparser.cpp
    src/utils/python/docxparser.py
    src/utils/parser/pptxparser.h src/utils/parser/pptxparser.cpp
    src/utils/python/pptxparser.py
    src/utils/parser/fileparserfactory.cpp src/utils/parser/fileparserfactory.h
    src/utils/autotokenizer.cpp
    src/utils/autotokenizer.h
    src/utils/imageembedder.cpp
    src/utils/imageembedder.h
    src/utils/textembedder.cpp
    src/utils/textembedder.h
    src/embeddingtaskmanager/baseembeddingservice.h src/embeddingtaskmanager/baseembeddingservice.cpp
    src/embeddingtaskmanager/embeddingtask.h src/embeddingtaskmanager/embeddingtask.cpp
    src/embeddingtaskmanager/embeddingtaskmanager.h src/embeddingtaskmanager/embeddingtaskmanager.cpp
    src/embeddingtaskmanager/imageembeddingservice.h src/embeddingtaskmanager/imageembeddingservice.cpp
    src/embeddingtaskmanager/textembeddingservice.h src/embeddingtaskmanager/textembeddingservice.cpp
    src/datamanagement/datamanagementjsonhelper.cpp
    src/datamanagement/datamanagementjsonhelper.h
    src/datamanagement/datamanagementdatabase.cpp
    src/datamanagement/datamanagementdatabase.h
    src/datamanagement/datamanagementservice.cpp
    src/datamanagement/datamanagementservice.h
    src/utils/vectordb/vectordb.cpp
    src/utils/vectordb/vectordb.h
    src/utils/utils.cpp
    src/utils/utils.h
)


find_package(PkgConfig REQUIRED)
find_package(Python 3.8 COMPONENTS Development REQUIRED)

pkg_check_modules(poppler REQUIRED IMPORTED_TARGET poppler-cpp)
pkg_check_modules(opencv REQUIRED IMPORTED_TARGET opencv4)
pkg_check_modules(onnxruntime REQUIRED IMPORTED_TARGET libonnxruntime)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-unix-2.0)
pkg_check_modules(jsoncpp REQUIRED IMPORTED_TARGET jsoncpp)
pkg_check_modules(UUID REQUIRED IMPORTED_TARGET uuid)

include_directories(
    ./src
    ${GIO_INCLUDE_DIRS}
    ${Python_INCLUDE_DIRS}
)

target_link_libraries(kylin-ai-business-framework-service
    ${Python_LIBRARIES}
    PkgConfig::GIO
    pthread
    PkgConfig::jsoncpp
    PkgConfig::poppler
    PkgConfig::opencv
    PkgConfig::onnxruntime
    PkgConfig::UUID
)

set(FRAMEWORK_PATH /usr/share/kylin-ai-business-framework)

include(GNUInstallDirs)
install(TARGETS kylin-ai-business-framework-service
    RUNTIME DESTINATION /usr/bin
)
install(DIRECTORY src/utils/python DESTINATION ${FRAMEWORK_PATH}/utils)
install(DIRECTORY configs/lib/systemd/user/ DESTINATION /usr/lib/systemd/user)
install(DIRECTORY python-depends/ DESTINATION ${FRAMEWORK_PATH}/python-depends)
