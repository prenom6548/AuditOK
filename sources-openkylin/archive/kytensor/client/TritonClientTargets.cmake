add_library(TritonClient::httpclient UNKNOWN IMPORTED)
set_target_properties(TritonClient::httpclient PROPERTIES
    IMPORTED_LOCATION "${TRITONCLIENT_CMAKE_DIR}/../../libhttpclient.so"
    INTERFACE_INCLUDE_DIRECTORIES "/usr/include/triton/client"
)

add_library(TritonClient::grpcclient UNKNOWN IMPORTED)
set_target_properties(TritonClient::grpcclient PROPERTIES
    IMPORTED_LOCATION "${TRITONCLIENT_CMAKE_DIR}/../../libgrpcclient.so"
    INTERFACE_INCLUDE_DIRECTORIES "/usr/include/triton/client"
)

add_library(TritonClient::grpcclient_static STATIC IMPORTED)
set_target_properties(TritonClient::grpcclient_static PROPERTIES
    IMPORTED_LOCATION "${TRITONCLIENT_CMAKE_DIR}/../../libgrpcclient_static.a"
    INTERFACE_INCLUDE_DIRECTORIES "/usr/include/triton/client"
    INTERFACE_LINK_LIBRARIES
      "libgrpc++.a;libgrpc.a;libcares_static.a;libaddress_sorting.a;libupb.a;libabsl_statusor.a;libgpr.a;libabsl_status.a;libabsl_strerror.a;libabsl_flags_internal.a;libabsl_flags_reflection.a;libabsl_hash.a;libabsl_city.a;libabsl_bad_variant_access.a;libabsl_low_level_hash.a;libabsl_raw_hash_set.a;libabsl_hashtablez_sampler.a;libabsl_flags_config.a;libabsl_flags_program_name.a;libabsl_flags_private_handle_accessor.a;libabsl_flags_commandlineflag.a;libabsl_flags_commandlineflag_internal.a;libabsl_flags_marshalling.a;libabsl_random_distributions.a;libabsl_random_seed_sequences.a;libabsl_random_internal_pool_urbg.a;libabsl_random_internal_randen.a;libabsl_random_internal_randen_hwaes.a;libabsl_random_internal_randen_hwaes_impl.a;libabsl_random_internal_randen_slow.a;libabsl_random_internal_platform.a;libabsl_random_internal_seed_material.a;libabsl_random_seed_gen_exception.a;libabsl_cord.a;libabsl_bad_optional_access.a;libabsl_cordz_info.a;libabsl_cord_internal.a;libabsl_cordz_functions.a;libabsl_exponential_biased.a;libabsl_cordz_handle.a;libabsl_str_format_internal.a;libabsl_synchronization.a;libabsl_stacktrace.a;libabsl_symbolize.a;libabsl_debugging_internal.a;libabsl_demangle_internal.a;libabsl_graphcycles_internal.a;libabsl_malloc_internal.a;libabsl_time.a;libabsl_strings.a;libabsl_throw_delegate.a;libabsl_int128.a;libabsl_strings_internal.a;libabsl_base.a;libabsl_spinlock_wait.a;libabsl_raw_logging_internal.a;libabsl_log_severity.a;libabsl_civil_time.a;libabsl_time_zone.a;-lssl;-lcrypto;-lpthread;-lz;libprotobuf.a;libre2.a"
)

add_library(TritonClient::httpclient_static STATIC IMPORTED)
set_target_properties(TritonClient::httpclient_static PROPERTIES
    IMPORTED_LOCATION "${TRITONCLIENT_CMAKE_DIR}/../../libhttpclient_static.a"
    INTERFACE_INCLUDE_DIRECTORIES "/usr/include/triton/client"
    INTERFACE_LINK_LIBRARIES "-lcurl;-lpthread"
)
