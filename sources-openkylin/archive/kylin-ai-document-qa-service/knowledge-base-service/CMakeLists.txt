cmake_minimum_required(VERSION 3.5)

project(kylin-ai-knowledge-base-service LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_LIST
     ${PROJECT_SOURCE_DIR}/src/common/*.cpp
     ${PROJECT_SOURCE_DIR}/src/hybridretriever/*.cpp
     ${PROJECT_SOURCE_DIR}/src/querydiversifier/*.cpp
     ${PROJECT_SOURCE_DIR}/src/storagemanager/*.cpp
     ${PROJECT_SOURCE_DIR}/src/*.cpp
     server.cpp)

find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Xapian REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Threads REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Development)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(KYDOCUMENT REQUIRED IMPORTED_TARGET kylin-ai-document-service)
pkg_check_modules(BUSINESS REQUIRED IMPORTED_TARGET kyai-business-framework)
pkg_check_modules(AI_COMMON REQUIRED kysdk-ai-common)
pkg_check_modules(GENAI_NLP REQUIRED kysdk-genai-nlp)
pkg_check_modules(VECTOR_CLIENT REQUIRED IMPORTED_TARGET kysdk-vector-engine-client)
pkg_check_modules(SSL REQUIRED IMPORTED_TARGET libcrypto)

include_directories(${AI_COMMON_INCLUDE_DIRS})
include_directories(${GENAI_NLP_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${VECTOR_CLIENT_INCLUDE_DIRS})
include_directories(${Python3_INCLUDE_DIRS})

add_subdirectory(test)

add_executable(${PROJECT_NAME} ${SOURCE_LIST})

target_link_libraries(${PROJECT_NAME}
                      PUBLIC Qt5::Core
                      PUBLIC kysdk-genai-nlp
                      PUBLIC ${GLIB_LIBRARIES}
                      PUBLIC ${Python3_LIBRARIES}
                      PUBLIC PkgConfig::BUSINESS
                      PUBLIC PkgConfig::KYDOCUMENT
                      PUBLIC nlohmann_json::nlohmann_json
                      PUBLIC chinese-segmentation
                      PRIVATE xapian
                      PRIVATE Threads::Threads
                      PUBLIC PkgConfig::VECTOR_CLIENT
                      PUBLIC PkgConfig::SSL)

install(FILES conf/knowledgebaseservice.json DESTINATION /usr/share/kylin-ai/kyai-business-framework)
install(FILES conf/kylin-ai-knowledgebase-service.service DESTINATION /usr/lib/systemd/user)
install(DIRECTORY conf/prompts/ DESTINATION /usr/share/kylin-ai/kylin-ai-knowledgebase-service/prompts FILES_MATCHING PATTERN "*")
install(TARGETS kylin-ai-knowledge-base-service RUNTIME DESTINATION /usr/bin)
