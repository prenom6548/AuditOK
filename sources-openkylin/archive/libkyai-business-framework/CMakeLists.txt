cmake_minimum_required(VERSION 3.5)

project(kyai-business-framework LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS -rdynamic) 用来打印调用栈
#add_definitions("-Wall -g")

file(GLOB_RECURSE SOURCE_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE RPC_EMBEDDING_SOURCE_LIST ${PROJECT_SOURCE_DIR}/src/rpc/embedding/*.cpp ${PROJECT_SOURCE_DIR}/src/rpc/embedding/*.c)
file(GLOB_RECURSE INCLUDE_FLIES ${PROJECT_SOURCE_DIR}/include/kyai-business-framework/*.h)

include_directories(${PROJECT_SOURCE_DIR}/include/kyai-business-framework)
include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)

find_package(PkgConfig REQUIRED)
find_package(KylinAiProto REQUIRED)
find_package(nlohmann_json REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-unix-2.0)
pkg_check_modules(SQLITE REQUIRED IMPORTED_TARGET sqlite3)
pkg_check_modules(UUID REQUIRED IMPORTED_TARGET uuid)
pkg_check_modules(Embedding REQUIRED IMPORTED_TARGET kysdk-coreai-embedding)

# 生成接口 glue 文件，当新增场景接口，继续往后面加xml文件名称，如果要跑 demo 需要把 example 加上
kylin_ai_generate_gdbus_proto_code(RETURNED_GDBUS_GLUE_FILES knowledgebase datamanagementservice advaiknowledgebaseservice)
#kylin_ai_generate_gdbus_proto_code(RETURNED_GDBUS_GLUE_FILES example knowledgebase datamanagementservice)

message("gdbus files: ${RETURNED_GDBUS_GLUE_FILES}")

add_library(kyai-business-framework SHARED
            ${SOURCE_LIST}
            ${RETURNED_GDBUS_GLUE_FILES}
            ${RPC_EMBEDDING_SOURCE_LIST}
            ${INCLUDE_FLIES})

# 设置动态库版本号
set_target_properties(kyai-business-framework PROPERTIES VERSION 0.0.1 SOVERSION 1)

# 获取系统处理器架构
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|aarch64)$")
    # amd64或arm64
    message(STATUS "Building for x86_64 or aarch64 architecture: ${CMAKE_SYSTEM_PROCESSOR}.")
    pkg_check_modules(VECTOR_DB REQUIRED IMPORTED_TARGET kysdk-vector-engine-client)
    target_include_directories(kyai-business-framework PUBLIC ${VECTOR_DB_INCLUDE_DIRS})
    target_link_libraries(kyai-business-framework PUBLIC PkgConfig::VECTOR_DB)

    # 设置用于.pc文件的变量
    set(VECTOR_ENGINE_LIBS "-lkysdk-vector-engine-client")
    set(VECTOR_ENGINE_CFLAGS "-I/usr/include/kysdk-vector-engine-client")
else()
    # 其他架构
    message(STATUS "Building for non-x86_64 and non-aarch64 architecture: ${CMAKE_SYSTEM_PROCESSOR}.")
    add_definitions(-DDISABLE_VECTOR_ENGINE)

    # 非目标架构时，设置空值
    set(VECTOR_ENGINE_LIBS "")
    set(VECTOR_ENGINE_CFLAGS "")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/usr/share/pkgconfig/kyai-business-framework.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/kyai-business-framework.pc
    @ONLY
)

target_include_directories(kyai-business-framework
        PUBLIC ${PROJECT_SOURCE_DIR}/include/kyai-business-framework
        PRIVATE ${GIO_INCLUDE_DIRS}
        PRIVATE ${SQLITE_INCLUDE_DIRS}
        PRIVATE ${UUID_INCLUDE_DIRS}
        PRIVATE ${Embedding_INCLUDE_DIRS})

target_link_libraries(kyai-business-framework
        PUBLIC pthread
        PRIVATE PkgConfig::GIO
        PRIVATE PkgConfig::SQLITE
        PRIVATE PkgConfig::UUID
        PRIVATE PkgConfig::Embedding
        PUBLIC nlohmann_json::nlohmann_json)

install(TARGETS kyai-business-framework
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY include/kyai-business-framework
        DESTINATION include)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kyai-business-framework.pc
        DESTINATION /usr/share/pkgconfig)

if (DEFINED ENABLE_DEMO)
    unset(ENABLE_DEMO CACHE)
endif(DEFINED ENABLE_DEMO)
option(ENABLE_DEMO "Build Demo" OFF)
if (ENABLE_DEMO)
    add_definitions(-DENABLE_EXAMPLE)
    add_subdirectory(demo)
endif(ENABLE_DEMO)

if (DEFINED ENABLE_TEST)
    unset(ENABLE_TEST CACHE)
endif(DEFINED ENABLE_TEST)

option(ENABLE_TEST "Build Test" OFF)
if (ENABLE_TEST)
    add_subdirectory(test)
    add_definitions(-DENABLE_TEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif (ENABLE_TEST)
