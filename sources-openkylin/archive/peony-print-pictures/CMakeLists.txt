cmake_minimum_required(VERSION 3.5)

project(top)

option(SHELL_EXEC "option for shell exec" ON)

add_subdirectory(3rdparty/kylinimagecodec)
add_subdirectory(peony-print-pictures)
add_subdirectory(kylin-photo-printer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

if (SHELL_EXEC)
    execute_process(COMMAND python3 ${PROJECT_SOURCE_DIR}/python/copy_translations.py
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    execute_process(COMMAND bash ${PROJECT_SOURCE_DIR}/sh/update_translations.sh
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    execute_process(COMMAND bash ${PROJECT_SOURCE_DIR}/sh/apply_translations.sh
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif(SHELL_EXEC)

file(GLOB TRANS
    "translations/*.qm"
)

install(FILES ${TRANS} DESTINATION  /usr/share/peony-print-pictures/translations/)
