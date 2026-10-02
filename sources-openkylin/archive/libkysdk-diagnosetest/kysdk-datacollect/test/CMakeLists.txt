add_executable(testdatanode testdatanode.c)
target_include_directories(testdatanode PUBLIC "/usr/include/kysdk/diagnosetest/")
target_link_libraries(testdatanode kysdk-datacollect)
# install(TARGETS testdatanode DESTINATION /usr/bin/ )