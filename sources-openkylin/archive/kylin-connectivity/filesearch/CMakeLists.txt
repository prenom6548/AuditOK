set(DATABASE_SOURCES
        database/searchdatabase.h
        database/searchdatabase.cpp
        database/filedatabase.h
        database/filedatabase.cpp
        )
source_group(database FILES ${DATABASE_SOURCES})

set(FILESEARCH_SOURCES
        ${DATABASE_SOURCES}
        searchserver.h
        searchserver.cpp
        dirmanager.h
        dirmanager.cpp
        )

add_library(filesearch ${FILESEARCH_SOURCES})

find_package(Qt5Sql COMPONENTS Sql REQUIRED)

target_include_directories(filesearch PUBLIC ${Qt5Sql_INCLUDE_DIRS})
target_include_directories(filesearch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(filesearch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/database)

target_link_libraries(filesearch PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        ${Qt5Sql_LIBRARIES}
        sqlite3
        pthread
        common
        )
