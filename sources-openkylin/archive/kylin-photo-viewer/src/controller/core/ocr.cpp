/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "ocr.h"
#include <QDebug>
#include <dlfcn.h>
#include <algorithm>
#include <vector>
#include <string>
//#include <sys/utsname.h>
OcrWorker::OcrWorker(QObject *parent)
    : QObject(parent)
    , m_ocrFd(nullptr)
{
}

OcrWorker::~OcrWorker()
{
    if (m_ocrFd) {
        dlclose(m_ocrFd);
        m_ocrFd = nullptr;
    }
}

//// 加载动态库
//bool OcrWorker::loadCoreAiVisionLib() {
//    // 打开动态库
//    g_libHandle = dlopen("libkysdk-coreai-vision.so.1", RTLD_LAZY);
//    if (!g_libHandle) {
//        qDebug() << "加载动态库失败:" << dlerror();
//        return false;
//    }

//    // 重置错误
//    dlerror();

//    // 获取所有需要的函数指针
//    text_recognition_create_session = (TextRecognitionCreateSessionFunc)dlsym(g_libHandle, "text_recognition_create_session");
//    text_recognition_destroy_session = (TextRecognitionDestroySessionFunc)dlsym(g_libHandle, "text_recognition_destroy_session");
//    text_recognition_model_config_create = (TextRecognitionModelConfigCreateFunc)dlsym(g_libHandle, "text_recognition_model_config_create");
//    text_recognition_model_config_destroy = (TextRecognitionModelConfigDestroyFunc)dlsym(g_libHandle, "text_recognition_model_config_destroy");
//    text_recognition_model_config_set_name = (TextRecognitionModelConfigSetNameFunc)dlsym(g_libHandle, "text_recognition_model_config_set_name");
//    text_recognition_model_config_set_deploy_type = (TextRecognitionModelConfigSetDeployTypeFunc)dlsym(g_libHandle, "text_recognition_model_config_set_deploy_type");
//    text_recognition_set_model_config = (TextRecognitionSetModelConfigFunc)dlsym(g_libHandle, "text_recognition_set_model_config");
//    text_recognition_init_session = (TextRecognitionInitSessionFunc)dlsym(g_libHandle, "text_recognition_init_session");
//    text_recognition_result_set_callback = (TextRecognitionResultSetCallbackFunc)dlsym(g_libHandle, "text_recognition_result_set_callback");
//    text_recognition_recognize_text_from_image_file_async = (TextRecognitionRecognizeTextFromImageFileAsyncFunc)dlsym(g_libHandle, "text_recognition_recognize_text_from_image_file_async");
//    text_recognition_result_get_value = (TextRecognitionResultGetValueFunc)dlsym(g_libHandle, "text_recognition_result_get_value");
//    text_recognition_result_get_error_code = (TextRecognitionResultGetErrorCodeFunc)dlsym(g_libHandle, "text_recognition_result_get_error_code");
//    text_recognition_result_get_error_message = (TextRecognitionResultGetErrorMessageFunc)dlsym(g_libHandle, "text_recognition_result_get_error_message");
//    text_recognition_result_get_text_lines = (TextRecognitionResultGetTextLinesFunc)dlsym(g_libHandle, "text_recognition_result_get_text_lines");
//    text_line_get_value = (TextLineGetValueFunc)dlsym(g_libHandle, "text_line_get_value");
//    text_line_get_corner_points = (TextLineGetCornerPointsFunc)dlsym(g_libHandle, "text_line_get_corner_points");

//    // 检查函数指针是否获取成功
//    const char* err;
//    if ((err = dlerror()) != nullptr) {
//        qDebug() << "获取函数指针失败:" << err;
//        dlclose(g_libHandle);
//        g_libHandle = nullptr;
//        return false;
//    }

//    qDebug() << "动态库加载成功";
//    return true;
//}

//void OcrWorker::cleanup(){
//    if (g_libHandle) {
//        dlclose(g_libHandle);
//        g_libHandle = nullptr;
//        qDebug() << "动态库卸载成功";
//    }
//}

//// 错误码解释函数（保留原有逻辑）
//void OcrWorker::print_error_info(int error_code) {
//    qDebug() << "=== 错误码分析 ===";
//    qDebug() << "错误码:" << error_code;

//    if (error_code == 0) {
//        qDebug() << "状态: 成功，无错误";
//        return;
//    }

//    if (error_code == OCR_IMAGE_ERROR) {
//        qDebug() << "OCR错误: 数据文件无效 (OCR_IMAGE_ERROR = 100)";
//        qDebug() << "可能原因: 图片文件损坏、格式不支持或文件不存在";
//    }
//    else if (error_code == OCR_PARAM_INVALID) {
//        qDebug() << "OCR错误: 参数无效 (OCR_PARAM_INVALID = 101)";
//        qDebug() << "可能原因: 传入的参数不正确";
//    }
//    else if (error_code >= 100) {
//        qDebug() << "OCR专用错误码，具体含义请参考文档";
//    }
//    else {
//        qDebug() << "通用错误码，请参考4.6章节";
//    }

//    qDebug() << "=== 错误码分析完成 ===";
//}

//void OcrWorker::processOCRResult(void *result)
//{
//    qDebug() << "=== 开始打印OCR识别结果 ===";

//    QVector<QString> resultVector;
//    fprintf(stdout, "Start printing results.\n");
//    int text_line_num = 0, points_num = 0;

//    // 打印识别出的文字内容
//    fprintf(stdout, "text: %s\n", text_recognition_result_get_value(result));

//    // 打印错误码和错误信息
//    int error_code = text_recognition_result_get_error_code(result);
//    fprintf(stdout, "err code: %i\n", error_code);
//    fprintf(stdout, "err message : %s\n", text_recognition_result_get_error_message(result));

//    qDebug() << "错误码:" << error_code;

//    // 详细分析错误码
//    print_error_info(error_code);

//    // 检查是否有错误
//    if (error_code != 0) {
//        qDebug() << "OCR识别失败，错误码:" << error_code;
//        return;
//    }

//    // 获取文本行信息
//    void** text_line = text_recognition_result_get_text_lines(result, &text_line_num);
//    if (text_line == nullptr) {
//        qDebug() << "识别结果无效，请检查图片";
//        fprintf(stderr, "The result is invalid, please check image\n");
//        return;
//    }
//    for(int i = 0;i< text_line_num; ++i){
//        QString line_text = QString::fromUtf8(text_line_get_value(text_line[i]));
//        resultVector.append(line_text);
//    }

//    qDebug() << "识别到" << text_line_num << "行文字";

//    // 遍历每一行文字
//    for (int i = 0; i < text_line_num; ++i)
//    {
//        // 获取该行的角点坐标
//        PixelPoint *point = (PixelPoint*)text_line_get_corner_points(text_line[i], &points_num);
//        if (point == nullptr)
//        {
//            qDebug() << "第" << i << "行没有角点信息";
//            fprintf(stderr, "No point\n");
//            return;
//        }

//        // 打印该行的文字内容
//        fprintf(stdout, "The %i line text: %s\n", i, text_line_get_value(text_line[i]));
//        qDebug() << "第" << i << "行文字:" << text_line_get_value(text_line[i]);

//        // 打印该行的角点坐标
//        for (int j = 0; j < points_num; j++)
//        {
//            fprintf(stdout, "The corner points text %d: (%d, %d)\n", j, point[j].x, point[j].y);
//            qDebug() << "第" << i << "行第" << j << "个角点: (" << point[j].x << "," << point[j].y << ")";
//        }
//    }

//    qDebug() << "=== OCR识别结果打印完成 ===";
//    fprintf(stdout, "Printing result completed.\n");
//    Q_EMIT ocrFinished(m_currentImagePath, resultVector);
//}

//// OCR识别结果回调函数（调整为void*兼容动态加载）
//void OcrWorker::ocrCallback(void *result, void *userData)
//{
//    OcrWorker *ocr = static_cast<OcrWorker*>(userData);
//    if (!ocr) {
//        return;
//    }

//    ocr->processOCRResult(result);
//}

void OcrWorker::doOcr(const QString &imagePath)
{
//    struct utsname sysinfo;
//    if (uname(&sysinfo) == 0) {
//        std::string machine = sysinfo.machine;
//        // 龙芯架构：loongarch64, loongarch32, loongarch
//        // 神威架构：sw_64, sw64
//        if (machine.find("loongarch") != std::string::npos ||
//                machine.find("sw") != std::string::npos) {
            QVector<QString> result;

                // 加载动态库
                m_ocrFd = dlopen("libkyocr.so", RTLD_LAZY);
                if (!m_ocrFd) {
                    char *err = dlerror();
                    qDebug() << "链接动态库失败：libkyocr.so" << QByteArray(err);
                    Q_EMIT ocrFinished(imagePath, result);
                    return;
                }

                // 获取函数指针
                GetOcrFunc getCls = (GetOcrFunc)dlsym(m_ocrFd, "getCls");
                if (getCls == nullptr) {
                    qDebug() << "找不到符号:getCls";
                    dlclose(m_ocrFd);
                    m_ocrFd = nullptr;
                    Q_EMIT ocrFinished(imagePath, result);
                    return;
                }

                // 执行OCR识别
                std::vector<std::string> ret = getCls(imagePath.toStdString(), 1);
                std::transform(ret.begin(), ret.end(), std::back_inserter(result), [](const std::string &v) {
                    return QString::fromStdString(v);
                });

                // 关闭动态库
                dlclose(m_ocrFd);
                m_ocrFd = nullptr;

                // 发射完成信号
                Q_EMIT ocrFinished(imagePath, result);
//        }else{
//            qDebug() << "=== 程序开始运行 ===";

//            m_currentImagePath = imagePath;

//            // 加载动态库
//            if (!loadCoreAiVisionLib()) {
//                qDebug() << "动态库加载失败，程序退出";
//            }

//            // 先检查动态库是否加载成功
//            if (!g_libHandle) {
//                qDebug() << "动态库未加载，无法执行OCR识别";
//                return;
//            }

//            qDebug() << "=== 开始OCR识别测试 ===";

//            // 创建OCR会话
//            void *session = text_recognition_create_session();
//            if (session == nullptr) {
//                qDebug() << "错误：无法创建OCR会话";
//                return;
//            }
//            qDebug() << "OCR会话创建完成";

//            // 创建模型配置
//            void *config = text_recognition_model_config_create();
//            if (config == nullptr) {
//                qDebug() << "错误：无法创建模型配置";
//                text_recognition_destroy_session(&session);
//                return;
//            }
//            qDebug() << "模型配置创建完成";

//            // 设置模型名称和部署类型
//            text_recognition_model_config_set_name(config, "vision");
//            text_recognition_model_config_set_deploy_type(config, ModelDeployType::OnDevice);
//            text_recognition_set_model_config(session, config);
//            qDebug() << "模型配置设置完成";

//            // 初始化会话
//            qDebug() << "开始初始化OCR会话...";
//            int init_result = text_recognition_init_session(session);
//            if (init_result != 0) {
//                qDebug() << "错误：OCR会话初始化失败，错误码:" << init_result;
//                print_error_info(init_result);
//                text_recognition_destroy_session(&session);
//                text_recognition_model_config_destroy(&config);
//                return;
//            }
//            qDebug() << "OCR会话初始化完成";

//            // 设置回调函数
//            text_recognition_result_set_callback(session, OcrWorker::ocrCallback, this);
//            qDebug() << "回调函数设置完成";

//            // 开始异步识别
//            text_recognition_recognize_text_from_image_file_async(session, imagePath.toUtf8().constData());
//            qDebug() << "OCR识别已启动，等待结果...";
//        }
//    }
}
