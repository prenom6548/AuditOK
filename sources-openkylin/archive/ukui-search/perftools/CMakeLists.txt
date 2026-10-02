cmake_minimum_required(VERSION 3.16)
project(perftools)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_STANDARD 17)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_MICRO 1)
set(LIBUKUI_SEARCH_TCMALLOC_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

find_package(Threads REQUIRED)
include(CheckCSourceCompiles)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckTypeSize)

set(LOCAL_PERFTOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LOCAL_PERFTOOLS_SRC_DIR ${LOCAL_PERFTOOLS_DIR}/src)
set(LOCAL_PERFTOOLS_CONFIG_DIR ${CMAKE_CURRENT_BINARY_DIR}/perftools_autogen)
file(MAKE_DIRECTORY ${LOCAL_PERFTOOLS_CONFIG_DIR})

# Generate perftools config.h at configure time (aligned with latest gperftools template).
check_include_file("fcntl.h" HAVE_FCNTL_H)
check_include_file("features.h" HAVE_FEATURES_H)
check_include_file("malloc.h" HAVE_MALLOC_H)
check_include_file("sys/cdefs.h" HAVE_SYS_CDEFS_H)
check_include_file("sys/syscall.h" HAVE_SYS_SYSCALL_H)
check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file("unistd.h" HAVE_UNISTD_H)

check_function_exists("sbrk" HAVE_SBRK)

set(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE=600)
check_symbol_exists("memalign" "stdlib.h;malloc.h" HAVE_DECL_MEMALIGN)
check_symbol_exists("posix_memalign" "stdlib.h;malloc.h" HAVE_DECL_POSIX_MEMALIGN)
check_symbol_exists("pvalloc" "stdlib.h;malloc.h" HAVE_DECL_PVALLOC)
check_symbol_exists("valloc" "stdlib.h;malloc.h" HAVE_DECL_VALLOC)
set(CMAKE_REQUIRED_DEFINITIONS)

set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_symbol_exists("program_invocation_name" "errno.h" HAVE_PROGRAM_INVOCATION_NAME)
set(CMAKE_REQUIRED_DEFINITIONS)

set(CMAKE_EXTRA_INCLUDE_FILES "malloc.h")
check_type_size("struct mallinfo" HAVE_STRUCT_MALLINFO LANGUAGE CXX)
check_type_size("struct mallinfo2" HAVE_STRUCT_MALLINFO2 LANGUAGE CXX)
set(CMAKE_EXTRA_INCLUDE_FILES)

check_c_source_compiles("#include <stdlib.h>
                         static void foo(void) __attribute__ ((unused));
                         void foo(void) { exit(1); }
                         int main() { return 0; }"
        HAVE___ATTRIBUTE__)
check_c_source_compiles("#include <stdlib.h>
                         void foo(void) __attribute__((aligned(128)));
                         void foo(void) { exit(1); }
                         int main() { return 0; }"
        HAVE___ATTRIBUTE__ALIGNED_FN)
if(NOT WIN32)
    set(HAVE_MMAP 1)
endif()

configure_file(
        ${LOCAL_PERFTOOLS_DIR}/cmake/config.h.in
        ${LOCAL_PERFTOOLS_CONFIG_DIR}/config.h
)

set(LOCAL_TCMALLOC_MINIMAL_SOURCES
        ${LOCAL_PERFTOOLS_SRC_DIR}/base/logging.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/base/generic_writer.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/base/proc_maps_iterator.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/base/dynamic_annotations.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/base/sysinfo.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/base/spinlock.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/base/spinlock_internal.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/common.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/internal_logging.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/system-alloc.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/memfs_malloc.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/safe_strerror.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/central_freelist.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/page_heap.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/sampler.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/span.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/stack_trace_table.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/static_vars.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/thread_cache.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/thread_cache_ptr.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/malloc_hook.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/malloc_extension.cc
        ${LOCAL_PERFTOOLS_SRC_DIR}/tcmalloc.cc
)
add_library(${PROJECT_NAME} SHARED ${LOCAL_TCMALLOC_MINIMAL_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ukui-search-tcmalloc
        SOVERSION ${VERSION_MAJOR}
        VERSION ${LIBUKUI_SEARCH_TCMALLOC_VERSION}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${LOCAL_PERFTOOLS_CONFIG_DIR}
        ${LOCAL_PERFTOOLS_DIR}
        ${LOCAL_PERFTOOLS_SRC_DIR}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        NO_TCMALLOC_SAMPLES
)

target_link_libraries(perftools PRIVATE Threads::Threads)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftools PUBLIC ${CMAKE_DL_LIBS})
endif()

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
)

if(BUILD_TEST)
    add_subdirectory(autotest)
endif()
