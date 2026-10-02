cmake_minimum_required(VERSION 3.5)

project(kylin-ai-runtime LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(RUN_IN_DEBUG)
    add_compile_definitions(CONFIGS_PATH="${CMAKE_SOURCE_DIR}/configs/etc/kylin-ai")
else ()
    add_compile_definitions(RUN_IN_RELEASE)
    add_compile_definitions(CONFIGS_PATH="/etc/kylin-ai")
endif ()

# 依据系统版本选择RUN_IN_V101_2503 or RUN_IN_V11_2603，本分支为V11_2603
add_compile_definitions(RUN_IN_V11_2603)

find_package(KylinAiEngine REQUIRED)
find_package(KysdkAiCommon REQUIRED)
find_package(KylinAiProto REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(spdlog REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(CURL REQUIRED)
find_package(ICU COMPONENTS uc REQUIRED)
find_package(OpenCV REQUIRED)
set(PYBIND11_PYTHON_VERSION 3)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(LIBPULSE REQUIRED IMPORTED_TARGET libpulse)
pkg_check_modules(SODIUM REQUIRED libsodium)

include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${SODIUM_INCLUDE_DIR})
include_directories(${Python3_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third-party)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/services)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)


kylin_ai_generate_gdbus_proto_code(PROTO_FILES corespeechservice configservice assistantservice
    gennlpservice genaivisionservice coretextembeddingservice coreimageembeddingservice corevisionservice advaiknowledgebaseservice)

include(GNUInstallDirs)
include(cmake/engine-plugin-path-defs.cmake)

# 获取系统处理器架构
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|aarch64)$")
    # amd64或arm64
    add_compile_definitions(SUPPORT_VECTOR_DB)
    message(STATUS "Building for x86_64 or aarch64 architecture: ${CMAKE_SYSTEM_PROCESSOR}.")
    pkg_check_modules(VECTOR_DB REQUIRED IMPORTED_TARGET kysdk-vector-engine-client)
    pkg_check_modules(GRPCPP REQUIRED IMPORTED_TARGET grpc++)
    pkg_check_modules(KYAI_KNOWLEDGE_BASE_CLIENT REQUIRED kyai-knowledgebase-client)
    include_directories(${VECTOR_DB_INCLUDE_DIRS})
    include_directories(${KYAI_KNOWLEDGE_BASE_CLIENT_INCLUDE_DIRS})
    set(VECTOR_DB_SOURCES
        src/services/assistant/intentdialoguemanager/fileinference/fileinference.h
        src/services/assistant/intentdialoguemanager/fileinference/fileinference.cpp
        src/services/assistant/intentdialoguemanager/textembedding/textembedding.h
        src/services/assistant/intentdialoguemanager/textembedding/textembedding.cpp
        src/services/assistant/intentdialoguemanager/slotsnormalization.h
        src/services/assistant/intentdialoguemanager/slotsnormalization.cpp
        src/services/assistant/rag/ragserverproxy.h
        src/services/assistant/rag/ragserverproxy.cpp
        src/services/assistant/knowledgebase/knowledgebaseproxy.h
        src/services/assistant/knowledgebase/knowledgebaseproxy.cpp
        src/services/assistant/intentdialoguemanager/intentrecognizer/intentslotrecognizer.h
        src/services/assistant/intentdialoguemanager/intentrecognizer/intentslotrecognizer.cpp
        src/services/assistant/intentdialoguemanager/intentrecognizer/intentslotrules.h
        src/services/assistant/intentdialoguemanager/intentrecognizer/intentslotrules.cpp
        src/services/assistant/intentdialoguemanager/intentrecognizer/intenttypes.h
        src/services/assistant/intentdialoguemanager/intentrecognizer/rulebasedintentretriever.h
        src/services/assistant/intentdialoguemanager/intentrecognizer/rulebasedintentretriever.cpp
        src/services/assistant/intentdialoguemanager/intentrecognizer/vectordatabase.h
        src/services/assistant/intentdialoguemanager/intentrecognizer/vectordatabase.cpp
        src/services/assistant/intentdialoguemanager/intentrecognizer/slotextractor.h
        src/services/assistant/intentdialoguemanager/intentrecognizer/slotextractor.cpp
    )
    add_compile_definitions(SUPPORT_LIFE_CYCLE)
    find_package(TritonClient REQUIRED)
    include_directories(${TRITONCLIENT_INCLUDE_DIRS})
    set(LIFE_CYCLE_SOURCES
        src/lifecycle/modellifecycleconfig.cpp
        src/lifecycle/modelruntimecontext.cpp
        src/lifecycle/modelusagehandle.cpp
        src/lifecycle/modellifecyclemanager.cpp
        src/lifecycle/modelidlesweeper.cpp
        src/lifecycle/powereventmonitor.cpp
        src/lifecycle/lifecycleawareengine.cpp
        src/lifecycle/enginestopcontroller.cpp
        src/lifecycle/modelresourcecontroller.cpp
    )
endif ()

set(CPR_THREADPOOL_SOURCES
    src/threadpool/singleton.h
    src/threadpool/async.h
    src/threadpool/async.cpp
    src/threadpool/threadpool.h
    src/threadpool/threadpool.cpp
)

add_executable(kylin-ai-runtime
    main.cpp
    src/server/abstractserver.cpp
    src/server/abstractserver.h
    src/server/assistantserver.h
    src/server/assistantserver.cpp
    src/server/coreaispeechserver.cpp
    src/server/corevisionserver.cpp
    src/server/configserver.cpp
    src/server/genainlpserver.cpp
    src/server/genaivisionserver.cpp
    src/server/servermanager.cpp
    src/server/coretextembeddingserver.h
    src/server/coretextembeddingserver.cpp
    src/server/coreimageembeddingserver.h
    src/server/coreimageembeddingserver.cpp
    src/services/coreai/speech/audioconfiginfo.cpp
    src/services/coreai/speech/coreaispeechservice.cpp
    src/services/coreai/speech/speechrecognizer.cpp
    src/services/coreai/speech/speechsynthesizer.cpp
    src/services/coreai/vision/corevisionservice.cpp
    src/services/coreai/vision/segmentationsession.h
    src/services/coreai/vision/segmentationsession.cpp
    src/services/coreai/vision/textrecognition.cpp
    src/services/coreai/vision/subjectsegmentation.cpp
    src/services/coreai/vision/subjectsegmentation.h
    src/services/coreai/vision/textrecognition.h
    src/services/coreai/vision/vision.h
    src/services/coreai/vision/coreaivisionmodelconfig.h
    src/services/coreai/speech/coreaispeechmodelconfig.h
    src/services/genai/nlp/genainlpservice.cpp
    src/services/genai/nlp/genainlpmodelconfig.h
    src/services/coreai/textembedding/coretextembeddingservice.cpp
    src/services/coreai/textembedding/coretextembeddingservice.h
    src/services/coreai/textembedding/textembedding.cpp
    src/services/coreai/textembedding/textembedding.h
    src/services/coreai/imageembedding/coreimageembeddingservice.cpp
    src/services/coreai/imageembedding/coreimageembeddingservice.h
    src/services/coreai/imageembedding/imageembedding.cpp
    src/services/coreai/imageembedding/imageembedding.h
    src/services/genai/nlp/chatbot.cpp
    src/services/genai/nlp/chatmessages.cpp
    src/services/genai/vision/genaivisionservice.cpp
    src/services/genai/vision/imagegenerationrequest.cpp
    src/services/genai/vision/imagegenerator.cpp
    src/services/genai/vision/vision.h
    src/services/modelconfig/configservice.cpp
    src/services/modelconfig/modelchangedcallbacks.cpp
    src/services/modelconfig/modelconfigmanager.cpp
    src/services/modelconfig/modelsettings.cpp
    src/services/modelconfig/modelstatustracker.cpp
    src/engine/aienginepluginmanager.h
    src/engine/aienginepluginmanager.cpp
    src/config/engineconfiguration.h
    src/config/engineconfiguration.cpp
    src/config/prompttemplate.h
    src/config/prompttemplate.cpp
    src/config/fileutil.h
    src/config/fileutil.cpp
    src/taskmanager/taskrunner.h
    src/taskmanager/taskrunner.cpp
    ${CPR_THREADPOOL_SOURCES}
    ${VECTOR_DB_SOURCES}
    ${LIFE_CYCLE_SOURCES}

    src/engineselector/engineselector.cpp

    src/applicationInfos/applicationinfos.h
    src/applicationInfos/applicationinfos.cpp
    src/APIs/systemapi.h
    src/APIs/systemapi.cpp
    src/APIs/complexsystemapi.h
    src/APIs/complexsystemapi.cpp
    src/APIs/apiplatform.h
    src/APIs/apiplatform.cpp
    src/APIs/systemapidbusmap.h
    src/APIs/systemapidbusmap.cpp
    src/services/assistant/assistantservice.h
    src/services/assistant/assistantservice.cpp
    src/services/assistant/message.h
    src/services/assistant/message.cpp
    src/services/assistant/handlers/requesthandler.h
    src/services/assistant/handlers/requesthandler.cpp
    src/services/assistant/handlers/chat/chatrequesthandler.cpp
    src/services/assistant/handlers/requesthandlermanager.h
    src/services/assistant/handlers/requesthandlermanager.cpp

    src/services/assistant/intentdialoguemanager/fileinference/fileinferenceinterface.h
    src/services/assistant/intentdialoguemanager/textembedding/textembeddinginterface.h
    src/services/assistant/intentdialoguemanager/modelinference/modelinferenceinterface.h
    src/services/assistant/intentdialoguemanager/modelinference/modelinference.h
    src/services/assistant/intentdialoguemanager/modelinference/modelinference.cpp
    src/services/assistant/intentdialoguemanager/imagehandler/imagehandlerinterface.h
    src/services/assistant/intentdialoguemanager/imagehandler/imagehandler.h
    src/services/assistant/intentdialoguemanager/imagehandler/imagehandler.cpp
    src/services/assistant/intentdialoguemanager/systemoperation/systemoperationinterface.h
    src/services/assistant/intentdialoguemanager/systemoperation/systemoperation.h
    src/services/assistant/intentdialoguemanager/systemoperation/systemoperation.cpp
    src/services/assistant/intentdialoguemanager/systemoperation/ctrl.h
    src/services/assistant/intentdialoguemanager/systemoperation/ctrl.cpp
    src/services/assistant/intentdialoguemanager/systemoperation/startuptools.h
    src/services/assistant/intentdialoguemanager/systemoperation/startuptools.cpp
    src/services/assistant/intentdialoguemanager/imagegentrigger.h
    src/services/assistant/intentdialoguemanager/imagegentrigger.cpp
    src/services/assistant/intentdialoguemanager/intentionrecognition.h
    src/services/assistant/intentdialoguemanager/intentionrecognition.cpp
    src/services/assistant/intentdialoguemanager/longchatimageparser.h
    src/services/assistant/intentdialoguemanager/longchatimageparser.cpp
    src/services/assistant/intentdialoguemanager/systemscenarioclass.h
    src/services/assistant/intentdialoguemanager/systemscenarioclass.cpp
    src/services/assistant/intentdialoguemanager/variabletypeconversion.h
    src/services/assistant/intentdialoguemanager/variabletypeconversion.cpp

    src/services/assistant/dialoguemanager/dialoguemanager.cpp
    src/services/assistant/dialoguemanager/dialoguemanager.h
    src/services/assistant/session.h
    src/services/assistant/session.cpp
    src/services/assistant/error.h
    src/services/assistant/error.cpp
    src/services/assistant/promptmanager.h
    src/services/assistant/promptmanager.cpp

    ${PROTO_FILES}
    src/utils/datetime.h
    src/utils/datetime.cpp
    src/utils/dirinfo.h
    src/utils/dirinfo.cpp
    src/utils/fileinfo.h
    src/utils/fileinfo.cpp
    src/utils/fileinfolist.h
    src/utils/fileinfolist.cpp
    src/utils/util.h
    src/utils/util.cpp
    src/utils/logger.h
    src/utils/logger.cpp
    src/utils/settings.h
    src/utils/settings.cpp
    src/utils/stringlist.h
    src/utils/stringlist.cpp
    src/utils/audiorecorder.h
    src/utils/audiorecorder.cpp
    src/utils/audioplayer.h
    src/utils/audioplayer.cpp
    src/utils/library.h
    src/utils/library.cpp
    src/encryption/encryption.h
    src/encryption/encryption.cpp
    src/encryption/aes256.h
    src/encryption/aes256.cpp
    src/encryption/pbkdf2.h
    src/encryption/pbkdf2.cpp
    src/encryption/rsa2048.h
    src/encryption/rsa2048.cpp

    src/longchatimageparser/processer/longimageparser.cpp
    src/longchatimageparser/processer/ocrengine.cpp
    src/longchatimageparser/processer/imageslicer.cpp
    src/longchatimageparser/processer/avatardetector.cpp
    src/longchatimageparser/processer/contentmarker.cpp
    src/longchatimageparser/processer/deduplicator.cpp
    src/longchatimageparser/processer/chatanalyzer.cpp
    src/longchatimageparser/utils/chatmessage.cpp
    src/longchatimageparser/utils/jsonexporter.cpp
    src/longchatimageparser/utils/visualization.cpp
    src/longchatimageparser/utils/chinesetextrenderer.cpp
    src/longchatimageparser/utils/imageutils.cpp
    src/longchatimageparser/utils/compatibilitypatch.cpp
    src/longchatimageparser/utils/config.cpp
    src/longchatimageparser/utils/ocravatartypes.cpp
    src/longchatimageparser/utils/sliceinfo.cpp
)

# 获取系统处理器架构
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|aarch64)$")
    target_link_libraries(
        kylin-ai-runtime
        PkgConfig::VECTOR_DB
        PkgConfig::GRPCPP
        ${KYAI_KNOWLEDGE_BASE_CLIENT_LIBRARIES}
        httpclient
        ${TRITONCLIENT_LIB_DIRS}
        Python3::Python
        pybind11::embed
        pybind11::pybind11
    )
endif ()

target_link_libraries(
    kylin-ai-runtime
    pthread
    jsoncpp
    crypto
    dl
    ${GIO_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${SODIUM_LIBRARIES}
    curl
    pulse
    pulse-simple
    spdlog::spdlog
    atomic
    ${ICU_UC_LIBRARIES}
    ${OpenCV_LIBS}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(kylin-ai-runtime stdc++fs)
    endif ()
endif ()

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(tests)
endif ()

include(CMakePackageConfigHelpers)

install(TARGETS kylin-ai-runtime DESTINATION /usr/bin)
install(DIRECTORY configs/etc/ DESTINATION /etc)
install(DIRECTORY configs/usr/ DESTINATION /usr)