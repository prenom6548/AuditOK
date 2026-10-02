cmake_minimum_required(VERSION 3.14)

project(kylin-ai-abstract-models VERSION 1.0 LANGUAGES CXX)


include(GNUInstallDirs)
add_subdirectory(ppocr)
add_subdirectory(model_bank)

set(KYLIN_INSTALL_DIR "/usr/share/kylin-ai/")
message(STATUS "KYLIN_INSTALL_DIR: ${KYLIN_INSTALL_DIR}")

install(DIRECTORY ${CMAKE_BINARY_DIR}/ppocr/model_repos/ocr_ppocr DESTINATION ${KYLIN_INSTALL_DIR}model-repository PATTERN ".gitkeep" EXCLUDE)
install(DIRECTORY ${CMAKE_BINARY_DIR}/model_bank/model_repos/model_bank DESTINATION ${KYLIN_INSTALL_DIR}model-repository PATTERN ".gitkeep" EXCLUDE)

install(DIRECTORY gte-base-uint8-onnx/embd_gte-base_uint8-text DESTINATION ${KYLIN_INSTALL_DIR}model-repository PATTERN ".gitkeep" EXCLUDE)
install(DIRECTORY gte-base-uint8-onnx/ensemble-embd_gte-base_uint8-text DESTINATION ${KYLIN_INSTALL_DIR}model-repository PATTERN ".gitkeep" EXCLUDE)
install(DIRECTORY gte-base-uint8-onnx/tokenizer_gte-base_uint8-text DESTINATION ${KYLIN_INSTALL_DIR}model-repository PATTERN ".gitkeep" EXCLUDE)

install(DIRECTORY cn-clip-uint8-onnx/embd_cn-clip_512-uint8-image DESTINATION ${KYLIN_INSTALL_DIR}model-repository PATTERN ".gitkeep" EXCLUDE)
install(DIRECTORY cn-clip-uint8-onnx/embd_cn-clip_512-uint8-text DESTINATION ${KYLIN_INSTALL_DIR}model-repository PATTERN ".gitkeep" EXCLUDE)
install(DIRECTORY cn-clip-uint8-onnx/ensemble-embd_cn-clip_512-uint8-text DESTINATION ${KYLIN_INSTALL_DIR}model-repository PATTERN ".gitkeep" EXCLUDE)
install(DIRECTORY cn-clip-uint8-onnx/tokenizer_cn-clip_512-uint8-text DESTINATION ${KYLIN_INSTALL_DIR}model-repository PATTERN ".gitkeep" EXCLUDE)

