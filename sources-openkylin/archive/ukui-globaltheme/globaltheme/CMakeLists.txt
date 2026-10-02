set(SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dtConvert.sh")
 
# 检查脚本文件是否存在
if(NOT EXISTS "${SCRIPT_PATH}")
  message(FATAL_ERROR "Script does not exist: ${SCRIPT_PATH}")
endif()
 
# 设置脚本为可执行
execute_process(
  COMMAND chmod
  +x "${SCRIPT_PATH}"
  OUTPUT_VARIABLE CHMOD_OUTPUT
)

#扫描当前路径及子目录中的css文件
file(GLOB_RECURSE CSS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.css)
#调用脚本将css转换为qml
foreach(file ${CSS_FILES})
    message("  ${file}")
    # 执行脚本
    execute_process(
      COMMAND "${SCRIPT_PATH}" "${file}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      RESULT_VARIABLE RESULT
      OUTPUT_VARIABLE OUTPUT
      ERROR_VARIABLE ERROR
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
    )
endforeach()
#扫描当前路径及子目录中的qml文件
file(GLOB_RECURSE QML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.qml)

#安装qml文件
set(THEME_INSTALL_PATH /usr/share/config/themeconfig/token/)
install(FILES
        ${QML_FILES}
        DESTINATION ${THEME_INSTALL_PATH})

# 输出结果
if(RESULT EQUAL "0")
  message(STATUS "Script executed successfully")
  message(STATUS "Script output: ${OUTPUT}")
else()
  message(STATUS "Script failed to execute")
  message(STATUS "Error output: ${ERROR}")
endif()
