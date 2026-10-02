find_package(PkgConfig REQUIRED)
find_package(KylinAiEngine REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

option(ENABLE_ENGINE_INTEGRATION_TEST "Build engine integration tests (requires plugins/credentials)" OFF)

function(link_gtest_main target_name)
    if (TARGET GTest::gtest_main)
        target_link_libraries(${target_name} GTest::gtest_main Threads::Threads)
    else ()
        target_include_directories(${target_name} PRIVATE ${GTEST_INCLUDE_DIRS})
        target_link_libraries(${target_name}
            ${GTEST_MAIN_LIBRARIES}
            ${GTEST_LIBRARIES}
            Threads::Threads
        )
    endif ()
endfunction()

set(encryption_SOURCES
    ../src/encryption/rsa2048.cpp
    ../src/encryption/pbkdf2.cpp
    ../src/encryption/aes256.cpp
    ../src/encryption/encryption.cpp
)

set(cpr_SOURCES
    ../src/services/threadpool/singleton.h
    ../src/services/threadpool/async.h
    ../src/services/threadpool/async.cpp
    ../src/services/threadpool/threadpool.cpp
)

set(engine_SOURCES
    ../src/engine/aienginepluginmanager.cpp
    ../src/utils/library.cpp
)

set(config_SOURCES
    ../src/config/engineconfiguration.cpp
    ../src/config/prompttemplate.cpp
)

set(lifecycle_SOURCES
    ../src/lifecycle/modellifecycleconfig.cpp
    ../src/lifecycle/modelruntimecontext.cpp
    ../src/lifecycle/modelusagehandle.cpp
    ../src/lifecycle/modellifecyclemanager.cpp
    ../src/lifecycle/enginestopcontroller.cpp
    ../src/lifecycle/modelresourcecontroller.cpp
    ../src/lifecycle/modelidlesweeper.cpp
    ../src/lifecycle/lifecycleawareengine.cpp
)

add_executable(testencryption
    testencryption.cpp
    ${encryption_SOURCES}
)
target_link_libraries(testencryption crypto ${OPENSSL_LIBRARIES})
link_gtest_main(testencryption)
add_test(NAME testencryption COMMAND testencryption)

add_executable(testaes256
    testaes256.cpp
    ../src/encryption/aes256.cpp
)
include_directories(../src/encryption)
target_link_libraries(testaes256 crypto ${OPENSSL_LIBRARIES})
link_gtest_main(testaes256)
add_test(NAME testaes256 COMMAND testaes256)

add_executable(testpbkdf2
    testpbkdf2.cpp
    ../src/encryption/pbkdf2.cpp
)
target_include_directories(testpbkdf2 PRIVATE ../src/encryption)
target_link_libraries(testpbkdf2 crypto ${OPENSSL_LIBRARIES})
link_gtest_main(testpbkdf2)
add_test(NAME testpbkdf2 COMMAND testpbkdf2)

add_executable(testrsa2048
    testrsa2048.cpp
    ../src/encryption/rsa2048.cpp
)
target_include_directories(testrsa2048 PRIVATE ../src/encryption)
target_link_libraries(testrsa2048 crypto ${OPENSSL_LIBRARIES})
link_gtest_main(testrsa2048)
add_test(NAME testrsa2048 COMMAND testrsa2048)

include(../cmake/engine-plugin-path-defs.cmake)
include_directories(../ai-engine)
configure_file(16k.pcm ${CMAKE_CURRENT_BINARY_DIR}/16k.pcm COPYONLY)
if (ENABLE_ENGINE_INTEGRATION_TEST)
    add_executable(testenginemanager
        ../src/engine/aienginepluginmanager.cpp
        ../src/utils/library.cpp
        ../src/utils/logger.cpp
        testenginemanager.cpp
    )
    target_link_libraries(testenginemanager dl ${OPENSSL_LIBRARIES} ${GIO_LIBRARIES})
    link_gtest_main(testenginemanager)
    add_test(NAME testenginemanager COMMAND testenginemanager)
endif ()

add_executable(testmodelstatustracker
    ../src/services/modelconfig/modelstatustracker.cpp
    testmodelstatustracker.cpp
)
add_test(NAME testmodelstatustracker COMMAND testmodelstatustracker)
target_link_libraries(testmodelstatustracker pthread)
link_gtest_main(testmodelstatustracker)

add_executable(testtaskrunner
    ../src/taskmanager/taskrunner.cpp
    testtaskrunner.cpp
)
target_compile_definitions(testtaskrunner PRIVATE TASKRUNNER_TESTING)
add_test(NAME testtaskrunner COMMAND testtaskrunner)
if (TARGET GTest::gtest_main)
    target_link_libraries(testtaskrunner
        pthread
        GTest::gtest_main
        jsoncpp
        spdlog::spdlog
    )
else ()
    target_include_directories(testtaskrunner PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(testtaskrunner
        pthread
        ${GTEST_MAIN_LIBRARIES}
        ${GTEST_LIBRARIES}
        jsoncpp
        spdlog::spdlog
    )
endif ()

include_directories(${JSON_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS})
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(PULSE REQUIRED libpulse libpulse-simple)

add_executable(testchatbot
    ../src/services/genai/nlp/chatbot.cpp
    ../src/services/genai/nlp/chatmessages.cpp
    ../src/taskmanager/taskrunner.cpp
    ../src/services/modelconfig/modelstatustracker.cpp
    ../src/services/modelconfig/modelconfigmanager.cpp
    ../src/services/modelconfig/modelsettings.cpp
    ../src/services/modelconfig/modelchangedcallbacks.cpp
    ../src/engineselector/engineselector.cpp
    ${engine_SOURCES}
    ${config_SOURCES}
    ${encryption_SOURCES}
    ${lifecycle_SOURCES}
    ../src/config/fileutil.cpp
    ../src/utils/logger.cpp
    ../src/utils/util.cpp
    testchatbot.cpp
)
target_include_directories(testchatbot PRIVATE ../src)
target_include_directories(testchatbot PRIVATE ${GIO_INCLUDE_DIRS})
target_compile_definitions(testchatbot PRIVATE TASKRUNNER_TESTING)
add_test(NAME testchatbot COMMAND testchatbot)
if (TARGET GTest::gtest_main)
    target_link_libraries(testchatbot
        pthread
        GTest::gtest
        jsoncpp
        spdlog::spdlog
        ${OPENSSL_LIBRARIES}
        ${GIO_LIBRARIES}
        ${ICU_UC_LIBRARIES}
        dl
        httpclient
        ${TRITONCLIENT_LIB_DIRS}
    )
else ()
    target_include_directories(testchatbot PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(testchatbot
        pthread
        ${GTEST_LIBRARIES}
        jsoncpp
        spdlog::spdlog
        ${OPENSSL_LIBRARIES}
        ${GIO_LIBRARIES}
        ${ICU_UC_LIBRARIES}
        dl
        httpclient
        ${TRITONCLIENT_LIB_DIRS}
    )
endif ()


set(testmodules_common_sources
    ../src/services/genai/vision/imagegenerator.cpp
    ../src/services/coreai/vision/textrecognition.cpp
    ../src/services/coreai/vision/subjectsegmentation.cpp
    ../src/services/coreai/speech/speechrecognizer.cpp
    ../src/services/coreai/speech/speechsynthesizer.cpp
    ../src/services/coreai/speech/audioconfiginfo.cpp
    ../src/taskmanager/taskrunner.cpp
    ../src/services/modelconfig/modelstatustracker.cpp
    ../src/services/modelconfig/modelconfigmanager.cpp
    ../src/services/modelconfig/modelsettings.cpp
    ../src/services/modelconfig/modelchangedcallbacks.cpp
    ../src/engineselector/engineselector.cpp
    ${engine_SOURCES}
    ${config_SOURCES}
    ${encryption_SOURCES}
    ${lifecycle_SOURCES}
    ../src/config/fileutil.cpp
    ../src/utils/logger.cpp
    ../src/utils/util.cpp
    ../src/utils/audiorecorder.cpp
    ../src/utils/audioplayer.cpp
)

function(add_testmodules_target target_name source_file)
    add_executable(${target_name}
        ${source_file}
        ${testmodules_common_sources}
    )
    target_include_directories(${target_name} PRIVATE ../src)
    target_include_directories(${target_name} PRIVATE ${GIO_INCLUDE_DIRS})
    target_include_directories(${target_name} PRIVATE ${PULSE_INCLUDE_DIRS})
    target_compile_definitions(${target_name} PRIVATE TASKRUNNER_TESTING)
    add_test(NAME ${target_name} COMMAND ${target_name})
    if (TARGET GTest::gtest_main)
        target_link_libraries(${target_name}
            pthread
            GTest::gtest_main
            jsoncpp
            spdlog::spdlog
            ${OPENSSL_LIBRARIES}
            ${GIO_LIBRARIES}
            ${PULSE_LIBRARIES}
            ${ICU_UC_LIBRARIES}
            httpclient
            ${TRITONCLIENT_LIB_DIRS}
            dl
        )
    else ()
        target_include_directories(${target_name} PRIVATE ${GTEST_INCLUDE_DIRS})
        target_link_libraries(${target_name}
            pthread
            ${GTEST_MAIN_LIBRARIES}
            ${GTEST_LIBRARIES}
            jsoncpp
            spdlog::spdlog
            ${OPENSSL_LIBRARIES}
            ${GIO_LIBRARIES}
            ${PULSE_LIBRARIES}
            ${ICU_UC_LIBRARIES}
            httpclient
            ${TRITONCLIENT_LIB_DIRS}
            dl
        )
    endif ()
endfunction()

add_testmodules_target(testimagegenerator testimagegenerator.cpp)
add_testmodules_target(testtextrecognition testtextrecognition.cpp)
add_testmodules_target(testsubjectsegmentation testsubjectsegmentation.cpp)
add_testmodules_target(testspeechrecognizer testspeechrecognizer.cpp)
add_testmodules_target(testspeechsynthesizer testspeechsynthesizer.cpp)
add_executable(testsystemoperation
    ../src/services/assistant/intentdialoguemanager/systemoperation/systemoperation.cpp
    ../src/services/assistant/intentdialoguemanager/systemoperation/systemoperation.h
    ../src/services/assistant/intentdialoguemanager/systemoperation/systemoperationinterface.h
    ../src/services/assistant/message.cpp
    ../src/services/assistant/message.h
    ../src/services/assistant/error.cpp
    ../src/services/assistant/error.h
    ../src/APIs/apiplatform.h
    ../src/APIs/apiplatform.cpp
    ../src/APIs/systemapi.cpp
    ../src/APIs/systemapi.h
    ../src/APIs/systemapidbusmap.cpp
    ../src/APIs/systemapidbusmap.h
    ../src/applicationInfos/applicationinfos.h
    ../src/applicationInfos/applicationinfos.cpp
    ../src/utils/logger.cpp
    ../src/utils/logger.h
    ../src/utils/util.h
    ../src/utils/util.cpp
    testsystemoperation.cpp
)
add_test(NAME testsystemoperation COMMAND testsystemoperation)
target_link_libraries(testsystemoperation
    pthread
    ${JSON_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${GIO_LIBRARIES}
    spdlog::spdlog
    jsoncpp
    ${ICU_UC_LIBRARIES}
    ${OpenCV_LIBS}
)
link_gtest_main(testsystemoperation)


add_executable(testmodellifecyclemanager
    testmodellifecyclemanager.cpp
    ../src/lifecycle/modellifecycleconfig.cpp
    ../src/lifecycle/modelruntimecontext.cpp
    ../src/lifecycle/modelusagehandle.cpp
    ../src/lifecycle/modellifecyclemanager.cpp
    ../src/lifecycle/enginestopcontroller.cpp
    ../src/lifecycle/modelresourcecontroller.cpp
    ../src/lifecycle/modelidlesweeper.cpp
)
target_include_directories(testmodellifecyclemanager PRIVATE ../src)
target_link_libraries(testmodellifecyclemanager
    pthread
    spdlog::spdlog
    jsoncpp
    ${ICU_UC_LIBRARIES}
    httpclient
    ${TRITONCLIENT_LIB_DIRS}
)
link_gtest_main(testmodellifecyclemanager)
if (TARGET GTest::gtest)
    get_target_property(GTEST_LIB_PATH GTest::gtest IMPORTED_LOCATION)
    if (NOT GTEST_LIB_PATH)
        get_target_property(GTEST_LIB_PATH GTest::gtest IMPORTED_LOCATION_RELEASE)
    endif ()
endif ()
if (GTEST_LIB_PATH)
    get_filename_component(GTEST_LIB_DIR ${GTEST_LIB_PATH} DIRECTORY)
    set_property(TARGET testmodellifecyclemanager APPEND PROPERTY BUILD_RPATH ${GTEST_LIB_DIR})
    set_property(TARGET testmodellifecyclemanager APPEND PROPERTY INSTALL_RPATH ${GTEST_LIB_DIR})
endif ()
add_test(NAME testmodellifecyclemanager COMMAND testmodellifecyclemanager)
if (GTEST_LIB_DIR)
    set(GTEST_RUNTIME_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/gtest-libs)
    file(MAKE_DIRECTORY ${GTEST_RUNTIME_LIB_DIR})
    file(GLOB GTEST_SHARED_LIBS "${GTEST_LIB_DIR}/libgtest*.so*")
    if (GTEST_SHARED_LIBS)
        file(COPY ${GTEST_SHARED_LIBS} DESTINATION ${GTEST_RUNTIME_LIB_DIR})
        set_property(TEST testmodellifecyclemanager APPEND PROPERTY ENVIRONMENT
            "LD_LIBRARY_PATH=${GTEST_RUNTIME_LIB_DIR}:$ENV{LD_LIBRARY_PATH}")
    endif ()
endif ()
