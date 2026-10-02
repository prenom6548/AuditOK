set(FILETRANSFER_SOURCES
        ftpclient/ftpmanager.h
        ftpclient/ftpmanager.cpp
        ftpclient/qftp.h
        ftpclient/qftp.cpp
        ftpclient/qurlinfo.h
        ftpclient/qurlinfo.cpp
        )

add_library(filetransfer ${FILETRANSFER_SOURCES})

target_include_directories(filetransfer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/ftpclient
        )

target_link_libraries(filetransfer PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        common
        )