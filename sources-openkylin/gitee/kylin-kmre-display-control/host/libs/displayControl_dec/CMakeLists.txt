set(GENERATED_SOURCES
    displayControl_dec.cpp
    displayControl_server_context.cpp)


add_library(displayControl_dec STATIC ${GENERATED_SOURCES})
target_link_libraries(displayControl_dec)
