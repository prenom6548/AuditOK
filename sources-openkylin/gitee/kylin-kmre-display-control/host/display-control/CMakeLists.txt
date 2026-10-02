set(SOURCES
    main.cpp
    display_control.c
    utils/debug.c
    utils/dll.c)

add_executable(display_control ${SOURCES})
add_dependencies(display_control Headers)
target_link_libraries(display_control
    DisplayControl dl pthread)
