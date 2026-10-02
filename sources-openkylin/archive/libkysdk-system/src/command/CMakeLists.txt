aux_source_directory(. SOURCESCODE)
include_directories(.)

add_executable(kylin-sysinfo libkycommand.c)

target_link_libraries(kylin-sysinfo kysysinfo)
