cmake_minimum_required(VERSION 3.10)
project(pag)

# 添加子目录参与统一编译
add_subdirectory(ffavc)
add_subdirectory(libpag)

include(GNUInstallDirs)
install(TARGETS pag ffavc tgfx
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ffavc/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ffavc)
install(DIRECTORY libpag/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY libpag/third_party/tgfx/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
