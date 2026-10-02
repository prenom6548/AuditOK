set(FILETRANSFER_SOURCES
        filesyncmanage.h
        filesyncmanage.cpp
        downloadtask.h
        downloadtask.cpp
        fileparser.h
        fileparser.cpp
        )

add_library(filesync ${FILETRANSFER_SOURCES})

target_include_directories(filesync PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(filesync PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        common
        )