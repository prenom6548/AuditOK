function (kylin_ai_add_engine_plugin PROJECT_NAME ENGINE_TYPE)
    if (${ENGINE_TYPE} STREQUAL AI_ENGINE)
        set(PLUGIN_PATH kylin-ai/plugins/ai-engines)
    elseif(${ENGINE_TYPE} STREQUAL VECTORDB_ENGINE)
        set(PLUGIN_PATH  kylin-ai/plugins/db-engines/vector-database)
    endif()

    add_library(${PROJECT_NAME} SHARED ${ARGN})
    target_link_libraries(${PROJECT_NAME} KylinAiEngine::KylinAiEngine)
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME} DESTINATION
        "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${PLUGIN_PATH}")

endfunction()