cmake_minimum_required(VERSION 3.10)
project(kylin-ai-document-service LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED IMPORTED_TARGET grpc)
find_package(KylinAiProto REQUIRED)
pkg_check_modules(gRPC REQUIRED grpc++)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

include_directories(include)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS document.proto)

kylin_ai_generate_grpc_proto_source(RETURNED_GRPC_SOURCE_FILES document-service document)
message("grpc source files: ${RETURNED_GRPC_SOURCE_FILES}")
kylin_ai_generate_grpc_proto_service(RETURNED_GRPC_SERVICE_FILES document-service document)
message("grpc source files: ${RETURNED_GRPC_SERVICE_FILES}")

add_library(kylin-ai-document-service SHARED
    src/client.cpp
    src/stub_manager.cpp
    ${RETURNED_GRPC_SOURCE_FILES}
    ${RETURNED_GRPC_SERVICE_FILES}
)

set_target_properties(kylin-ai-document-service PROPERTIES VERSION 2.0.0 SOVERSION 2)

include(CMakePackageConfigHelpers)

target_link_libraries(kylin-ai-document-service
    PUBLIC ${gRPC_LIBRARIES}
    PUBLIC protobuf
    PUBLIC ${SYSTEMD_LIBRARIES}
)

install(TARGETS kylin-ai-document-service
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include
    DESTINATION /usr/include/kylin-ai-document-service)

install(FILES pkg-configs/kylin-ai-document-service.pc DESTINATION /usr/share/pkgconfig)
install(DIRECTORY python/ DESTINATION /usr/share/kylin-ai-document-service/python)
install(DIRECTORY configs/lib/systemd/user/ DESTINATION /usr/lib/systemd/user)


if (DEFINED ENABLE_TEST)
    unset(ENABLE_TEST CACHE)
endif(DEFINED ENABLE_TEST)
option(ENABLE_TEST "Build Test" OFF)
if (ENABLE_TEST)
    add_subdirectory(test)
endif (ENABLE_TEST)