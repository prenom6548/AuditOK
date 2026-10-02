add_executable(kyimage-test kyimage-test.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(KYSDKIMAGEPROC kysdk-imageproc)
target_include_directories(kyimage-test PRIVATE ${KYSDKIMAGEPROC_INCLUDE_DIRS})
target_link_directories(kyimage-test PRIVATE ${KYSDKIMAGEPROC_LIBRARY_DIRS})
target_link_libraries(kyimage-test PRIVATE ${KYSDKIMAGEPROC_LIBRARIES})