cmake_minimum_required(VERSION 3.10)
project(kylin-model-downloader)

# # 检查Qt5和Qt5 Widgets模块
# find_package(Qt5Widgets REQUIRED)

# # 添加源文件
# set(SRC
#     src/main.cpp
#     src/mainwindow.cpp
#     src/mainwindow.h
#     src/downloader.cpp
#     src/downloader.h
# )

# # 生成 moc 文件
# qt5_generate_moc(src/mainwindow.h moc/mainwindow_moc.cpp)
# qt5_generate_moc(src/downloader.h moc/downloader_moc.cpp)

# # 添加 moc 文件到源文件列表
# list(APPEND SRC moc/mainwindow_moc.cpp)
# list(APPEND SRC moc/downloader_moc.cpp)

# # 创建Qt应用程序目标
# add_executable(kylin-model-downloader ${SRC})

# # 链接Qt5Widgets
# target_link_libraries(kylin-model-downloader Qt5::Widgets)

# # 设置运行时所需的库
# if(APPLE)
#     target_link_libraries(kylin-model-downloader -framework QtWidgets -framework QtGui -framework QtCore)
# elseif(UNIX)
#     target_link_libraries(kylin-model-downloader Qt5::Widgets Qt5::Gui Qt5::Core)
# endif()

# # 设置头文件目录
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

#install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/download.py DESTINATION bin)
#install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/download-in-venv DESTINATION bin)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/modelsmd5 DESTINATION share/kylin-ai/ondevice/)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/kylin-model-management-tools.desktop DESTINATION share/applications)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/com.kylin.ai.model.management.policy DESTINATION share/polkit-1/actions)
#install(TARGETS kylin-model-downloader DESTINATION bin)
