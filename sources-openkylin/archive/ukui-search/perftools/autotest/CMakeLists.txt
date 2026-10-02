set(PERFTOOLS_TEST_COMMON_SOURCES
    ../src/base/logging.cc
    ../src/base/generic_writer.cc
    ../src/base/proc_maps_iterator.cc
    ../src/base/dynamic_annotations.cc
    ../src/base/sysinfo.cc
    ../src/internal_logging.cc
    ../src/safe_strerror.cc
)

add_executable(perftoolsCoreTest
    perftools-core-test.cpp
    ${PERFTOOLS_TEST_COMMON_SOURCES}
)

target_include_directories(perftoolsCoreTest PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_compile_definitions(perftoolsCoreTest PRIVATE
    NO_TCMALLOC_SAMPLES
)

target_link_libraries(perftoolsCoreTest PRIVATE
    Threads::Threads
)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftoolsCoreTest PRIVATE ${CMAKE_DL_LIBS})
endif()

add_test(perftoolsCoreTest ${CMAKE_BINARY_DIR}/perftools/autotest/perftoolsCoreTest)

add_executable(perftoolsWhiteboxTest
    perftools-whitebox-test.cpp
    ../src/sampler.cc
    ../src/stack_trace_table.cc
    ../src/base/spinlock.cc
    ../src/base/spinlock_internal.cc
    ../src/system-alloc.cc
    ../src/malloc_hook.cc
    ${PERFTOOLS_TEST_COMMON_SOURCES}
)

target_include_directories(perftoolsWhiteboxTest PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_compile_definitions(perftoolsWhiteboxTest PRIVATE
    STACK_TRACE_TABLE_IS_TESTED
    NO_TCMALLOC_SAMPLES
)

target_link_libraries(perftoolsWhiteboxTest PRIVATE
    Threads::Threads
)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftoolsWhiteboxTest PRIVATE ${CMAKE_DL_LIBS})
endif()

add_test(perftoolsWhiteboxTest ${CMAKE_BINARY_DIR}/perftools/autotest/perftoolsWhiteboxTest)

add_executable(perftoolsMallocExtensionDefaultsTest
    perftools-malloc-extension-defaults-test.cpp
    ../src/malloc_extension.cc
    ${PERFTOOLS_TEST_COMMON_SOURCES}
)

target_include_directories(perftoolsMallocExtensionDefaultsTest PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_compile_definitions(perftoolsMallocExtensionDefaultsTest PRIVATE
    NO_TCMALLOC_SAMPLES
)

target_link_libraries(perftoolsMallocExtensionDefaultsTest PRIVATE
    Threads::Threads
)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftoolsMallocExtensionDefaultsTest PRIVATE ${CMAKE_DL_LIBS})
endif()

add_test(perftoolsMallocExtensionDefaultsTest ${CMAKE_BINARY_DIR}/perftools/autotest/perftoolsMallocExtensionDefaultsTest)

add_executable(perftoolsSharedAllocatorTest
    perftools-shared-allocator-test.cpp
)

target_include_directories(perftoolsSharedAllocatorTest PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_compile_definitions(perftoolsSharedAllocatorTest PRIVATE
    NO_TCMALLOC_SAMPLES
)

target_link_libraries(perftoolsSharedAllocatorTest PRIVATE
    perftools
    Threads::Threads
)

add_test(perftoolsSharedAllocatorTest ${CMAKE_BINARY_DIR}/perftools/autotest/perftoolsSharedAllocatorTest)

add_library(perftoolsSampledTestLib STATIC
    ${LOCAL_TCMALLOC_MINIMAL_SOURCES}
    perftools-stacktrace-stub.cpp
)

target_include_directories(perftoolsSampledTestLib PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_link_libraries(perftoolsSampledTestLib PRIVATE Threads::Threads)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftoolsSampledTestLib PRIVATE ${CMAKE_DL_LIBS})
endif()

add_executable(perftoolsSampledAllocatorTest
    perftools-sampled-allocator-test.cpp
)

target_include_directories(perftoolsSampledAllocatorTest PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_link_libraries(perftoolsSampledAllocatorTest PRIVATE
    perftoolsSampledTestLib
    Threads::Threads
)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftoolsSampledAllocatorTest PRIVATE ${CMAKE_DL_LIBS})
endif()

add_test(perftoolsSampledAllocatorTest ${CMAKE_BINARY_DIR}/perftools/autotest/perftoolsSampledAllocatorTest)

add_library(perftoolsLargeAllocReportTestLib STATIC
    ${LOCAL_TCMALLOC_MINIMAL_SOURCES}
    perftools-stacktrace-stub.cpp
)

target_include_directories(perftoolsLargeAllocReportTestLib PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_compile_definitions(perftoolsLargeAllocReportTestLib PRIVATE
    NO_TCMALLOC_SAMPLES
    ENABLE_LARGE_ALLOC_REPORT
)

target_link_libraries(perftoolsLargeAllocReportTestLib PRIVATE Threads::Threads)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftoolsLargeAllocReportTestLib PRIVATE ${CMAKE_DL_LIBS})
endif()

add_executable(perftoolsLargeAllocReportEnabledTest
    perftools-large-alloc-report-test.cpp
)

target_include_directories(perftoolsLargeAllocReportEnabledTest PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_link_libraries(perftoolsLargeAllocReportEnabledTest PRIVATE
    perftoolsLargeAllocReportTestLib
    Threads::Threads
)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftoolsLargeAllocReportEnabledTest PRIVATE ${CMAKE_DL_LIBS})
endif()

add_test(perftoolsLargeAllocReportEnabledTest ${CMAKE_BINARY_DIR}/perftools/autotest/perftoolsLargeAllocReportEnabledTest)

add_executable(perftoolsLargeAllocReportNegativeThresholdTest
    perftools-large-alloc-report-test.cpp
)

target_include_directories(perftoolsLargeAllocReportNegativeThresholdTest PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_compile_definitions(perftoolsLargeAllocReportNegativeThresholdTest PRIVATE
    LARGE_ALLOC_REPORT_NEGATIVE_THRESHOLD
)

target_link_libraries(perftoolsLargeAllocReportNegativeThresholdTest PRIVATE
    perftoolsLargeAllocReportTestLib
    Threads::Threads
)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftoolsLargeAllocReportNegativeThresholdTest PRIVATE ${CMAKE_DL_LIBS})
endif()

add_test(perftoolsLargeAllocReportNegativeThresholdTest ${CMAKE_BINARY_DIR}/perftools/autotest/perftoolsLargeAllocReportNegativeThresholdTest)

add_executable(perftoolsPageHeapWhiteboxTest
    perftools-page-heap-whitebox-test.cpp
)

target_include_directories(perftoolsPageHeapWhiteboxTest PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_compile_definitions(perftoolsPageHeapWhiteboxTest PRIVATE
    NO_TCMALLOC_SAMPLES
)

target_link_libraries(perftoolsPageHeapWhiteboxTest PRIVATE
    perftools
    Threads::Threads
)

add_test(perftoolsPageHeapWhiteboxTest ${CMAKE_BINARY_DIR}/perftools/autotest/perftoolsPageHeapWhiteboxTest)

add_executable(perftoolsMemfsAllocatorTest
    perftools-memfs-allocator-test.cpp
    ../src/base/logging.cc
    ../src/base/generic_writer.cc
    ../src/internal_logging.cc
    ../src/safe_strerror.cc
)

target_include_directories(perftoolsMemfsAllocatorTest PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_compile_definitions(perftoolsMemfsAllocatorTest PRIVATE
    NO_TCMALLOC_SAMPLES
)

target_link_libraries(perftoolsMemfsAllocatorTest PRIVATE
    Threads::Threads
)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftoolsMemfsAllocatorTest PRIVATE ${CMAKE_DL_LIBS})
endif()

add_test(perftoolsMemfsAllocatorTest ${CMAKE_BINARY_DIR}/perftools/autotest/perftoolsMemfsAllocatorTest)

add_executable(perftoolsProcMapsWhiteboxTest
    perftools-proc-maps-whitebox-test.cpp
    ../src/base/logging.cc
    ../src/base/generic_writer.cc
    ../src/base/dynamic_annotations.cc
    ../src/base/sysinfo.cc
    ../src/internal_logging.cc
    ../src/safe_strerror.cc
)

target_include_directories(perftoolsProcMapsWhiteboxTest PRIVATE
    ${LOCAL_PERFTOOLS_CONFIG_DIR}
    ${LOCAL_PERFTOOLS_DIR}
    ${LOCAL_PERFTOOLS_SRC_DIR}
)

target_compile_definitions(perftoolsProcMapsWhiteboxTest PRIVATE
    NO_TCMALLOC_SAMPLES
)

target_link_libraries(perftoolsProcMapsWhiteboxTest PRIVATE
    Threads::Threads
)

if(CMAKE_DL_LIBS)
    target_link_libraries(perftoolsProcMapsWhiteboxTest PRIVATE ${CMAKE_DL_LIBS})
endif()

add_test(perftoolsProcMapsWhiteboxTest ${CMAKE_BINARY_DIR}/perftools/autotest/perftoolsProcMapsWhiteboxTest)
