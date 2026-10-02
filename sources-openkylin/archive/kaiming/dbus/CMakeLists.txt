cmake_minimum_required(VERSION 3.10)

# 配置生成静态库
set(XML_FILE ${CMAKE_SOURCE_DIR}/dbus/kmsysteminterface.xml)
set(GENERATED_FILE_PREFIX ${CMAKE_BINARY_DIR}/dbus/kmsysteminterface)

set(XML_FILE_SESSION ${CMAKE_SOURCE_DIR}/dbus/kmsessionproxy.xml)
set(GENERATED_FILE_PREFIX_SESSION ${CMAKE_BINARY_DIR}/dbus/kmsessionproxy)

set(XML_FILE_ASYNC ${CMAKE_SOURCE_DIR}/dbus/kmsystemproxy.xml)
set(GENERATED_FILE_PREFIX_ASYNC ${CMAKE_BINARY_DIR}/dbus/kmsystemproxy)

# 生成 kmsysteminterface 的 C 和 H 文件
add_custom_command(
    OUTPUT ${GENERATED_FILE_PREFIX}.c ${GENERATED_FILE_PREFIX}.h
    COMMAND gdbus-codegen
            --interface-prefix=org.kylin.
            # --c-namespace=kaiming
            --generate-c-code=${GENERATED_FILE_PREFIX}
            ${XML_FILE}
    DEPENDS ${XML_FILE}
    COMMENT "Generating C and H files from XML using gdbus-codegen"
)

# 生成 kmsessionproxy 的 C 和 H 文件
add_custom_command(
    OUTPUT ${GENERATED_FILE_PREFIX_SESSION}.c ${GENERATED_FILE_PREFIX_SESSION}.h
    COMMAND gdbus-codegen
            --interface-prefix=org.kaiming.
            # --c-namespace=kaiming
            --generate-c-code=${GENERATED_FILE_PREFIX_SESSION}
            ${XML_FILE_SESSION}
    DEPENDS ${XML_FILE_SESSION}
    COMMENT "Generating C and H files from XML using gdbus-codegen for session interface"
)

# 生成 kmsystemasyncinterface 的 C 和 H 文件
add_custom_command(
    OUTPUT ${GENERATED_FILE_PREFIX_ASYNC}.c ${GENERATED_FILE_PREFIX_ASYNC}.h
    COMMAND gdbus-codegen
            --interface-prefix=org.kaiming.
            --generate-c-code=${GENERATED_FILE_PREFIX_ASYNC}
            ${XML_FILE_ASYNC}
    DEPENDS ${XML_FILE_ASYNC}
    COMMENT "Generating C and H files from XML using gdbus-codegen for async interface"
)

# 编译 kmsysteminterface 的 C 文件
add_custom_command(
    OUTPUT ${GENERATED_FILE_PREFIX}.o
    COMMAND gcc -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIC -fPIE -pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -c ${GENERATED_FILE_PREFIX}.c
            -o ${GENERATED_FILE_PREFIX}.o
            ${GLIB_CFLAGS} ${GIO2_CFLAGS} ${GIO_UNIX2_CFLAGS}
    DEPENDS ${GENERATED_FILE_PREFIX}.c ${GENERATED_FILE_PREFIX}.h
    COMMENT "Compiling kmsysteminterface.c to kmsysteminterface.o"
)

# 编译 kmsessionproxy 的 C 文件
add_custom_command(
    OUTPUT ${GENERATED_FILE_PREFIX_SESSION}.o
    COMMAND gcc -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIC -fPIE -pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -c ${GENERATED_FILE_PREFIX_SESSION}.c
            -o ${GENERATED_FILE_PREFIX_SESSION}.o
            ${GLIB_CFLAGS} ${GIO2_CFLAGS} ${GIO_UNIX2_CFLAGS}
    DEPENDS ${GENERATED_FILE_PREFIX_SESSION}.c ${GENERATED_FILE_PREFIX_SESSION}.h
    COMMENT "Compiling kmsessionproxy.c to kmsessionproxy.o"
)

# 编译 kmsystemasyncinterface 的 C 文件
add_custom_command(
    OUTPUT ${GENERATED_FILE_PREFIX_ASYNC}.o
    COMMAND gcc -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIC -fPIE -pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -c ${GENERATED_FILE_PREFIX_ASYNC}.c
            -o ${GENERATED_FILE_PREFIX_ASYNC}.o
            ${GLIB_CFLAGS} ${GIO2_CFLAGS} ${GIO_UNIX2_CFLAGS}
    DEPENDS ${GENERATED_FILE_PREFIX_ASYNC}.c ${GENERATED_FILE_PREFIX_ASYNC}.h
    COMMENT "Compiling kmsystemproxy.c to kmsystemproxy.o"
)

# 打包 kmsysteminterface.o 到静态库 kmsysteminterface.a
add_custom_command(
    OUTPUT ${GENERATED_FILE_PREFIX}.a
    COMMAND ar -rc ${GENERATED_FILE_PREFIX}.a ${GENERATED_FILE_PREFIX}.o
    DEPENDS ${GENERATED_FILE_PREFIX}.o
    COMMENT "Packing kmsysteminterface.o into kmsysteminterface.a"
)

# 打包 kmsessionproxy.o 到静态库 kmsessionproxy.a
add_custom_command(
    OUTPUT ${GENERATED_FILE_PREFIX_SESSION}.a
    COMMAND ar -rc ${GENERATED_FILE_PREFIX_SESSION}.a ${GENERATED_FILE_PREFIX_SESSION}.o
    DEPENDS ${GENERATED_FILE_PREFIX_SESSION}.o
    COMMENT "Packing kmsessionproxy.o into kmsessionproxy.a"
)

# 打包 kmsystemasyncinterface.o 到静态库 kmsystemasyncinterface.a
add_custom_command(
    OUTPUT ${GENERATED_FILE_PREFIX_ASYNC}.a
    COMMAND ar -rc ${GENERATED_FILE_PREFIX_ASYNC}.a ${GENERATED_FILE_PREFIX_ASYNC}.o
    DEPENDS ${GENERATED_FILE_PREFIX_ASYNC}.o
    COMMENT "Packing kmsystemproxy.o into kmsystemproxy.a"
)

# 定义目标，依赖生成的静态库
add_custom_target(build_kmsysteminterface DEPENDS ${GENERATED_FILE_PREFIX}.a)
add_custom_target(build_kmsessionproxy DEPENDS ${GENERATED_FILE_PREFIX_SESSION}.a)
add_custom_target(build_kmsystemproxy DEPENDS ${GENERATED_FILE_PREFIX_ASYNC}.a)