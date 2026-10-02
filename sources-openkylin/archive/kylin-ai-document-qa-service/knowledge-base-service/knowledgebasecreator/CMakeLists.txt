cmake_minimum_required(VERSION 3.5)

project(faqknowledgebasecreator LANGUAGES CXX C)
project(ragknowledgebasecreator LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(FAQ_TARGET faqknowledgebasecreator)
set(RAG_TARGET ragknowledgebasecreator)

file(GLOB_RECURSE FAQ_SOURCE_LIST
     ${PROJECT_SOURCE_DIR}/faqmain.cpp
     ${PROJECT_SOURCE_DIR}/../src/common/embedding.cpp
     ${PROJECT_SOURCE_DIR}/../src/utils.cpp)


file(GLOB_RECURSE RAG_SOURCE_LIST
     ${PROJECT_SOURCE_DIR}/ragmain.cpp
     ${PROJECT_SOURCE_DIR}/../src/common/embedding.cpp
     ${PROJECT_SOURCE_DIR}/../src/common/fileparser.cpp
     ${PROJECT_SOURCE_DIR}/../src/configmanager.cpp
     ${PROJECT_SOURCE_DIR}/../src/utils.cpp)

message(STATUS "FAQ_SOURCE_LIST: ${FAQ_SOURCE_LIST}")
message(STATUS "RAG_SOURCE_LIST: ${RAG_SOURCE_LIST}")
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Development)
pkg_check_modules(Embedding REQUIRED IMPORTED_TARGET kysdk-coreai-embedding)
pkg_check_modules(KYDOCUMENT REQUIRED IMPORTED_TARGET kylin-ai-document-service)
pkg_check_modules(BUSINESS REQUIRED IMPORTED_TARGET kyai-business-framework)
pkg_check_modules(SSL REQUIRED IMPORTED_TARGET libcrypto)

include_directories(${AI_COMMON_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${BUSINESS_INCLUDE_DIRS})
include_directories(${Python3_INCLUDE_DIRS})

add_executable(${FAQ_TARGET} ${FAQ_SOURCE_LIST})
target_link_libraries(${FAQ_TARGET}
               PUBLIC PkgConfig::BUSINESS
               PUBLIC nlohmann_json::nlohmann_json
               PRIVATE PkgConfig::Embedding
               PUBLIC PkgConfig::SSL)


add_executable(${RAG_TARGET} ${RAG_SOURCE_LIST})
target_link_libraries(${RAG_TARGET}
               PUBLIC PkgConfig::BUSINESS
               PRIVATE nlohmann_json::nlohmann_json
               PRIVATE PkgConfig::Embedding
               PRIVATE PkgConfig::KYDOCUMENT
               PUBLIC PkgConfig::SSL)
