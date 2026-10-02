#include <gio/gio.h>
#include <gio/giotypes.h>
#include <kylin-ai/common/error.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include "kylin-ai/genai/vision/image.h"
#include "logger.h"

static void writeBinaryDataToFile(const std::string& filename, const std::vector<char>& data) {
    std::ofstream outputFile(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    // 写入数据到文件
    outputFile.write(data.data(), data.size());

    // 检查是否写入成功
    if (!outputFile.good()) {
        std::cerr << "Error occurred while writing to file." << std::endl;
    } else {
        std::cout << "Binary data has been written to file: " << filename << std::endl;
    }

    // 关闭文件
    outputFile.close();
}

void callback(VisionImageResult* image_data, void* user_data) {
    int imageDataLength;
    const uint8_t* imageData = +image_result_get_data(image_data, &imageDataLength);
    int imageWidth = image_result_get_size(image_data).width;
    int imageHeight = image_result_get_size(image_data).height;
    ImageFormat format = image_result_get_format(image_data);
    int index = image_result_get_index(image_data);
    int total = image_result_get_total(image_data);

    int errorcode = image_result_get_error_code(image_data);
    std::string errorMsg = image_result_get_error_message(image_data);

    std::vector<char> imagedata{imageData, imageData + imageDataLength};
    writeBinaryDataToFile("./test3.png", imagedata);

    std::cout << "length " << imageDataLength << " width " << imageWidth << " height " << imageHeight << " format "
              << format << std::endl;
    std::cout << "index " << index << " total " << total << " errorCode " << errorcode << " errorMag " << errorMsg
              << std::endl;
}

void test() {
    GenAiImageSession* session = genai_image_create_session();
    genai_image_init_session(session);

    ImageConfig* config = image_config_create();
    image_config_set_generation_number(config, 1);
    image_config_set_style(config, "写实风格");
    image_config_set_size(config, ImageSize{1280, 720});
    genai_image_set_config(session, config);

    bool imageNumber = genai_image_get_supported_image_number(session);
    std::cout << "supported image number " << imageNumber << std::endl;

    int sizeNumber;
    const ImageSize* imageSize = genai_image_get_supported_image_size(session, &sizeNumber);
    int i = 0;
    while (i < sizeNumber) {
        std::cout << "supported image size width " << imageSize[i].width << " height " << imageSize[i].height
                  << std::endl;
        ++i;
    }

    int styleNumber;
    int j = 0;
    const char** imageStyle = genai_image_get_supported_image_style(session, &styleNumber);
    while (j < styleNumber) {
        std::cout << "supported image style " << imageStyle[j] << std::endl;
        ++j;
    }

    int* a = new int();
    *a = 100;
    genai_image_result_set_callback(session, callback, a);

    genai_image_generate_image_async(session, "生成一张小狗的图片");

    auto* pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, pMainLoop] {
        while (std::getchar() != '\n') {
        }
        image_config_destroy(&config);
        genai_image_destroy_session(&session);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();

    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test2() {
    ImageConfig* config = image_config_create();
    image_config_set_generation_number(config, 1);
    image_config_set_style(config, "写实风格");
    image_config_set_size(config, ImageSize{1280, 720});
    image_config_set_model_name(config, "xunfei-spark-image-generation");
    GenAiImageSession* session = genai_image_create_session();
    genai_image_enable_internal_event_loop(session, true);
    genai_image_set_config(session, config);
    int errorCode = genai_image_init_session(session);
    if (errorCode != AI_COMMON_SUCCESS) {
        LOGE("Init error!");
        return;
    }
    genai_image_result_set_callback(session, callback, nullptr);
    genai_image_generate_image_async(session, "生成一张小狗的图片");
    while (std::getchar() != '\n');

    image_config_set_model_name(config, "baidu-ernie-drawing-advanced");
    genai_image_set_config(session, config);
    genai_image_generate_image_async(session, "生成一张小猪的图片");
    while (std::getchar() != '\n');
    image_config_set_model_name(config, "xunfei-spark-image-generation");
    genai_image_set_config(session, config);
    genai_image_generate_image_async(session, "生成一张小猫的图片");
    while (std::getchar() != '\n');

    genai_image_destroy_session(&session);
    image_config_destroy(&config);
}

void test3() {
    GenAiImageSession* session = genai_image_create_session();
    genai_image_enable_internal_event_loop(session, true);
    ImageConfig* config = image_config_create();
    image_config_set_generation_number(config, 1);
    image_config_set_style(config, "写实风格");
    image_config_set_size(config, ImageSize{1280, 720});
    int errorCode = genai_image_init_session(session);
    if (errorCode != AI_COMMON_SUCCESS) {
        LOGE("Init error!");
        return;
    }
    genai_image_result_set_callback(session, callback, nullptr);
    while (std::getchar() != '\n');

    image_config_set_model_name(config, "");
    genai_image_set_config(session, config);
    genai_image_generate_image_async(session, "生成一张小狗的图片");
    while (std::getchar() != '\n');

    image_config_set_model_name(config, "");
    genai_image_set_config(session, config);
    genai_image_generate_image_async(session, "生成一张小猪的图片");
    while (std::getchar() != '\n');

    image_config_set_model_name(config, "");
    genai_image_set_config(session, config);
    genai_image_generate_image_async(session, "生成一张小猫的图片");
    while (std::getchar() != '\n');

    image_config_set_model_name(config, "baidu-ernie-drawing-advanced");
    genai_image_set_config(session, config);
    genai_image_generate_image_async(session, "生成一张小狗的图片");
    while (std::getchar() != '\n');

    image_config_set_model_name(config, "xunfei-spark-image-generation");
    genai_image_set_config(session, config);
    genai_image_generate_image_async(session, "生成一张小猪的图片");
    while (std::getchar() != '\n');

    image_config_set_model_name(config, "");
    genai_image_set_config(session, config);
    genai_image_generate_image_async(session, "生成一张小狗的图片");
    while (std::getchar() != '\n');

    genai_image_destroy_session(&session);
    image_config_destroy(&config);
}

void test4() {
    GenAiImageSession* session = genai_image_create_session();
    genai_image_enable_internal_event_loop(session, true);
    ImageConfig* config = image_config_create();
    image_config_set_generation_number(config, 1);
    image_config_set_style(config, "写实风格");
    image_config_set_size(config, ImageSize{1280, 720});
    int errorCode = genai_image_init_session(session);
    if (errorCode != AI_COMMON_SUCCESS) {
        LOGE("Init error!");
        return;
    }
    genai_image_result_set_callback(session, callback, nullptr);
    while (std::getchar() != '\n');

    image_config_set_model_name(config, nullptr);
    genai_image_set_config(session, config);
    genai_image_generate_image_async(session, nullptr);
    while (std::getchar() != '\n');
    genai_image_destroy_session(&session);
    image_config_destroy(&config);
}

int main() {
    test4();
    return 0;
}
