/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gio/gio.h>

#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

#include "config.h"
#include "logger.h"
#include "textrecognition.h"

void callback(TextRecognitionResult *result, void *user_data) {
    int textLineNum = 0, pointsNum = 0;
    LOGI("result      :", text_recognition_result_get_text_lines(result, &textLineNum));
    LOGI("textLineNum :", textLineNum);
    LOGI("text        :", text_recognition_result_get_value(result));
    LOGI("err code    :", text_recognition_result_get_error_code(result));
    LOGI("err message :", text_recognition_result_get_error_message(result));

    _TextLine **textline = text_recognition_result_get_text_lines(result, &textLineNum);
    if (textline == nullptr) {
        LOGE("The result is invalid, please check image");
        return;
    }
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "point.txt";
    if (not fs::exists(path)) {
        LOGE("error");
        return;
    }
    // 打开输出文件
    std::ofstream outFile(path);
    if (!outFile.is_open()) {
        LOGE("Failed to open output file:", path);
        return;
    }
    outFile << "[";
    for (int j = 0; j < textLineNum; j++) {
        PixelPoint *point = text_line_get_corner_points(textline[j], &pointsNum);
        if (point == nullptr) {
            LOGE("No point");
            return;
        }

        outFile << "[";
        for (int i = 0; i < 4; i++) {
            outFile << "(" << point[i].x << "," << point[i].y << ")";
            if (i <= 2) {
                outFile << ",";
            }
        }
        outFile << "],\n";
    }
    outFile << "]";

    outFile.close();
    for (int j = 0; j < textLineNum; j++) {
        PixelPoint *point = text_line_get_corner_points(textline[j], &pointsNum);
        if (point == nullptr) {
            LOGE("No point");
            return;
        }
        printf("[");
        for (int i = 0; i < 4; i++) {
            printf("(%d,%d)", point[i].x, point[i].y);
            if (i <= 2) {
                printf(",");
            }
        }
        printf("],\n");
    }

    if (user_data != nullptr) {
        LOGD(*reinterpret_cast<int *>(user_data));
    } else {
        LOGD("user data is nullptr");
    }
}
std::vector<char> readImageData(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file:", filePath);
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> imageData(fileSize);
    file.read(reinterpret_cast<char *>(imageData.data()), fileSize);
    return imageData;
}
void test01_OcrFromFile() {
    namespace fs = std::filesystem;
    fs::path dir = fs::current_path();
    fs::path imagePath = dir / "samples/test2.png";
    if (not fs::exists(imagePath)) {
        LOGE("Error:", imagePath, "is not exist.");
        return;
    }

    TextRecognitionResult *result = {};

    TextRecognitionSession *session = text_recognition_create_session();

    TextRecognitionModelConfig *config = text_recognition_model_config_create();
    text_recognition_model_config_set_name(config, "vision");
    text_recognition_model_config_set_deploy_type(config, ModelDeployType::OnDevice);

    text_recognition_set_model_config(session, config);

    text_recognition_init_session(session);

    text_recognition_result_set_callback(session, callback, result);

    text_recognition_recognize_text_from_image_file_async(session, imagePath.string().c_str());

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, pMainLoop] {
        while (std::getchar() != '\n') {
        }
        text_recognition_destroy_session(&session);
        text_recognition_model_config_destroy(&config);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();
    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}
void test02_OcrFromData() {
    namespace fs = std::filesystem;
    fs::path dir = fs::current_path();
    fs::path path = dir / "samples/test2.png";
    if (not fs::exists(path)) {
        LOGE("error");
        return;
    }
    TextRecognitionResult *result = {};
    const std::vector<char> imageData = readImageData(path.string());

    TextRecognitionSession *session = text_recognition_create_session();

    TextRecognitionModelConfig *config = text_recognition_model_config_create();
    ;

    text_recognition_set_model_config(session, config);

    text_recognition_init_session(session);

    text_recognition_result_set_callback(session, callback, result);

    text_recognition_recognize_text_from_image_data_async(session, imageData.data(), imageData.size());

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, pMainLoop] {
        while (std::getchar() != '\n') {
        }
        text_recognition_model_config_destroy(&config);
        text_recognition_destroy_session(&session);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();
    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

void test03_OcrMutilFile() {
    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();

    std ::vector<fs::path> paths;
    for (int i{0}; i < 3; ++i) {
        fs::path path = dir / ("test-" + std::to_string(i) + ".png");
        if (not fs::exists(path)) {
            LOGE("Error:", path, "is not exist.");
            continue;
        }
        paths.push_back(path);
    }

    if (paths.empty()) {
        LOGE("Error: none of the test paths exist.");
        return;
    }

    TextRecognitionResult *result = {};

    TextRecognitionSession *session = text_recognition_create_session();

    TextRecognitionModelConfig *config = text_recognition_model_config_create();

    text_recognition_set_model_config(session, config);

    text_recognition_init_session(session);

    text_recognition_result_set_callback(session, callback, result);

    text_recognition_recognize_text_from_image_file_async(session, paths[0].c_str());
    text_recognition_recognize_text_from_image_file_async(session, paths[1].c_str());
    text_recognition_recognize_text_from_image_file_async(session, paths[2].c_str());

    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

    std::thread ctrlThread([&session, &config, pMainLoop] {
        while (std::getchar() != '\n') {
        }
        text_recognition_destroy_session(&session);
        text_recognition_model_config_destroy(&config);
        g_main_loop_quit(pMainLoop);
    });
    ctrlThread.detach();
    g_main_loop_run(pMainLoop);
    g_main_loop_unref(pMainLoop);
}

int main() {
    test01_OcrFromFile();
    test02_OcrFromData();
    test03_OcrMutilFile();
    return 0;
}