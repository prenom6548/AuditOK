cmake_minimum_required(VERSION 3.5)

project(test-knowledge-base-service LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCE_LIST ${PROJECT_SOURCE_DIR}/*.cpp)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-unix-2.0)

kylin_ai_generate_gdbus_proto_code(RETURNED_GDBUS_GLUE_FILES advaiknowledgebaseservice)
message("gdbus files: ${RETURNED_GDBUS_GLUE_FILES}")

add_executable(${PROJECT_NAME}
               ${SOURCE_LIST}
               ${RETURNED_GDBUS_GLUE_FILES})

target_link_libraries(${PROJECT_NAME}
                      PRIVATE PkgConfig::GIO
                      PRIVATE kyai-business-framework
                      PRIVATE Threads::Threads)
