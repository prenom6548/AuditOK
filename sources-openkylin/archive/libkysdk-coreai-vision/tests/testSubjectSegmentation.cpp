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
#include <assert.h>
#include <gio/gio.h>
#include <sys/time.h>

#include <condition_variable>
#include <cstdio>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "error.h"
#include "subjectsegmentation.h"

// 定义测试用例结构体
struct TestCase {
    std::string doc;
    int mode;
    bool enable_glib;
};

struct InitContext {
    gboolean completed = false;
    int result = 0;
};

std::vector<std::tuple<std::string, SubjectSegmentationMode, bool>> testCases = {
    {"portrait1.jpg", static_cast<SubjectSegmentationMode>(0), true},
    {"portrait2.jpg", static_cast<SubjectSegmentationMode>(0), true},
    {"portrait3.jpg", static_cast<SubjectSegmentationMode>(0), true},
    {"portrait4.jpg", static_cast<SubjectSegmentationMode>(0), true},
    {"portrait5.jpg", static_cast<SubjectSegmentationMode>(0), true},
    {"portrait6.jpg", static_cast<SubjectSegmentationMode>(1), true},
    {"portrait7.jpg", static_cast<SubjectSegmentationMode>(1), true},
    {"portrait8.jpg", static_cast<SubjectSegmentationMode>(1), true},
    {"portrait9.jpg", static_cast<SubjectSegmentationMode>(1), true},
    {"portrait10.jpg", static_cast<SubjectSegmentationMode>(1), true},
    {"portrait1.jpg", static_cast<SubjectSegmentationMode>(1), false},
    {"portrait2.jpg", static_cast<SubjectSegmentationMode>(1), false},
    {"portrait3.jpg", static_cast<SubjectSegmentationMode>(1), false},
    {"portrait4.jpg", static_cast<SubjectSegmentationMode>(1), false},
    {"portrait5.jpg", static_cast<SubjectSegmentationMode>(1), false},
    {"portrait6.jpg", static_cast<SubjectSegmentationMode>(0), false},
    {"portrait7.jpg", static_cast<SubjectSegmentationMode>(0), false},
    {"portrait8.jpg", static_cast<SubjectSegmentationMode>(0), false},
    {"portrait9.jpg", static_cast<SubjectSegmentationMode>(0), false},
    {"portrait10.jpg", static_cast<SubjectSegmentationMode>(0), false}};

// 添加PPM格式保存函数
bool save_as_ppm(const char *filename, const unsigned char *data, int width, const int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) return false;

    // 写入PPM头部信息（灰度图 P5）
    fprintf(fp, "P5\n%d %d\n255\n", width, height);
    fwrite(data, 1, width * height, fp);
    fclose(fp);
    return true;
}

unsigned char *create_edge_points_image(const int width, const int height, const SubjectSegmentationPointGroup *groups,
                                        const int group_counts) {
    assert(width > 0 && height > 0);
    auto *new_image = new unsigned char[width * height];
    memset(new_image, 0, width * height);  // 黑色背景

    for (int i = 0; i < group_counts; ++i) {
        for (int j = 0; j < groups[i].point_count; ++j) {
            int x = groups[i].points[j].x;
            int y = groups[i].points[j].y;

            if (x >= 0 && x < width && y >= 0 && y < height) {
                new_image[y * width + x] = 255;  // 白色点
            }
        }
    }

    return new_image;
}

std::string get_full_time_stamp() {
    const time_t now = time(nullptr);
    timeval tv{};
    gettimeofday(&tv, nullptr);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", localtime(&now));
    char full_timestamp[35];
    sprintf(full_timestamp, "%s%03ld", timestamp, tv.tv_usec / 1000);
    return std::string(full_timestamp);
}

void callback(SubjectSegmentationResult *result, void *user_data) {
    int err = subject_segmentation_result_get_error_code(result);
    printf("portrait subject segmentation result: error_code=%d\n", err);
    assert(err == AISDK_NO_ERROR);

    // 处理前景图像
    int width = 0, height = 0;
    size_t data_length = 0;
    const unsigned char *imageData =
        subject_segmentation_result_get_foreground_image_data(result, &width, &height, &data_length);
    assert(imageData != nullptr);
    const char *format = subject_segmentation_result_get_foreground_image_format(result);
    if (format) {
        printf("imageData format: %s\n", format);
    }
    printf("imageData: %dx%d, data_length=%zu\n", width, height, data_length);

    printf("imageData first 10 bytes: ");
    for (size_t i = 0; i < data_length && i < 10; ++i) {
        printf("%02X ", imageData[i]);
    }
    printf("\n");

    std::string filename = "imageData_" + get_full_time_stamp() + ".png";
    FILE *fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fwrite(imageData, 1, data_length, fp);
        fclose(fp);
        printf("imageData data saved to %s\n", filename.c_str());
    } else {
        printf("failed to save imageData data to file\n");
    }

    // 处理掩码
    auto maskData = subject_segmentation_result_get_mask_data(result);
    assert(maskData != nullptr);
    printf("maskData: %dx%d, data_length=%zu\n", maskData->mask_width, maskData->mask_height, maskData->mask_length);

    printf("maskData first 10 bytes: ");
    for (size_t i = 0; i < maskData->mask_length && i < 10; ++i) {
        printf("%02X ", maskData->mask_data[i]);
    }
    printf("\n");

    std::string mask_filename = "mask_" + get_full_time_stamp() + ".png";
    FILE *mask_fp = fopen(mask_filename.c_str(), "wb");
    if (mask_fp) {
        fwrite(maskData->mask_data, 1, maskData->mask_length, mask_fp);
        fclose(mask_fp);
        printf("Mask data saved to %s\n", mask_filename.c_str());
    } else {
        printf("failed to save mask data to file\n");
    }

    // 处理边缘点
    int group_counts = 0;
    const SubjectSegmentationPointGroup *groups =
        subject_segmentation_result_get_edge_point_groups(result, &group_counts);
    assert(groups != nullptr);
    assert(group_counts > 0);
    printf("edge point group count: %d\n", group_counts);
    for (int i = 0; i < group_counts; ++i) {
        printf("  group %d point count: %d\n", i, groups[i].point_count);
        for (int j = 0; j < groups[i].point_count && j < 5; ++j) {
            printf("    (%d, %d) ", groups[i].points[j].x, groups[i].points[j].y);
        }
        if (groups[i].point_count > 5) printf("...");
        printf("\n");
    }

    unsigned char *edge_points_image = create_edge_points_image(width, height, groups, group_counts);
    if (edge_points_image) {
        std::string ppm_filename = "mask_edge_points_" + get_full_time_stamp() + ".ppm";
        if (save_as_ppm(ppm_filename.c_str(), edge_points_image, width, height)) {
            printf("Edge points image saved to %s\n", ppm_filename.c_str());
        } else {
            printf("Failed to save edge points image to %s\n", ppm_filename.c_str());
        }
        delete[] edge_points_image;
    }

    // 统一通过 GMainLoop 控制退出
    auto *loop = static_cast<GMainLoop *>(user_data);
    if (loop) {
        g_main_loop_quit(loop);
    }
}

static gboolean wait_for_async_init_completion(GMainLoop *loop, gboolean *completed, int *result, int expected_result) {
    const int MAX_WAIT_TIME_MS = 10000;  // 10秒超时
    const int CHECK_INTERVAL_MS = 10;
    int waited_time = 0;

    while (!(*completed) && waited_time < MAX_WAIT_TIME_MS) {
        while (g_main_context_pending(g_main_loop_get_context(loop))) {
            if (!g_main_context_iteration(g_main_loop_get_context(loop), FALSE)) {
                break;
            }
        }
        if (!(*completed)) {
            g_usleep(CHECK_INTERVAL_MS * 1000);
            waited_time += CHECK_INTERVAL_MS;
        }
    }

    if (waited_time >= MAX_WAIT_TIME_MS && !(*completed)) {
        fprintf(stderr, "Error: Async initialization timed out\n");
        return FALSE;
    }

    if (*result != expected_result) {
        fprintf(stderr, "Error: Expected result %d, got %d\n", expected_result, *result);
        return FALSE;
    }

    return TRUE;
}

SubjectSegmentationConfig *createDefaultSubjectSegmentationConfig() {
    auto *config = subject_segmentation_config_create();
    assert(config != nullptr);
    return config;
}

void testNullSession() {
    SubjectSegmentationSession *session = nullptr;
    subject_segmentation_init_session_async(
        session,
        [](int error, void *user_data) {
            assert(error == AI_COMMON_INVALID_SESSION);
            std::fprintf(stderr, "testNullSession passed.\n");
        },
        nullptr);
}

void testNoExistingImageFile() {
    GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);
    SubjectSegmentationSession *session = subject_segmentation_create_session();
    if (!session) {
        g_main_loop_unref(pMainLoop);
        return;
    }

    auto config = createDefaultSubjectSegmentationConfig();
    if (!config) {
        subject_segmentation_destroy_session(&session);
        g_main_loop_unref(pMainLoop);
        return;
    }

    subject_segmentation_set_config(session, config);

    InitContext ctx;

    subject_segmentation_init_session_async(
        session,
        [](int error, void *user_data) {
            auto *c = static_cast<InitContext *>(user_data);
            c->result = error;
            c->completed = TRUE;
        },
        &ctx);

    bool completed = wait_for_async_init_completion(pMainLoop, &ctx.completed, &ctx.result, AI_COMMON_SUCCESS);
    assert(completed);

    subject_segmentation_result_set_callback(
        session,
        [](SubjectSegmentationResult *result, void *user_data) {
            int err = subject_segmentation_result_get_error_code(result);
            assert(err == AI_COMMON_FILE_NOT_FOUND);
            std::fprintf(stderr, "testNoExistingImageFile passed.\n");

            auto *loop = static_cast<GMainLoop *>(user_data);
            if (loop) {
                g_main_loop_quit(loop);
            }
        },
        pMainLoop);

    subject_segmentation_run_from_image_file_async(session, nullptr, "non_existing_file.jpg");
    // g_main_loop_run(pMainLoop);

    subject_segmentation_destroy_session(&session);
    subject_segmentation_config_destroy(&config);
    if (pMainLoop) {
        g_main_loop_unref(pMainLoop);
    }
}

void testKillRuntime() {
    system("killall kylin-ai-runtime");
    SubjectSegmentationSession *session = subject_segmentation_create_session();
    assert(session != nullptr);
    SubjectSegmentationConfig *config = createDefaultSubjectSegmentationConfig();
    subject_segmentation_set_config(session, config);
    subject_segmentation_init_session_async(
        session, [](int error, void *user_data) { assert(error == AI_COMMON_RUNTIME_ERROR); }, nullptr);
    subject_segmentation_destroy_session(&session);
    subject_segmentation_config_destroy(&config);
}

void testSubjectSegmentationFromImageFile(const char *imageFile, SubjectSegmentationMode mode,
                                          bool enableInternalGlib) {
    auto *session = subject_segmentation_create_session();
    assert(session != nullptr);
    SubjectSegmentationConfig *config = createDefaultSubjectSegmentationConfig();
    subject_segmentation_config_set_mode(config, mode);
    subject_segmentation_set_config(session, config);
    assert(subject_segmentation_config_get_mode(config) == mode);
    subject_segmentation_enable_internal_event_loop(session, enableInternalGlib);

    InitContext ctx;

    subject_segmentation_init_session_async(
        session,
        [](int error, void *user_data) {
            auto *c = static_cast<InitContext *>(user_data);
            c->result = error;
            c->completed = TRUE;
        },
        &ctx);

    GMainLoop *initLoop = g_main_loop_new(nullptr, FALSE);
    bool completed = wait_for_async_init_completion(initLoop, &ctx.completed, &ctx.result, AI_COMMON_SUCCESS);
    assert(completed);
    g_main_loop_unref(initLoop);

    if (!enableInternalGlib) {
        GMainLoop *runLoop = g_main_loop_new(nullptr, FALSE);
        subject_segmentation_result_set_callback(session, callback, runLoop);
        subject_segmentation_run_from_image_file_async(session, nullptr, imageFile);
        g_main_loop_run(runLoop);
        g_main_loop_unref(runLoop);
    } else {
        // 同步等待 callback 完成
        std::mutex mtx;
        std::condition_variable cv;
        bool done = false;

        struct SyncContext {
            std::mutex *mtx;
            std::condition_variable *cv;
            bool *done;
        } syncCtx{&mtx, &cv, &done};

        auto internal_callback = [](SubjectSegmentationResult *result, void *user_data) {
            callback(result, nullptr);  // 原 callback 不依赖 loop

            auto *ctx = static_cast<SyncContext *>(user_data);
            {
                std::lock_guard<std::mutex> lock(*ctx->mtx);
                *ctx->done = true;
            }
            ctx->cv->notify_one();
        };

        subject_segmentation_result_set_callback(session, internal_callback, &syncCtx);
        subject_segmentation_run_from_image_file_async(session, nullptr, imageFile);

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&done]() { return done; });
    }

    subject_segmentation_destroy_session(&session);
    subject_segmentation_config_destroy(&config);
}

void testSubjectSegmentationFromImageData(const char *imageData, unsigned int imageDataSize,
                                          SubjectSegmentationMode mode, bool enableInternalGlib) {
    auto *session = subject_segmentation_create_session();
    assert(session != nullptr);
    SubjectSegmentationConfig *config = createDefaultSubjectSegmentationConfig();
    subject_segmentation_config_set_mode(config, mode);
    subject_segmentation_set_config(session, config);
    assert(subject_segmentation_config_get_mode(config) == mode);
    subject_segmentation_enable_internal_event_loop(session, enableInternalGlib);

    InitContext ctx;

    subject_segmentation_init_session_async(
        session,
        [](int error, void *user_data) {
            auto *c = static_cast<InitContext *>(user_data);
            c->result = error;
            c->completed = TRUE;
        },
        &ctx);

    GMainLoop *initLoop = g_main_loop_new(nullptr, FALSE);
    bool completed = wait_for_async_init_completion(initLoop, &ctx.completed, &ctx.result, AI_COMMON_SUCCESS);
    assert(completed);
    g_main_loop_unref(initLoop);

    if (!enableInternalGlib) {
        GMainLoop *runLoop = g_main_loop_new(nullptr, FALSE);
        subject_segmentation_result_set_callback(session, callback, runLoop);
        subject_segmentation_run_from_image_data_async(session, nullptr, imageData, imageDataSize);
        g_main_loop_run(runLoop);
        g_main_loop_unref(runLoop);
    } else {
        // 同步等待 callback 完成
        std::mutex mtx;
        std::condition_variable cv;
        bool done = false;

        struct SyncContext {
            std::mutex *mtx;
            std::condition_variable *cv;
            bool *done;
        } syncCtx{&mtx, &cv, &done};

        auto internal_callback = [](SubjectSegmentationResult *result, void *user_data) {
            callback(result, nullptr);  // 原 callback 不依赖 loop

            auto *ctx = static_cast<SyncContext *>(user_data);
            {
                std::lock_guard<std::mutex> lock(*ctx->mtx);
                *ctx->done = true;
            }
            ctx->cv->notify_one();
        };

        subject_segmentation_result_set_callback(session, internal_callback, &syncCtx);
        subject_segmentation_run_from_image_data_async(session, nullptr, imageData, imageDataSize);
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&done]() { return done; });
    }

    subject_segmentation_destroy_session(&session);
    subject_segmentation_config_destroy(&config);
}

void testSubjectSegmentation(const std::string &samplesDir, bool fromFile = true) {
    for (const auto &testCase : testCases) {
        std::string filename = std::get<0>(testCase);
        SubjectSegmentationMode mode = std::get<1>(testCase);
        bool enableInternalGlib = std::get<2>(testCase);

        std::string fullPath = samplesDir + filename;

        if (fromFile) {
            testSubjectSegmentationFromImageFile(fullPath.c_str(), mode, enableInternalGlib);
        } else {
            std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
            if (file.is_open()) {
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<char> buffer(size);
                if (file.read(buffer.data(), size)) {
                    testSubjectSegmentationFromImageData(buffer.data(), size, mode, enableInternalGlib);
                }
                file.close();
            }
        }
    }
}

void testUnsupportedMode(const std::string &samplesDir) {
    SubjectSegmentationSession *session = subject_segmentation_create_session();
    assert(session != nullptr);
    SubjectSegmentationConfig *config = createDefaultSubjectSegmentationConfig();
    subject_segmentation_config_set_mode(config, static_cast<SubjectSegmentationMode>(3));
    subject_segmentation_set_config(session, config);
    InitContext ctx;

    subject_segmentation_init_session_async(
        session,
        [](int error, void *user_data) {
            auto *c = static_cast<InitContext *>(user_data);
            c->result = error;
            c->completed = TRUE;
        },
        &ctx);

    GMainLoop *initLoop = g_main_loop_new(nullptr, FALSE);
    bool completed = wait_for_async_init_completion(initLoop, &ctx.completed, &ctx.result, AI_COMMON_SUCCESS);
    assert(completed);
    g_main_loop_unref(initLoop);

    GMainLoop *runLoop = g_main_loop_new(nullptr, FALSE);
    subject_segmentation_result_set_callback(
        session,
        [](SubjectSegmentationResult *result, void *user_data) {
            auto errorCode = subject_segmentation_result_get_error_code(result);
            assert(errorCode == AI_COMMON_INVALID_PARAMETER);
            std::fprintf(stderr, "testUnsupportedMode passed.\n");
            auto *loop = static_cast<GMainLoop *>(user_data);
            if (loop) {
                g_main_loop_quit(loop);
            }
        },
        runLoop);
    std::string fullPath = samplesDir + std::get<0>(testCases[0]);

    subject_segmentation_run_from_image_file_async(session, nullptr, fullPath.c_str());

    g_main_loop_run(runLoop);
    g_main_loop_unref(runLoop);

    subject_segmentation_destroy_session(&session);
    subject_segmentation_config_destroy(&config);
}

int main() {
    testNullSession();
    testNoExistingImageFile();
    char cwd[1024];
    assert(getcwd(cwd, sizeof(cwd)) != nullptr);
    std::string samplesDir = std::string(cwd) + "/samples/";
    testUnsupportedMode(samplesDir);
    testSubjectSegmentation(samplesDir, true);   // 从文件加载
    testSubjectSegmentation(samplesDir, false);  // 从内存数据加载
    // testKillRuntime();

    std::fprintf(stderr, "All tests passed.\n");
    return 0;
}