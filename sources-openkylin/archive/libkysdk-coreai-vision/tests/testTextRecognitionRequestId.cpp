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
#include <gtest/gtest.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "error.h"
#include "textrecognition.h"
#include "textrecognitionconfig.h"
#include "textrecognitionresult.h"

namespace {

constexpr const char *kObjectPath = "/com/kylin/AiRuntime/CoreVision";
constexpr const char *kInterfaceName = "com.kylin.AiRuntime.CoreVisionService";

constexpr const char *kIntrospectionXml = R"XML(
<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
<node>
    <interface name="com.kylin.AiRuntime.CoreVisionService">
        <method name="init">
            <arg name="sessionId" type="i" direction="out" />
            <arg name="errorCode" type="i" direction="out" />
            <arg name="config" type="s" direction="in" />
        </method>
        <method name="destroy">
            <arg name="sessionId" type="i" direction="in" />
        </method>
        <method name="recognizeTextFromImageFile">
            <arg name="sessionId" type="i" direction="in" />
            <arg name="imageFilePath" type="s" direction="in" />
            <arg name="result" type="(sa(sa(ii)d)is)" direction="out" />
        </method>
        <method name="recognizeTextFromImageFileWithRequestId">
            <arg name="sessionId" type="i" direction="in" />
            <arg name="requestId" type="s" direction="in" />
            <arg name="imageFilePath" type="s" direction="in" />
            <arg name="result" type="(ssa(sa(ii)d)is)" direction="out" />
        </method>
        <method name="recognizeTextFromImageData">
            <arg name="sessionId" type="i" direction="in" />
            <arg name="imageData" type="s" direction="in" />
            <arg name="imageDataLength" type="u" direction="in" />
            <arg name="result" type="(sa(sa(ii)d)is)" direction="out" />
        </method>
        <method name="recognizeTextFromImageDataWithRequestId">
            <arg name="sessionId" type="i" direction="in" />
            <arg name="requestId" type="s" direction="in" />
            <arg name="imageData" type="s" direction="in" />
            <arg name="imageDataLength" type="u" direction="in" />
            <arg name="result" type="(ssa(sa(ii)d)is)" direction="out" />
        </method>
    </interface>
</node>
)XML";

struct CallbackResult {
    std::string request_id;
    std::string text;
    int error_code = -1;
    std::string error_message;
    int line_count = 0;
    std::vector<std::string> line_texts;
    std::vector<std::array<PixelPoint, 4>> line_points;
};

struct CallbackState {
    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;
    CallbackResult result;
};

struct MultiCallbackState {
    std::mutex mutex;
    std::condition_variable cv;
    int expected_count = 0;
    std::vector<CallbackResult> results;
};

class MockCoreVisionServer {
public:
    void Start() {
        const uid_t uid = getuid();
        const std::string base_dir = "/tmp/.kylin-ai-runtime-unix/" + std::to_string(uid);
        const std::string socket_path = base_dir + "/core-vision.sock";
        address_ = "unix:path=" + socket_path;

        std::error_code ec;
        std::filesystem::create_directories(base_dir, ec);
        std::filesystem::remove(socket_path, ec);

        ready_ = std::promise<void>();
        thread_ = std::thread([this]() { Run(); });
        ready_.get_future().wait();
    }

    bool IsStarted() const { return started_; }

    void Stop() {
        if (loop_ != nullptr) {
            g_main_loop_quit(loop_);
            if (context_ != nullptr) {
                g_main_context_wakeup(context_);
            }
        }
        if (thread_.joinable()) {
            thread_.join();
        }
    }

private:
    void Run() {
        context_ = g_main_context_new();
        g_main_context_push_thread_default(context_);
        loop_ = g_main_loop_new(context_, FALSE);

        GError *error = nullptr;
        introspection_ = g_dbus_node_info_new_for_xml(kIntrospectionXml, &error);
        if (introspection_ == nullptr) {
            if (error) {
                g_error_free(error);
            }
            started_ = false;
            ready_.set_value();
            return;
        }

        gchar *guid = g_dbus_generate_guid();
        server_ = g_dbus_server_new_sync(address_.c_str(), G_DBUS_SERVER_FLAGS_NONE, guid, nullptr, nullptr, &error);
        g_free(guid);
        if (server_ == nullptr) {
            if (error) {
                g_error_free(error);
            }
            started_ = false;
            ready_.set_value();
            return;
        }

        g_signal_connect(server_, "new-connection", G_CALLBACK(OnNewConnection), this);
        g_dbus_server_start(server_);

        started_ = true;
        ready_.set_value();
        g_main_loop_run(loop_);

        Cleanup();
        g_main_context_pop_thread_default(context_);
    }

    void Cleanup() {
        if (connection_ != nullptr && registration_id_ != 0) {
            g_dbus_connection_unregister_object(connection_, registration_id_);
        }
        if (connection_ != nullptr) {
            g_object_unref(connection_);
        }
        if (server_ != nullptr) {
            g_object_unref(server_);
        }
        if (introspection_ != nullptr) {
            g_dbus_node_info_unref(introspection_);
        }
        if (loop_ != nullptr) {
            g_main_loop_unref(loop_);
        }
        if (context_ != nullptr) {
            g_main_context_unref(context_);
        }
    }

    static gboolean OnNewConnection(GDBusServer *server, GDBusConnection *connection, gpointer user_data) {
        std::fprintf(stderr, "New connection established\n");
        auto *self = static_cast<MockCoreVisionServer *>(user_data);
        GError *error = nullptr;
        static const GDBusInterfaceVTable kVtable = {
            HandleMethodCall,
            nullptr,
            nullptr,
        };

        self->registration_id_ = g_dbus_connection_register_object(
            connection, kObjectPath, self->introspection_->interfaces[0], &kVtable, self, nullptr, &error);
        if (self->registration_id_ == 0) {
            if (error) {
                g_error_free(error);
            }
            return FALSE;
        }

        self->connection_ = G_DBUS_CONNECTION(g_object_ref(connection));
        return TRUE;
    }

    static void HandleMethodCall(GDBusConnection *connection, const gchar *sender, const gchar *object_path,
                                 const gchar *interface_name, const gchar *method_name, GVariant *parameters,
                                 GDBusMethodInvocation *invocation, gpointer user_data) {
        (void)connection;
        (void)sender;
        (void)object_path;
        (void)interface_name;
        (void)parameters;
        auto *self = static_cast<MockCoreVisionServer *>(user_data);

        if (g_strcmp0(method_name, "init") == 0) {
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(ii)", 1, AISDK_NO_ERROR));
            return;
        }
        if (g_strcmp0(method_name, "destroy") == 0) {
            g_dbus_method_invocation_return_value(invocation, nullptr);
            return;
        }
        if (g_strcmp0(method_name, "recognizeTextFromImageFile") == 0 ||
            g_strcmp0(method_name, "recognizeTextFromImageData") == 0) {
            g_dbus_method_invocation_return_value(invocation, self->BuildRecognitionResult());
            return;
        }
        if (g_strcmp0(method_name, "recognizeTextFromImageFileWithRequestId") == 0) {
            gint session_id = 0;
            const gchar *request_id = nullptr;
            const gchar *image_path = nullptr;
            g_variant_get(parameters, "(i&s&s)", &session_id, &request_id, &image_path);
            g_dbus_method_invocation_return_value(invocation,
                                                  self->BuildRecognitionResultWithRequestId(request_id));
            return;
        }
        if (g_strcmp0(method_name, "recognizeTextFromImageDataWithRequestId") == 0) {
            gint session_id = 0;
            const gchar *request_id = nullptr;
            const gchar *image_data = nullptr;
            guint image_data_length = 0;
            g_variant_get(parameters, "(i&s&su)", &session_id, &request_id, &image_data, &image_data_length);
            g_dbus_method_invocation_return_value(invocation,
                                                  self->BuildRecognitionResultWithRequestId(request_id));
            return;
        }

        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_FAILED, "Unsupported method");
    }

    GVariant *BuildRecognitionResult() {
        GVariantBuilder points_builder;
        g_variant_builder_init(&points_builder, G_VARIANT_TYPE("a(ii)"));
        g_variant_builder_add(&points_builder, "(ii)", 0, 0);
        g_variant_builder_add(&points_builder, "(ii)", 1, 0);
        g_variant_builder_add(&points_builder, "(ii)", 1, 1);
        g_variant_builder_add(&points_builder, "(ii)", 0, 1);
        GVariant *points = g_variant_builder_end(&points_builder);

        GVariantBuilder lines_builder;
        g_variant_builder_init(&lines_builder, G_VARIANT_TYPE("a(sa(ii)d)"));
        g_variant_builder_add(&lines_builder, "(s@a(ii)d)", "hello", points, 0.98);
        GVariant *lines = g_variant_builder_end(&lines_builder);

        return g_variant_new("((s@a(sa(ii)d)is))", "hello", lines, 0, "");
    }

    GVariant *BuildRecognitionResultWithRequestId(const gchar *request_id) {
        GVariantBuilder points_builder;
        g_variant_builder_init(&points_builder, G_VARIANT_TYPE("a(ii)"));
        g_variant_builder_add(&points_builder, "(ii)", 0, 0);
        g_variant_builder_add(&points_builder, "(ii)", 1, 0);
        g_variant_builder_add(&points_builder, "(ii)", 1, 1);
        g_variant_builder_add(&points_builder, "(ii)", 0, 1);
        GVariant *points = g_variant_builder_end(&points_builder);

        GVariantBuilder lines_builder;
        g_variant_builder_init(&lines_builder, G_VARIANT_TYPE("a(sa(ii)d)"));
        g_variant_builder_add(&lines_builder, "(s@a(ii)d)", "hello", points, 0.98);
        GVariant *lines = g_variant_builder_end(&lines_builder);

        const gchar *safe_request_id = request_id ? request_id : "";
        return g_variant_new("((ss@a(sa(ii)d)is))", safe_request_id, "hello", lines, 0, "");
    }

private:
    std::thread thread_;
    std::promise<void> ready_;
    std::string address_;
    GMainContext *context_ = nullptr;
    GMainLoop *loop_ = nullptr;
    GDBusServer *server_ = nullptr;
    GDBusNodeInfo *introspection_ = nullptr;
    GDBusConnection *connection_ = nullptr;
    guint registration_id_ = 0;
    bool started_ = false;
};

void CaptureResult(TextRecognitionResult *result, void *user_data) {
    auto *state = static_cast<CallbackState *>(user_data);
    CallbackResult snapshot;
    if (result != nullptr) {
        const char *request_id = text_recognition_result_get_request_id(result);
        const char *text = text_recognition_result_get_value(result);
        const char *error_message = text_recognition_result_get_error_message(result);

        snapshot.request_id = request_id ? request_id : "";
        snapshot.text = text ? text : "";
        snapshot.error_code = text_recognition_result_get_error_code(result);
        snapshot.error_message = error_message ? error_message : "";

        int line_count = 0;
        TextLine **lines = text_recognition_result_get_text_lines(result, &line_count);
        snapshot.line_count = line_count;
        if (lines != nullptr) {
            for (int i = 0; i < line_count; ++i) {
                const char *line_text = text_line_get_value(lines[i]);
                snapshot.line_texts.emplace_back(line_text ? line_text : "");

                int point_count = 0;
                PixelPoint *points = text_line_get_corner_points(lines[i], &point_count);
                if (points != nullptr && point_count == 4) {
                    std::array<PixelPoint, 4> copied = {points[0], points[1], points[2], points[3]};
                    snapshot.line_points.push_back(copied);
                }
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(state->mutex);
        state->done = true;
        state->result = std::move(snapshot);
    }
    state->cv.notify_one();
}

bool WaitForCallback(CallbackState *state, int timeout_ms) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(state->mutex);
            if (state->done) {
                return true;
            }
        }
        while (g_main_context_pending(nullptr)) {
            g_main_context_iteration(nullptr, FALSE);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return false;
}

bool WaitForCallbacks(MultiCallbackState *state, int timeout_ms) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(state->mutex);
            if (static_cast<int>(state->results.size()) >= state->expected_count) {
                return true;
            }
        }
        while (g_main_context_pending(nullptr)) {
            g_main_context_iteration(nullptr, FALSE);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return false;
}

class TextRecognitionRequestIdTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        server_ = std::make_unique<MockCoreVisionServer>();
        server_->Start();
        ASSERT_TRUE(server_->IsStarted());
    }

    static void TearDownTestSuite() {
        server_->Stop();
        server_.reset();
    }

    void SetUp() override {
        session_ = text_recognition_create_session();
        ASSERT_NE(session_, nullptr);

        config_ = text_recognition_model_config_create();
        ASSERT_NE(config_, nullptr);
        text_recognition_model_config_set_name(config_, "vision");
        text_recognition_model_config_set_deploy_type(config_, ModelDeployType::OnDevice);
        text_recognition_set_model_config(session_, config_);

        ASSERT_EQ(text_recognition_init_session(session_), AISDK_NO_ERROR);
    }

    void TearDown() override {
        if (config_ != nullptr) {
            text_recognition_model_config_destroy(&config_);
        }
        if (session_ != nullptr) {
            text_recognition_destroy_session(&session_);
        }
    }

    static std::unique_ptr<MockCoreVisionServer> server_;
    TextRecognitionSession *session_ = nullptr;
    TextRecognitionModelConfig *config_ = nullptr;
};

std::unique_ptr<MockCoreVisionServer> TextRecognitionRequestIdTest::server_;

}  // namespace

TEST(TextRecognitionResultAccessors, NullHandling) {
    EXPECT_EQ(text_line_get_value(nullptr), nullptr);
    EXPECT_EQ(text_line_get_corner_points(nullptr, nullptr), nullptr);
    EXPECT_EQ(text_recognition_result_get_value(nullptr), nullptr);
    EXPECT_EQ(text_recognition_result_get_text_lines(nullptr, nullptr), nullptr);
    EXPECT_EQ(text_recognition_result_get_error_message(nullptr), nullptr);
    EXPECT_EQ(text_recognition_result_get_request_id(nullptr), nullptr);
    EXPECT_EQ(text_recognition_result_get_error_code(nullptr), -1);
}

TEST_F(TextRecognitionRequestIdTest, RequestIdFromImageFile) {
    CallbackState state;
    text_recognition_result_set_callback(session_, CaptureResult, &state);

    text_recognition_recognize_text_from_image_file_async_with_request_id(session_, "request-001", "/tmp/a.png");
    ASSERT_TRUE(WaitForCallback(&state, 2000));

    EXPECT_EQ(state.result.request_id, "request-001");
    EXPECT_EQ(state.result.text, "hello");
    EXPECT_EQ(state.result.error_code, AISDK_NO_ERROR);
    EXPECT_EQ(state.result.line_count, 1);
    ASSERT_EQ(state.result.line_texts.size(), 1u);
    EXPECT_EQ(state.result.line_texts[0], "hello");
    ASSERT_EQ(state.result.line_points.size(), 1u);
    EXPECT_EQ(state.result.line_points[0][0].x, 0);
    EXPECT_EQ(state.result.line_points[0][0].y, 0);
}

TEST_F(TextRecognitionRequestIdTest, AutoGeneratedRequestId) {
    CallbackState state;
    text_recognition_result_set_callback(session_, CaptureResult, &state);

    text_recognition_recognize_text_from_image_file_async_with_request_id(session_, nullptr, "/tmp/b.png");
    ASSERT_TRUE(WaitForCallback(&state, 2000));

    EXPECT_FALSE(state.result.request_id.empty());
    EXPECT_TRUE(state.result.request_id.rfind("req-", 0) == 0);
}

TEST_F(TextRecognitionRequestIdTest, RequestIdFromImageData) {
    CallbackState state;
    text_recognition_result_set_callback(session_, CaptureResult, &state);

    const char payload[] = "data";
    text_recognition_recognize_text_from_image_data_async_with_request_id(session_, "request-002", payload,
                                                                          sizeof(payload));
    ASSERT_TRUE(WaitForCallback(&state, 2000));

    EXPECT_EQ(state.result.request_id, "request-002");
    EXPECT_EQ(state.result.text, "hello");
    EXPECT_EQ(state.result.error_code, AISDK_NO_ERROR);
}

TEST_F(TextRecognitionRequestIdTest, AutoGeneratedRequestIdFromImageData) {
    CallbackState state;
    text_recognition_result_set_callback(session_, CaptureResult, &state);

    const char payload[] = "data";
    text_recognition_recognize_text_from_image_data_async_with_request_id(session_, nullptr, payload,
                                                                          sizeof(payload));
    ASSERT_TRUE(WaitForCallback(&state, 2000));

    EXPECT_FALSE(state.result.request_id.empty());
    EXPECT_TRUE(state.result.request_id.rfind("req-", 0) == 0);
}

TEST_F(TextRecognitionRequestIdTest, EmptyRequestIdForLegacyApi) {
    CallbackState state;
    text_recognition_result_set_callback(session_, CaptureResult, &state);

    const char payload[] = "data";
    text_recognition_recognize_text_from_image_data_async(session_, payload, sizeof(payload));
    ASSERT_TRUE(WaitForCallback(&state, 2000));

    EXPECT_TRUE(state.result.request_id.empty());
}

TEST_F(TextRecognitionRequestIdTest, ConsecutiveRequestsReturnIds) {
    MultiCallbackState state;
    state.expected_count = 3;

    auto multi_callback = [](TextRecognitionResult *result, void *user_data) {
        auto *multi_state = static_cast<MultiCallbackState *>(user_data);
        CallbackResult snapshot;
        if (result != nullptr) {
            const char *request_id = text_recognition_result_get_request_id(result);
            snapshot.request_id = request_id ? request_id : "";
            snapshot.error_code = text_recognition_result_get_error_code(result);
        }
        {
            std::lock_guard<std::mutex> lock(multi_state->mutex);
            multi_state->results.push_back(std::move(snapshot));
        }
        multi_state->cv.notify_one();
    };

    text_recognition_result_set_callback(session_, multi_callback, &state);

    const char payload[] = "data";
    text_recognition_recognize_text_from_image_data_async_with_request_id(session_, "request-101", payload,
                                                                          sizeof(payload));
    text_recognition_recognize_text_from_image_data_async_with_request_id(session_, "request-102", payload,
                                                                          sizeof(payload));
    text_recognition_recognize_text_from_image_data_async_with_request_id(session_, "request-103", payload,
                                                                          sizeof(payload));

    ASSERT_TRUE(WaitForCallbacks(&state, 2000));
    ASSERT_EQ(state.results.size(), 3u);
    EXPECT_EQ(state.results[0].request_id, "request-101");
    EXPECT_EQ(state.results[1].request_id, "request-102");
    EXPECT_EQ(state.results[2].request_id, "request-103");
    EXPECT_EQ(state.results[0].error_code, AISDK_NO_ERROR);
    EXPECT_EQ(state.results[1].error_code, AISDK_NO_ERROR);
    EXPECT_EQ(state.results[2].error_code, AISDK_NO_ERROR);
}
