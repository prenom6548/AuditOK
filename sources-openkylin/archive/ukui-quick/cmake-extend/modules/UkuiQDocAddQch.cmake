#[=======================================================================[.rst:
当前模块提供了 ‘ukui_qdoc_add_qch’ 函数用于自动化生成Qt风格的API文档。
以及ukui_install_qch_export 函数用于导出文档的CMake目标，用于查找和创建指向QCH文件的链接

ukui_qdoc_add_qch(<target_name>
      NAME <name>
      DOC_NAME <doc_name>
      VERSION <version>
      [NAMESPACE <namespace>]
      QCH_INSTALL_DESTINATION <qchfile_install_path>
      TAGFILE_INSTALL_DESTINATION <tagsfile_install_path>
      [CONFIG_TEMPLATE <configtemplate_file>]
      [STYLE_FILE <style_file>]
      [SOURCES <file> [<file2> [...]]]
      [SOURCE_DIRS <dir> [<dir2> [...]]]
      [HEADERS <file> [<file2> [...]]]
      [HEADER_DIRS <dir> [<dir2> [...]]]
      [INCLUDE_DIRS <dir> [<dir2> [...]]]
      [IMAGES <file> [<file2> [...]]]
      [IMAGE_DIRS <dir> [<dir2> [...]]]
      [EXAMPLES <file> [<file2> [...]]]
      [EXAMPLE_DIRS <dir> [<dir2> [...]]]
      [EXCLUDE_FILES <file> [<file2> [...]]]
      [EXCLUDE_DIRS <dir> [<dir2> [...]]]
      [DEPEND_QCHS <qch> [<qch2> [...]]]
  )

’NAME’ 用于显示的名称
‘DOC_NAME’ 文档名称
‘VERSION’ 版本号
‘NAMESPACE’ 命名空间
‘QCH_INSTALL_DESTINATION’ qch 文件安装路径
‘TAGFILE_INSTALL_DESTINATION’ tags 文件安装路径
‘CONFIG_TEMPLATE’ 配置模板文件，可参考QDOCConfig.qdocconf.in 编写
‘STYLE_FILE’ 指定文档样式文件，如不显示指定，默认(default.css)和Qt文档风格一致
‘SOURCES’ 源码文件，可以和SOURCE_DIRS 一起使用，指定源码文件也需要同步指定对应的头文件
‘SOURCE_DIRS’ 源文件目录
‘HEADERS’ 头文件，可以和HEADER_DIRS 一起使用
‘HEADER_DIRS’ 头文件目录
‘INCLUDE_DIRS’ 包含目录
‘IMAGES’ 图片文件，文档注释中使用的图片文件
‘IMAGE_DIRS’ 图片文件目录
‘EXAMPLES’ 示例文件
‘EXAMPLE_DIRS’ 示例文件目录
‘EXCLUDE_FILES’ 扫描注释时排除的文件
‘EXCLUDE_DIRS’ 排除目录
‘DEPEND_QCHS’ 依赖的 qch 文件， 链接指定模块的文档
‘DEPEND_DIRS’ 一般与DEPEND_QCHS一起使用， 如果 DEPEND_QCHS 指定的模块文档在默认目录下（/usr/share/qt5/doc/）,无需显式指定当前变量；
如果依赖的模块文档不在默认目录下，需要通过DEPEND_DIRS 指定依赖的文档目录， 然后通过DEPEND_QCHS指定模块即可
‘INDEX_FILES’ 文档索引文件，通过指定依赖的文档索引，来建立和文档的链接， 一般不和DEPEND_QCHS 一起使用。

#use example
qdoc_add_qch(ukui-_quick-core-doc
        NAME "ukui-quick-core"
        DOC_NAME ukui-quick-core
        VERSION ${UKUI_QUICK_VERSION}
        NAMESPACE org.ukui.
        QCH_INSTALL_DESTINATION "/usr/share/qt5/doc"
        SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        HEADER_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

::

  ukui_install_qch_export(
      TARGETS [<name> [<name2> [...]]]
      FILE <file>
      DESTINATION <dest>
      [COMPONENT <component>]
  )


'TARGETS' 指定要导出的文档目标
‘FILE’ 指定生成的 CMake 文件名
‘DESTINATION’ 指定 CMake 文件的安装目录
‘COMPONENT’ 指定安装规则关联的组件名称

Example usage:

  ukui_install_qch_export(
      TARGETS MyLib_QCH
      FILE MyLibQCHTargets.cmake
      DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/MyLib"
      COMPONENT Devel
  )

#]=======================================================================]

include(FeatureSummary)

if (TARGET Qt5::Core)
    set(QT_MAJOR_VERSION 5)
else ()
    set(QT_MAJOR_VERSION 6)
endif ()

function(ukui_qdoc_add_qch target_name)
    set(options VERBOSE)
    set(oneValueArgs NAME DOC_NAME VERSION NAMESPACE QCH_INSTALL_DESTINATION TAGFILE_INSTALL_DESTINATION CONFIG_TEMPLATE STYLE_FILE)
    set(multiValueArgs SOURCES SOURCE_DIRS HEADERS HEADER_DIRS INCLUDE_DIRS IMAGES IMAGE_DIRS EXAMPLES EXAMPLE_DIRS EXCLUDE_FILES EXCLUDE_DIRS DEPEND_QCHS DEPEND_DIRS INDEX_FILES)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})


    #---------------------------------------------------------
    # 检查必要参数
    #---------------------------------------------------------
    foreach (arg_name NAME DOC_NAME VERSION QCH_INSTALL_DESTINATION)
        if(NOT DEFINED ARGS_${arg_name})
            message(FATAL_ERROR "${arg_name} needs to be defined when calling ukui_qdoc_add_qch")
        endif()
    endforeach ()

    if(NOT DEFINED ARGS_SOURCE_DIRS AND NOT DEFINED ARGS_SOURCES)
        message(FATAL_ERROR "SOURCE_DIRS or SOURCES needs to be defined when calling ukui_qdoc_add_qch")
    endif()

    if(NOT DEFINED ARGS_HEADER_DIRS AND NOT DEFINED ARGS_HEADERS)
        message(FATAL_ERROR "HEADER_DIRS or HEADERS needs to be defined when calling ukui_qdoc_add_qch")
    endif()

    #---------------------------------------------------------
    # 查找qdoc qhelpgenerator 工具
    #---------------------------------------------------------
    if (QT_VERSION_MAJOR EQUAL 5)
        find_program(QDoc_EXECUTABLE NAMES qdoc qdoc-qt5)
        set_package_properties(QDoc_EXECUTABLE PROPERTIES
                TYPE REQUIRED
                PURPOSE "Needed for API dox QCH file generation"
                DESCRIPTION "Part of Qt5 tools"
        )
        find_program(QHelpGenerator_EXECUTABLE NAMES qhelpgenerator qhelpgenerator-qt5)
        set_package_properties(QHelpGenerator_EXECUTABLE PROPERTIES
                TYPE REQUIRED
                PURPOSE "Needed for API dox QCH file generation"
                DESCRIPTION "Part of Qt5 tools"
        )

    else()
        find_package(Qt6 COMPONENTS ToolsTools CONFIG REQUIRED)
        set_package_properties(Qt6ToolsTools PROPERTIES
                TYPE REQUIRED
                PURPOSE "Needed for API dox QCH file generation"
                DESCRIPTION "qhelpgenerator from Qt6 tools"
        )
        if(TARGET Qt6::qdoc)
            get_target_property(QDoc_EXECUTABLE Qt6::qdoc LOCATION)
        endif()
        if(TARGET Qt6::qhelpgenerator)
            get_target_property(QHelpGenerator_EXECUTABLE Qt6::qhelpgenerator LOCATION)
        endif()
    endif()

    set(missing_tools)
    if (NOT QDoc_EXECUTABLE AND NOT TARGET Qt6::qdoc)
        list(APPEND missing_tools "qdoc")
    endif()
    if (NOT QHelpGenerator_EXECUTABLE AND NOT TARGET Qt6::qhelpgenerator)
        list(APPEND missing_tools "qhelpgenerator")
    endif()

    if (missing_tools)
        message(WARNING "API dox QCH file will not be generated, tools missing: ${missing_tools}!")
    else()
        #---------------------------------------------------------
        # 配置模板文件参数
        #---------------------------------------------------------

        set(QDOC_PROJECT_NAME "${ARGS_NAME}")

        string(REPLACE "-" "" qhp_project_name "${ARGS_NAME}")
        string(REPLACE " " "" qhp_project_name "${qhp_project_name}")
        set(QDOC_QHP_PROJECT_NAME "${qhp_project_name}")

        if (DEFINED ARGS_DOC_NAME)
            string(TOLOWER "${ARGS_DOC_NAME}" doc_name)
            string(REGEX REPLACE "[ _-]" "" doc_name "${doc_name}")
        else()
            string(TOLOWER "${qhp_project_name}" doc_name)
            string(REGEX REPLACE "[ _-]" "" doc_name "${doc_name}")
        endif()

        set(output_dir "${CMAKE_CURRENT_BINARY_DIR}/doc/${doc_name}")
        set(QDOC_OUTPUT_DIR "${output_dir}")

        set(QDOC_NAME "${doc_name}")

        set(QDOC_PROJECT_VERSION ${ARGS_VERSION})
        set(QDOC_NAMESPACE ${ARGS_NAMESPACE})

        #qch 安装路径
        set(qch_install_destination ${ARGS_QCH_INSTALL_DESTINATION})
        #tagfile 安装路径
        if (DEFINED ARGS_TAGFILE_INSTALL_DESTINATION)
            set(tagfile_install_destination ${ARGS_TAGFILE_INSTALL_DESTINATION})
        else ()
            set(tagfile_install_destination "${qch_install_destination}/${doc_name}")
        endif ()
        #index 安装路径
        set(index_install_destination "${qch_install_destination}/${doc_name}")
        #其他模块引用当前index的路径
        set(index_depend_path "${qch_install_destination}")

        #tags
        set(qdoc_output_tags_file "${output_dir}/${doc_name}.tags")
        set(QDOC_TAGS_FILE "${qdoc_output_tags_file}")

        if (DEFINED ARGS_CONFIG_TEMPLATE)
            set(qdoc_config_template_file ${ARGS_CONFIG_TEMPLATE})
        else()
            set(qdoc_config_template_file "${UKUI_QUICK_CMAKE_EXTEND_DIR}/QDocConfig.qdocconf.in")
        endif()

        #style
        if (DEFINED ARGS_STYLE_FILE)
            set(qdoc_style_file ${ARGS_STYLE_FILE})
        else()
            set(qdoc_style_file "${UKUI_QUICK_CMAKE_EXTEND_STYLE_DIR}default.css")
        endif()
        set(QDOC_HTML_STYLE_FILE "${qdoc_style_file}")
        get_filename_component(STYLE_NAME "${qdoc_style_file}" NAME)

        set(qdoc_config_file "${CMAKE_CURRENT_BINARY_DIR}/${doc_name}.qdocconf")

        #sources source_dirs
        set(QDOC_SOURCE_DIRS)
        foreach(source_dir IN LISTS ARGS_SOURCE_DIRS)
            if (NOT IS_ABSOLUTE ${source_dir})
                set(source_dir "${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}")
            endif()
            set(QDOC_SOURCE_DIRS "${QDOC_SOURCE_DIRS} \\\n\"${source_dir}\"")
        endforeach()

        set(QDOC_SOURCES)
        foreach(source IN LISTS ARGS_SOURCES)
            if (NOT IS_ABSOLUTE ${source})
                set(source "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
            endif()
            list(APPEND sources "${source}")
        endforeach()
        foreach(source IN LISTS sources)
            set(QDOC_SOURCES "${QDOC_SOURCES} \\\n\"${source}\"")
        endforeach()

        #headers header_dirs
        set(QDOC_HEADER_DIRS)
        foreach(header_dir IN LISTS ARGS_HEADER_DIRS)
            if (NOT IS_ABSOLUTE ${header_dir})
                set(header_dir "${CMAKE_CURRENT_SOURCE_DIR}/${header_dir}")
            endif()
            set(QDOC_HEADER_DIRS "${QDOC_HEADER_DIRS} \\\n\"${header_dir}\"")
        endforeach()

        set(QDOC_HEADERS)
        foreach(header IN LISTS ARGS_HEADERS)
            if (NOT IS_ABSOLUTE ${header})
                set(header "${CMAKE_CURRENT_SOURCE_DIR}/${header}")
            endif()
            list(APPEND headers "${header}")
        endforeach()
        foreach(header IN LISTS headers)
            set(QDOC_HEADERS "${QDOC_HEADERS} \\\n\"${header}\"")
        endforeach()

        #include_dirs
        set(QDOC_INCLUDE_DIRS)
        foreach(include_dir IN LISTS ARGS_INCLUDE_DIRS)
            if (NOT IS_ABSOLUTE ${include_dir})
                set(include_dir "${CMAKE_CURRENT_SOURCE_DIR}/${include_dir}")
            endif()
            set(QDOC_INCLUDE_DIRS "${QDOC_INCLUDE_DIRS} \\\n\"${include_dir}\"")
        endforeach()

        #images
        set(QDOC_IMAGE_DIRS)
        foreach(image_dir IN LISTS ARGS_IMAGE_DIRS)
            if (NOT IS_ABSOLUTE ${image_dir})
                set(image_dir "${CMAKE_CURRENT_SOURCE_DIR}/${image_dir}")
            endif()
            set(QDOC_IMAGE_DIRS "${QDOC_IMAGE_DIRS} \\\n\"${image_dir}\"")
        endforeach()

        set(QDOC_IMAGES)
        foreach(image IN LISTS ARGS_IMAGES)
            if (NOT IS_ABSOLUTE ${image})
                set(image "${CMAKE_CURRENT_SOURCE_DIR}/${image}")
            endif()
            list(APPEND images "${image}")
        endforeach()
        foreach(image IN LISTS images)
            set(QDOC_IMAGES "${QDOC_IMAGES} \\\n\"${image}\"")
        endforeach()

        #example_dirs examples 示例目录
        set(QDOC_EXAMPLE_DIRS)
        foreach(example_dir IN LISTS ARGS_EXAMPLE_DIRS)
            if (NOT IS_ABSOLUTE ${example_dir})
                set(example_dir "${CMAKE_CURRENT_SOURCE_DIR}/${example_dir}")
            endif()
            set(QDOC_EXAMPLE_DIRS "${QDOC_EXAMPLE_DIRS} \\\n\"${example_dir}\"")
        endforeach()

        set(QDOC_EXAMPLES)
        foreach(example IN LISTS ARGS_EXAMPLES)
            if (NOT IS_ABSOLUTE ${example})
                set(example "${CMAKE_CURRENT_SOURCE_DIR}/${example}")
            endif()
            list(APPEND examples "${example}")
        endforeach()
        foreach(example IN LISTS examples)
            set(QDOC_EXAMPLES "${QDOC_EXAMPLES} \\\n\"${example}\"")
        endforeach()

        #exclude_files exclude_dirs 需要排除的文件
        set(QDOC_EXCLUDE_DIRS)
        foreach(exclude_dir IN LISTS ARGS_EXCLUDE_DIRS)
            if (NOT IS_ABSOLUTE ${exclude_dir})
                set(exclude_dir "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_dir}")
            endif()
            set(QDOC_EXCLUDE_DIRS "${QDOC_EXCLUDE_DIRS} \\\n\"${exclude_dir}\"")
        endforeach()

        set(QDOC_EXCLUDE_FILES)
        foreach(exclude_file IN LISTS ARGS_EXCLUDE_FILES)
            if (NOT IS_ABSOLUTE ${exclude_file})
                set(exclude_file "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
            endif()
            list(APPEND exclude_files "${exclude_file}")
        endforeach()
        foreach(exclude_file IN LISTS exclude_files)
            set(QDOC_EXCLUDE_FILES "${QDOC_EXCLUDE_FILES} \\\n\"${exclude_file}\"")
        endforeach()

        #depends_qchs
        set(QDOC_DEPEND_QCHS)
        foreach(depend_qch IN LISTS ARGS_DEPEND_QCHS)
            set(QDOC_DEPEND_QCHS "${QDOC_DEPEND_QCHS} \\\n${depend_qch}")
        endforeach()

        #depend_dirs  依赖的目录, 如果依赖文档没有安装在默认目录（/usr/share/qt5/doc/），通过当前变量指定
        set(depend_dirs)
        #增加默认的依赖目录
        list(APPEND ARGS_DEPEND_DIRS ${qch_install_destination})
        #去重
        list(REMOVE_DUPLICATES ARGS_DEPEND_DIRS)
        foreach(depend_dir IN LISTS ARGS_DEPEND_DIRS)
            if (NOT IS_ABSOLUTE ${depend_dir})
                set(depend_dir "${CMAKE_CURRENT_SOURCE_DIR}/${depend_dir}")
            endif()
            list(APPEND depend_dirs "-indexdir" "${depend_dir}")
        endforeach()

        set(QDOC_INDEX_FILES)
        foreach(index_file IN LISTS ARGS_INDEX_FILES)
            set(QDOC_INDEX_FILES "${QDOC_INDEX_FILES} \\\n${index_file}")
        endforeach()
        #----------------------------------
        # finally create qdoc config file
        configure_file(
                "${qdoc_config_template_file}"
                "${qdoc_config_file}"
                @ONLY
        )

        set(qdoc_output_qhp_file "${output_dir}/${doc_name}.qhp")
        set(qdoc_output_qch_file "${output_dir}/${doc_name}.qch")
        set(qdoc_output_index_file "${output_dir}/${doc_name}.index")

        #----------------------------------
        # 文档相关的输入文件，确保注释变更能触发重建
        set(qdoc_dependency_files)
        list(APPEND qdoc_dependency_files
                ${sources}
                ${headers}
                ${images}
                ${examples}
                ${ARGS_INDEX_FILES}
                ${qdoc_config_template_file}
                ${qdoc_style_file}
        )

        set(qdoc_scan_dirs
                ${ARGS_SOURCE_DIRS}
                ${ARGS_HEADER_DIRS}
                ${ARGS_IMAGE_DIRS}
                ${ARGS_EXAMPLE_DIRS}
        )
        list(REMOVE_DUPLICATES qdoc_scan_dirs)

        foreach(scan_dir IN LISTS qdoc_scan_dirs)
            if (NOT IS_ABSOLUTE ${scan_dir})
                set(scan_dir "${CMAKE_CURRENT_SOURCE_DIR}/${scan_dir}")
            endif()
            if (EXISTS "${scan_dir}")
                # Avoid CONFIGURE_DEPENDS to keep configuration fast in large projects.
                # New files in scanned directories are picked up on the next CMake configure run.
                file(GLOB_RECURSE scan_dir_files LIST_DIRECTORIES FALSE
                        "${scan_dir}/*.h"
                        "${scan_dir}/*.hh"
                        "${scan_dir}/*.hpp"
                        "${scan_dir}/*.hxx"
                        "${scan_dir}/*.c"
                        "${scan_dir}/*.cc"
                        "${scan_dir}/*.cpp"
                        "${scan_dir}/*.cxx"
                        "${scan_dir}/*.qml"
                        "${scan_dir}/*.js"
                        "${scan_dir}/*.qdoc"
                        "${scan_dir}/*.qdocinc"
                        "${scan_dir}/*.md"
                        "${scan_dir}/*.png"
                        "${scan_dir}/*.jpg"
                        "${scan_dir}/*.jpeg"
                        "${scan_dir}/*.gif"
                        "${scan_dir}/*.svg"
                        "${scan_dir}/*.webp"
                        "${scan_dir}/*.bmp"
                        "${scan_dir}/*.ico"
                )
                list(APPEND qdoc_dependency_files ${scan_dir_files})
            endif()
        endforeach()

        # 排除明确指定的文件
        foreach(exclude_file IN LISTS ARGS_EXCLUDE_FILES)
            if (NOT IS_ABSOLUTE ${exclude_file})
                set(exclude_file "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
            endif()
            list(REMOVE_ITEM qdoc_dependency_files "${exclude_file}")
        endforeach()

        # 排除明确指定目录下的文件
        set(qdoc_exclude_dirs_abs)
        foreach(exclude_dir IN LISTS ARGS_EXCLUDE_DIRS)
            if (NOT IS_ABSOLUTE ${exclude_dir})
                set(exclude_dir "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_dir}")
            endif()
            string(REGEX REPLACE "/+$" "" exclude_dir "${exclude_dir}")
            list(APPEND qdoc_exclude_dirs_abs "${exclude_dir}")
        endforeach()

        set(qdoc_dependency_files_filtered)
        foreach(input_file IN LISTS qdoc_dependency_files)
            set(skip_file FALSE)
            foreach(exclude_dir IN LISTS qdoc_exclude_dirs_abs)
                if (input_file STREQUAL "${exclude_dir}")
                    set(skip_file TRUE)
                    break()
                endif()
                string(FIND "${input_file}" "${exclude_dir}/" exclude_pos)
                if (exclude_pos EQUAL 0)
                    set(skip_file TRUE)
                    break()
                endif()
            endforeach()
            if (NOT skip_file)
                list(APPEND qdoc_dependency_files_filtered "${input_file}")
            endif()
        endforeach()
        set(qdoc_dependency_files "${qdoc_dependency_files_filtered}")
        list(REMOVE_DUPLICATES qdoc_dependency_files)

        # 开启VERBOSE输出日志到终端，默认输出到文件
        if(ARGS_VERBOSE)
            set(qdoc_log_command)
            set(qhelp_log_command)
        else()
            set(log_file "${CMAKE_CURRENT_BINARY_DIR}/qdoc_log.log")
            set(qdoc_log_command ">" ${log_file} "2>&1")
            set(qhelp_log_command ">>" ${log_file} "2>&1")
        endif()

        add_custom_command(
                OUTPUT ${qdoc_output_qhp_file} ${qdoc_output_qch_file}
                COMMENT "Generating ${_relative_qch_file}, ${_relative_tags_file}"
                COMMAND cmake -E remove_directory "${output_dir}"
                COMMAND cmake -E make_directory "${output_dir}"
                COMMAND ${QDoc_EXECUTABLE} "${qdoc_config_file}" ${depend_dirs} ${qdoc_log_command}
                COMMAND ${QHelpGenerator_EXECUTABLE} "${qdoc_output_qhp_file}" -o "${qdoc_output_qch_file}" ${qhelp_log_command}
                DEPENDS
                ${qdoc_config_file}
                ${qdoc_dependency_files}
        )
        add_custom_target(${target_name} ALL DEPENDS ${qdoc_output_qhp_file} ${qdoc_output_qch_file})
        #设置被引用的模块名和安装路径
        set_target_properties(${target_name} PROPERTIES
                DEPEND_QCH_MODULE ${doc_name}
                DEPEND_QCH_PATH ${index_depend_path}
                DEPEND_QCH_INDEX "${index_install_destination}/${doc_name}.index"
        )
        # setup installation
        install(FILES
                ${qdoc_output_qch_file}
                DESTINATION ${qch_install_destination}
        )

        install(FILES
                ${qdoc_output_tags_file}
                DESTINATION ${tagfile_install_destination}
        )

        install(FILES
                ${qdoc_output_index_file}
                DESTINATION ${index_install_destination}
        )

    endif ()
endfunction()

#  Imitation  ECM ecm_install_qch_export
function(ukui_install_qch_export)
    set(options )
    set(oneValueArgs FILE DESTINATION COMPONENT)
    set(multiValueArgs TARGETS)

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED ARGS_FILE)
        message(FATAL_ERROR "FILE needs to be defined when calling ukui_qdoc_qch_export().")
    endif()

    if(NOT DEFINED ARGS_DESTINATION)
        message(FATAL_ERROR "DESTINATION needs to be defined when calling ukui_qdoc_qch_export().")
    endif()

    set(_content
            "# This file was generated by ukui_install_qch_export(). DO NOT EDIT!
"
    )

    foreach(_target IN LISTS ARGS_TARGETS)
        set(_target_usable TRUE)

        if (NOT TARGET ${_target})
            message(STATUS "No such target ${_target} when calling ukui_install_qch_export().")
            set(_target_usable FALSE)
        else()
            get_target_property(depend_qch_module ${_target} DEPEND_QCH_MODULE)
            get_target_property(depend_qch_path ${_target} DEPEND_QCH_PATH)
            get_target_property(depend_qch_index ${_target} DEPEND_QCH_INDEX)
            if(NOT depend_qch_module)
                message(STATUS "No depend_qch_module for ${_target}.")
                set(_target_usable FALSE)
            endif()
            if(NOT depend_qch_path)
                message(STATUS "No depend_qch_path for ${_target}.")
                set(_target_usable FALSE)
            endif()
            if(NOT depend_qch_index)
                message(STATUS "No depend_qch_index for ${_target}.")
                set(_target_usable FALSE)
            endif()
        endif()

        if(_target_usable)
            set(_content "${_content}
if (NOT TARGET ${_target})

add_custom_target(${_target})
set_target_properties(${_target} PROPERTIES
    DEPEND_QCH_MODULE \"${depend_qch_module}\"
    DEPEND_QCH_PATH \"${depend_qch_path}\"
    DEPEND_QCH_INDEX \"${depend_qch_index}\"
)

endif()
"
            )
        else()
            message(STATUS "No target exported for ${_target}.")
        endif()
    endforeach()

    if (NOT IS_ABSOLUTE ${ARGS_FILE})
        set(ARGS_FILE "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_FILE}")
    endif()

    file(GENERATE
            OUTPUT "${ARGS_FILE}"
            CONTENT "${_content}"
    )

    if (DEFINED ARGS_COMPONENT)
        set(_component COMPONENT ${ARGS_COMPONENT})
    else()
        set(_component)
    endif()
    install(
            FILES "${ARGS_FILE}"
            DESTINATION "${ARGS_DESTINATION}"
            ${_component}
    )

endfunction()
