set (CMAKE_AUTOMOC ON)

aux_source_directory(. SOURCECODE)
include_directories(/usr/include/kysdk/kysdk-system/)

find_package(Qt5 COMPONENTS Core DBus REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5DBus_INCLUDE_DIRS})
ADD_EXECUTABLE(kySdkDbus kysdkdbus.cpp main.cpp)
target_link_libraries(kySdkDbus kysysinfo kyhw kydiskinfo kyrtinfo kynetinfo kyrealtime kydiskinfo kylocation kypackage Qt5::Core Qt5::DBus dl)


ADD_EXECUTABLE(kySdkDbus-session kysdkrest.cpp main_rest.cpp)
target_link_libraries(kySdkDbus-session kysysinfo kyhw Qt5::Core Qt5::DBus)
