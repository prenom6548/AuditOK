project(ukui-qqc2-style)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Gui
)

file(GLOB QML_UKUIFILES "org.ukui.style/*.qml" "org.ukui.style/qmldir")
file(GLOB PRIVATE_FILES "org.ukui.style/private/*")
file(GLOB_RECURSE OTHER_FILES qml.qrc)

SOURCE_GROUP("QMLFiles" FILES ${QML_UKUIFILES})
SOURCE_GROUP("QMLPrivateFiles" FILES ${PRIVATE_FILES})
SOURCE_GROUP("other files" FILES ${OTHER_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${QMLFiles} ${QMLPrivateFiles} ${OTHER_FILES})

message("cmake_install_libdir..." , ${CMAKE_INSTALL_LIBDIR})

if(UNIX)
#    file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/org.ukui.style/*.qml)
message("CMAKE_LIBRARY_ARCHITECTURE123....." "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/QtQuick/Controls.2/org.ukui.style/")


    set(TARGET_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/QtQuick/Controls.2/org.ukui.style/")
    message("qqc2 targetpath", ${TARGET_PATH})
    set(TARGET_UKUIFILES ${QML_UKUIFILES})
    install(FILES ${TARGET_UKUIFILES} DESTINATION ${TARGET_PATH})
    install(FILES ${PRIVATE_FILES} DESTINATION "${TARGET_PATH}/private")
endif()
