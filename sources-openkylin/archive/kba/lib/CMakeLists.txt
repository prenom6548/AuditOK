cmake_minimum_required(VERSION 3.1.3)

project(kba)

aux_source_directory(. SOURCESCODE)

include_directories(${PKGS_INCLUDE_DIRS})
add_library(kba SHARED ${SOURCESCODE})

include_directories(lib ../include/kba)

target_link_libraries(kba -ljson-c ${PKGS_LIBRARIES})

install(TARGETS kba
    DESTINATION lib/)

install(FILES ../include/kba/kba.h
    DESTINATION include/kba)

install(FILES ../conf/question.json
    DESTINATION share/libkba/conf)

install(FILES ../conf/question.json
    DESTINATION etc/kba)