cmake_minimum_required(VERSION 3.5)

project(kylin-coreai-vision-test LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

find_package(KylinAiProto REQUIRED)
kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES corevisionservice)

set(LIBRARIES
    pthread
    jsoncpp
    ${OPENSSL_LIBRARIES}
    ${GIO_LIBRARIES}
)

add_executable(testSubjectSegmentation testSubjectSegmentation.cpp
    ${SERVICE_SOURCES}
    ${COMMON_PROTO_FILES}
)

target_link_libraries(testSubjectSegmentation
    ${LIBRARIES}
)


add_executable(testCoreVisionApi testCoreVision.cpp
    ${SERVICE_SOURCES} ${COMMON_PROTO_FILES}
)

target_link_libraries(testCoreVisionApi ${LIBRARIES})


add_executable(testSegmentation testSegmentation.cpp
    ${SERVICE_SOURCES}
    ${COMMON_PROTO_FILES}
)

target_link_libraries(testSegmentation ${LIBRARIES} ${OpenCV_LIBS})
