cmake_minimum_required(VERSION 3.5)

project(kylin-ai-document-qa-service LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp server.cpp)

find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)
pkg_check_modules(KYDOCUMENT REQUIRED IMPORTED_TARGET kylin-ai-document-service)
pkg_check_modules(BUSINESS REQUIRED IMPORTED_TARGET kyai-business-framework)
pkg_check_modules(VECTOR_DB REQUIRED IMPORTED_TARGET kysdk-vector-engine-client)

include_directories(${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/service
        ${PROJECT_SOURCE_DIR}/src/dao)

add_executable(kylin-ai-document-qa-service ${SOURCE_LIST})

target_link_libraries(kylin-ai-document-qa-service
                    PUBLIC PkgConfig::BUSINESS
                    PUBLIC PkgConfig::KYDOCUMENT
                    PUBLIC nlohmann_json::nlohmann_json
                    PUBLIC PkgConfig::VECTOR_DB
                    )

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(kylin-ai-document-qa-service PRIVATE stdc++fs)
    endif()
endif()

install(FILES conf/KnowledgeBase.json DESTINATION /usr/share/kylin-ai/kyai-business-framework)
install(FILES conf/kylin-ai-document-qa-service.service DESTINATION /usr/lib/systemd/user)
install(TARGETS kylin-ai-document-qa-service RUNTIME DESTINATION /usr/bin)