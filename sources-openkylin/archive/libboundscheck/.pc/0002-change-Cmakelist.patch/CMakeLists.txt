cmake_minimum_required(VERSION 3.10)
set(TARGET boundscheck)
project(boundscheck)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat=2 -Wfloat-equal -Wshadow")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wconversion")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat-security")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --param ssp-buffer-size=4")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Warray-bounds")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-qual")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-overflow=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-aliasing=2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wswitch -Wswitch-default")

include_directories(include)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -s -Wl,-z,relro,-z,now,-z,noexecstack -fstack-protector-all")


file(GLOB SOURCES "src/*.c")
add_library(${TARGET} SHARED ${SOURCES})
install(TARGETS ${TARGET}
	DESTINATION ${CMAKE_INSTALL_BOUNDSCHECK}/${CMAKE_INSTALL_LIBDIR}
)
