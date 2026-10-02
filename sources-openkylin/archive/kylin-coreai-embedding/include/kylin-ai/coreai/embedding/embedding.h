/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#ifndef EMBEDDING_H
#define EMBEDDING_H

#include "modelinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _EmbeddingResult EmbeddingResult;

/// @brief 获取向量化结果 返回结果与result生命周期保持一致
/// @param result 传入 向量化结果的结构体
/// @return float 数组 存储的向量数据
float *embedding_result_get_vector_data(EmbeddingResult *result);

/// @brief 获取向量化结果
/// @param result 传入 向量化结果的结构体
/// @return float 数组 数组长度
int embedding_result_get_vector_length(EmbeddingResult *result);

/// @brief 获取向量化错误码
/// @param result 传入 向量化结果的结构体
/// @return 错误码
int embedding_result_get_error_code(EmbeddingResult *result);

/// @brief 获取向量化错误信息 返回结果与result生命周期保持一致
/// @param result 传入 向量化结果的结构体
/// @return 错误信息
const char *embedding_result_get_error_message(EmbeddingResult *result);

/// @brief  销毁向量化结果 [仅同步接口需要]
/// @param result
void embedding_result_destroy(EmbeddingResult **result);

__attribute__((deprecated)) void embedding_model_info_destroy(char *result);

typedef struct _TextEmbeddingSession TextEmbeddingSession;

/// @param result 生命周期为callback的作用域，资源释放由sdk侧管理
typedef void (*TextEmbeddingResultCallback)(EmbeddingResult *result,
                                            void *callback_user_data);

TextEmbeddingSession *text_embedding_create_session();
void text_embedding_destroy_session(TextEmbeddingSession **session);
int text_embedding_init_session(TextEmbeddingSession *session);

/// @brief 加载指定模型
/// @param session[入参] 调用text_embedding_init_session()后的session 对象
/// @param model_name[入参] 传入 embedding_model_info_get_model_name()
/// 获取的模型名称
/// @return 错误码
int text_embedding_init_model(TextEmbeddingSession *session,
                              const char *model_name);

/// @brief 获取文本向量化模型列表
/// @param session[入参] 调用text_embedding_init_session()后的session 对象
/// @param error_code[出参] 获取成功时返回0，否则返回具体的错误码
/// @return 文本向量化模型列表 对象的生命周期和session对象绑定
EmbeddingModelList *text_embedding_get_model_list(TextEmbeddingSession *session,
                                                  int *error_code);

__attribute__((
    deprecated("Use func text_embedding_get_model_list instead"))) bool
text_embedding_get_model_info(TextEmbeddingSession *session, char **model_info);

/// @brief 文本向量化模型 同步接口 向量化文本
/// @param session[入参] 传入 text_embedding_create_session() 创建的 session
/// @param text[入参] 传入 待向量化的文本
/// @param result[出参] 在使用时定义一个EmbeddingResult *result=nullptr,
/// 然后传入&result，需要embedding_result_destroy来销毁资源
/// @return 返回向量化结果
bool text_embedding(TextEmbeddingSession *session, const char *text,
                    EmbeddingResult **result);

/// @brief 文本向量化模型 异步接口 向量化文本
/// @param session 传入 text_embedding_create_session() 创建的 session
/// @param text 传入 待向量化的文本
/// @param callback 向量化结果回调函数 传入 TextEmbeddingResultCallback
/// 类型的结果回调函数
/// @param callback_user_data 调用者自定义的数据，作为 callback 的参数返回
void text_embedding_async(TextEmbeddingSession *session, const char *text,
                          TextEmbeddingResultCallback callback,
                          void *callback_user_data);

/// @brief 控制 SDK 是否自动驱动内部事件循环
/// @param session 传入text_embedding_create_session()创建的 session
/// @param enable  true 表示启用内部事件循环，false 表示关闭。默认值为 false
void text_embedding_enable_internal_event_loop(TextEmbeddingSession *session,
                                               bool enable);

typedef struct _ImageEmbeddingSession ImageEmbeddingSession;

/// @param callback参数的生命周期仅为callback的作用域，资源释放由sdk侧管理
typedef void (*ImageEmbeddingResultCallback)(EmbeddingResult *result,
                                             void *callback_user_data);

ImageEmbeddingSession *image_embedding_create_session();
void image_embedding_destroy_session(ImageEmbeddingSession **session);
int image_embedding_init_session(ImageEmbeddingSession *session);

/// @brief 加载指定模型
/// @param session[入参] image_embedding_init_session()后的session对象
/// @param model_name[入参] 传入 embedding_model_info_get_model_name()
/// 获取的模型名称
/// @return 错误码
int image_embedding_init_model(ImageEmbeddingSession *session,
                               const char *model_name);

/// @brief 获取图像向量化模型列表
/// @param session[入参] 调用image_embedding_init_session()后的session对象
/// @param error_code[出参] 获取成功时返回0，否则返回具体的错误码
/// @return 图像向量化模型列表 对象的生命周期和session对象绑定
EmbeddingModelList *image_embedding_get_model_list(
    ImageEmbeddingSession *session, int *error_code);

__attribute__((
    deprecated("Use func image_embedding_get_model_list instead"))) bool
image_embedding_get_model_info(ImageEmbeddingSession *session,
                               char **model_info);

/// @brief 图像向量化模型同步接口 向量化文本
/// @param session[入参] 传入 image_embedding_create_session() 创建的 session
/// @param text[入参] 传入 待向量化的文本
/// @param result[出参] 在使用时定义一个EmbeddingResult *result=nullptr,
/// 然后传入&result，需要embedding_result_destroy来销毁资源
/// @return 返回向量化结果
bool text_embedding_by_image_model(ImageEmbeddingSession *session,
                                   const char *text, EmbeddingResult **result);

/// @brief 图像向量化模型同步接口 向量化图像
/// @param session[入参] 传入 image_embedding_create_session() 创建的 session
/// @param image_file[入参] 传入 待向量化的图像路径
/// @param result[出参] 在使用时定义一个EmbeddingResult *result=nullptr,
/// 然后传入&result，需要embedding_result_destroy来销毁资源
/// @return 返回向量化结果
bool image_embedding_by_image_file(ImageEmbeddingSession *session,
                                   const char *image_file,
                                   EmbeddingResult **result);

/// @brief 图像向量化模型同步接口 向量化图像
/// @param session[入参] 传入 image_embedding_create_session() 创建的 session
/// @param image_data[入参] 传入 待向量化的base64图像数据
/// @param image_data_length[入参] 传入 base64图像数据的长度
/// @param result[出参] 在使用时定义一个EmbeddingResult *result=nullptr,
/// 然后传入&result，需要embedding_result_destroy来销毁资源
/// @return 返回向量化结果
bool image_embedding_by_base64_image_data(ImageEmbeddingSession *session,
                                          const unsigned char *image_data,
                                          unsigned int image_data_length,
                                          EmbeddingResult **result);

/// @brief 图像向量化模型 异步不并发接口 向量化文本
/// @param session 传入 image_embedding_create_session() 创建的 session
/// @param text 传入 待向量化的文本
/// @param callback 向量化结果回调函数 传入 ImageEmbeddingResultCallback
/// 类型的结果回调函数
/// @param callback_user_data 调用者自定义的数据，作为 callback 的参数返回
void text_embedding_by_image_model_async(ImageEmbeddingSession *session,
                                         const char *text,
                                         ImageEmbeddingResultCallback callback,
                                         void *callback_user_data);

/// @brief 图像向量化模型 异步不并发接口 向量化图像
/// @param session 传入 image_embedding_create_session() 创建的 session
/// @param file_path 传入 待向量化的图像路径
/// @param callback 向量化结果回调函数 传入 ImageEmbeddingResultCallback
/// 类型的结果回调函数
/// @param callback_user_data 调用者自定义的数据，作为 callback 的参数返回
void image_embedding_from_by_file_async(ImageEmbeddingSession *session,
                                        const char *file_path,
                                        ImageEmbeddingResultCallback callback,
                                        void *callback_user_data);

/// @brief 图像向量化模型 异步不并发接口 向量化图像
/// @param session 传入 image_embedding_create_session() 创建的 session
/// @param image_data 传入 待向量化的base64图像数据
/// @param image_data_length 传入 base64图像数据的长度
/// @param callback 向量化结果回调函数 传入 ImageEmbeddingResultCallback
/// 类型的结果回调函数
/// @param callback_user_data 调用者自定义的数据，作为 callback 的参数返回
void image_embedding_by_base64_image_data_async(
    ImageEmbeddingSession *session, const unsigned char *image_data,
    unsigned int image_data_length, ImageEmbeddingResultCallback callback,
    void *callback_user_data);

/// @brief 控制 SDK 是否自动驱动内部事件循环
/// @param session image_embedding_create_session()创建的 session
/// @param enable  true 表示启用内部事件循环，false 表示关闭。默认值为 false
void image_embedding_enable_internal_event_loop(ImageEmbeddingSession *session,
                                                bool enable);

#ifdef __cplusplus
}
#endif

#endif
