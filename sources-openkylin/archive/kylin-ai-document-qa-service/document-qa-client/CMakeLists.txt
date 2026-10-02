cmake_minimum_required(VERSION 3.5)

project(kyai-knowledgebase-client LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(KylinAiProto REQUIRED)
find_package(nlohmann_json REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-unix-2.0)
kylin_ai_generate_gdbus_proto_code(RETURNED_GDBUS_GLUE_FILES --glib-min-required=2.64 knowledgebase)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${GIO_INCLUDE_DIRS})

file(GLOB SOURCE_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(kyai-knowledgebase-client SHARED 
        ${SOURCE_LIST}
        ${RETURNED_GDBUS_GLUE_FILES})

set_target_properties(kyai-knowledgebase-client PROPERTIES VERSION 1.0.0 SOVERSION 1)

target_link_libraries(kyai-knowledgebase-client 
        PRIVATE PkgConfig::GIO
        PRIVATE nlohmann_json::nlohmann_json
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
                target_link_libraries(kyai-knowledgebase-client PRIVATE stdc++fs)
        endif()
endif()

install(TARGETS kyai-knowledgebase-client
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/
        DESTINATION include)
install(FILES pkgconf/kyai-knowledgebase-client.pc
        DESTINATION /usr/share/pkgconfig)