cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(kaiming-builder
        VERSION 0.0.0.1
        DESCRIPTION "kaiming-builder packages"
        HOMEPAGE_URL ""
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 默认构建类型为Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
#set(CMAKE_BUILD_TYPE "debug")

# 安全编译选项
# -z,relro:Relocation Read Only。Create an ELF "PT_GNU_RELRO" segment header in the object。
# 也就是会创建一个PT_GNU_RELRO段，这个重定向区域时只读的。这个是用来保护目标文件防止被篡改，编译器一般都会默认带上此参数。
# -z,now常用于跟-z,relro一起使用，用于保护生成的目标文件被篡改。设置这个参数表明在程序运行或者dlopen这个库的时候就将所有的动态符号加载并绑定进来，而不是在动态符号需要使用的时候加载。
# -z,noexecstack 标记当前链接的目标文件是不需要executable stack的。
set(CMAKE_C_FLAGS "$ENV{CFLAGS} -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIC -fPIE -pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++17 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIC -fPIE -pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(LOCALEDIR "/opt/system/resource/locale")
# set(CMAKE_C_COMPILER gcc-12)
# set(CMAKE_CXX_COMPILER g++-12)
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBCRYPTO REQUIRED libcrypto)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
pkg_check_modules(JSON REQUIRED nlohmann_json)

# 手动添加额外依赖库（静态链接需要显式指定）
set(EXTRA_STATIC_LIBS
    -lz
    -lm
    -lmvec
    -ldl
    -pthread
)

configure_file(config.h.in config.h @ONLY)

aux_source_directory(src SRC_DIR)
aux_source_directory(src/common SRC_COMMON)

add_compile_options(-Wno-unused-result)
add_link_options(-static-libgcc -static-libstdc++)

add_executable(kaiming-builder
    ${SRC_DIR}
    ${SRC_COMMON}
)

target_include_directories(kaiming-builder PRIVATE
    ${LIBCRYPTO_INCLUDE_DIRS}
    ${YAMLCPP_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIRS}
)

target_link_libraries(kaiming-builder PRIVATE
    -Wl,-Bstatic
    ${LIBCRYPTO_LIBRARIES}
    ${YAMLCPP_LIBRARIES}
    # ${EXTRA_STATIC_LIBS}
    -Wl,-Bdynamic
)

# 安装二进制到  /opt/kaiming-tools/bin
install( TARGETS kaiming-builder
    RUNTIME DESTINATION  /opt/kaiming-tools/bin
)

set(USR_SHARE_PATH "/opt/system/resource")
set(ETC_PATH "/opt/system/conf")

# 部署yaml模板到 /opt/kaiming-tools/config
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/template.yaml
        DESTINATION  /opt/kaiming-tools/config
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/cn.kylin.kaiming-builder.policy
    DESTINATION ${USR_SHARE_PATH}/polkit-1/actions
)

# 安装 kaiming-builder 命令补全脚本
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/shells/kaiming-builder_completion.sh
        DESTINATION ${ETC_PATH}/bash_completion.d
)

# 安装 kaiming-buildpackage
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/shells/kaiming-buildpackage
        DESTINATION /opt/kaiming-tools/bin
)

# 安装 kch
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/shells/kch
        DESTINATION /opt/kaiming-tools/bin
)

# 安装 build_post_hook.sh
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/shells/build_post_hook.sh
        DESTINATION /opt/kaiming-tools/bin
)

#########################################################################################翻译文件

# 设置 POT 文件、PO文件和目标 MO 文件的路径
set(POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/po/kaiming-builder.pot)
set(PO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/po/kaiming-builder.po)
set(MO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/po/kaiming-builder.mo)  # 在构建目录下生成 .mo 文件

# # 生成pot
# add_custom_command(
#     # generate_pot
#     OUTPUT ${POT_FILE}
#     COMMAND xgettext -k_ --from-code=UTF-8 -o ${POT_FILE} ${SOURCE_FILE}
#     DEPENDS ${SOURCE_FILE}
#     COMMENT "Generating ${POT_FILE}"
# )

# # 更新po,文件添加需要新翻译的内容时使用
# add_custom_command(
#     OUTPUT ${PO_FILE}
#     COMMAND msgmerge -U ${PO_FILE} ${POT_FILE}
#     DEPENDS ${POT_FILE}
#     COMMENT "Generating ${PO_FILE}"
# )

# 生成po 开始没有po文件时使用
# add_custom_command(
#     OUTPUT ${PO_FILE}
#     COMMAND msginit -l zh_CN.UTF-8 -i ${POT_FILE} -o ${PO_FILE}
#     DEPENDS ${POT_FILE}
#     COMMENT "Generating ${PO_FILE}"
# )

# 生成mo
add_custom_command(
    OUTPUT ${MO_FILE}  # 输出的 .mo 文件
    COMMAND msgfmt --output-file ${MO_FILE} ${PO_FILE}  # msgfmt 转换命令
    DEPENDS ${PO_FILE}  # 声明 .mo 文件依赖于 .po 文件
    COMMENT "Converting PO to MO file"  # 转换注释
    VERBATIM  # 保证命令中的空格问题
)

# 创建自定义目标，确保在构建时生成 .mo 文件
add_custom_target(
    translations ALL # 将此目标设置为构建时自动执行

    # 仅生成mo文件时使用
    DEPENDS ${MO_FILE}
)

# 安装 .mo 文件到指定的系统目录
install(
    FILES ${MO_FILE}  # 安装生成的 .mo 文件
    DESTINATION ${USR_SHARE_PATH}/locale/zh_CN/LC_MESSAGES  # 安装路径

)

#删除生成的mo文件
install(
    CODE "file(REMOVE \"${MO_FILE}\")"
)


############################################################################################翻译
