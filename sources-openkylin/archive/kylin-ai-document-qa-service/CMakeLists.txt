cmake_minimum_required(VERSION 3.5)

project(kylin-ai-document-qa-service LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(document-qa-service)
add_subdirectory(document-qa-client)
add_subdirectory(knowledge-base-service)

