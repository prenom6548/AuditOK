/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef OCR_H
#define OCR_H

#include <QObject>
#include <QString>
#include <QVector>
#include "base.h"
//#include <error.h>  // 保留错误码定义（如果头文件不依赖库）
//#include <QApplication>
//#include <iostream>
//#include <QDebug>
//#include <dlfcn.h>  // 动态加载库头文件
class OcrWorker : public QObject
{
    Q_OBJECT

public:
    explicit OcrWorker(QObject *parent = nullptr);
    ~OcrWorker();
//    void cleanup();

public Q_SLOTS:
    void doOcr(const QString &imagePath);

Q_SIGNALS:
    void ocrFinished(const QString &path, const QVector<QString> &result);
//    void ocrResult(const QVector<QString> &result);
private:
    void *m_ocrFd;
//    QString m_currentImagePath;
//    void processOCRResult(void *result);
//    bool loadCoreAiVisionLib();
//    void print_error_info(int error_code);
//    static void ocrCallback(void *result, void *userData);
//    // 定义函数指针类型（需与库中函数签名完全一致）
//    typedef void* (*TextRecognitionCreateSessionFunc)();
//    typedef void (*TextRecognitionDestroySessionFunc)(void**);
//    typedef void* (*TextRecognitionModelConfigCreateFunc)();
//    typedef void (*TextRecognitionModelConfigDestroyFunc)(void**);
//    typedef void (*TextRecognitionModelConfigSetNameFunc)(void*, const char*);
//    typedef void (*TextRecognitionModelConfigSetDeployTypeFunc)(void*, int);  // 假设ModelDeployType是int枚举
//    typedef void (*TextRecognitionSetModelConfigFunc)(void*, void*);
//    typedef int (*TextRecognitionInitSessionFunc)(void*);
//    typedef void (*TextRecognitionResultSetCallbackFunc)(void*, void (*)(void*, void*), void*);
//    typedef void (*TextRecognitionRecognizeTextFromImageFileAsyncFunc)(void*, const char*);
//    typedef const char* (*TextRecognitionResultGetValueFunc)(void*);
//    typedef int (*TextRecognitionResultGetErrorCodeFunc)(void*);
//    typedef const char* (*TextRecognitionResultGetErrorMessageFunc)(void*);
//    typedef void** (*TextRecognitionResultGetTextLinesFunc)(void*, int*);
//    typedef const char* (*TextLineGetValueFunc)(void*);
//    typedef void* (*TextLineGetCornerPointsFunc)(void*, int*);

//    // 动态库句柄和函数指针
//    void* g_libHandle = nullptr;
//    TextRecognitionCreateSessionFunc text_recognition_create_session = nullptr;
//    TextRecognitionDestroySessionFunc text_recognition_destroy_session = nullptr;
//    TextRecognitionModelConfigCreateFunc text_recognition_model_config_create = nullptr;
//    TextRecognitionModelConfigDestroyFunc text_recognition_model_config_destroy = nullptr;
//    TextRecognitionModelConfigSetNameFunc text_recognition_model_config_set_name = nullptr;
//    TextRecognitionModelConfigSetDeployTypeFunc text_recognition_model_config_set_deploy_type = nullptr;
//    TextRecognitionSetModelConfigFunc text_recognition_set_model_config = nullptr;
//    TextRecognitionInitSessionFunc text_recognition_init_session = nullptr;
//    TextRecognitionResultSetCallbackFunc text_recognition_result_set_callback = nullptr;
//    TextRecognitionRecognizeTextFromImageFileAsyncFunc text_recognition_recognize_text_from_image_file_async = nullptr;
//    TextRecognitionResultGetValueFunc text_recognition_result_get_value = nullptr;
//    TextRecognitionResultGetErrorCodeFunc text_recognition_result_get_error_code = nullptr;
//    TextRecognitionResultGetErrorMessageFunc text_recognition_result_get_error_message = nullptr;
//    TextRecognitionResultGetTextLinesFunc text_recognition_result_get_text_lines = nullptr;
//    TextLineGetValueFunc text_line_get_value = nullptr;
//    TextLineGetCornerPointsFunc text_line_get_corner_points = nullptr;
//    enum ModelDeployType { OnDevice = 0 };  // 需与库中定义一致

//    // 像素点结构体（需与库中定义一致）
//    struct PixelPoint {
//        int x;
//        int y;
//    };
};

#endif // OCR_H
