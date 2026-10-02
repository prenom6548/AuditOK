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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "client.h"

#include "universal_image_reader.h"

using namespace std;
namespace tc = triton::client;

namespace ai_engine::core_ai::vision {

// 格式化 masks_list 为 JSON 字符串
std::string format_masks_to_json(const std::vector<MatInfo>& masks_list) {
    nlohmann::json jsonArray = nlohmann::json::array();

    for (size_t i = 0; i < masks_list.size(); i++) {
        nlohmann::json jsonObj;

        const cv::Mat& mask = masks_list[i].mask;

        // 使用 RLE (Run-Length Encoding) 编码以减少数据大小
        std::vector<int> rle;
        bool in_run = false;
        int run_start = 0;
        int run_length = 0;

        for (int y = 0; y < mask.rows; y++) {
            for (int x = 0; x < mask.cols; x++) {
                bool is_foreground = (mask.at<uchar>(y, x) > 0);
                if (is_foreground != in_run) {
                    if (in_run && run_length > 0) {
                        rle.push_back(run_start);
                        rle.push_back(run_length);
                    }
                    in_run = is_foreground;
                    run_start = y * mask.cols + x;
                    run_length = 1;
                } else {
                    run_length++;
                }
            }
        }
        if (in_run && run_length > 0) {
            rle.push_back(run_start);
            rle.push_back(run_length);
        }

        // 存储掩码信息
        jsonObj["mask_width"] = mask.cols;
        jsonObj["mask_height"] = mask.rows;
        jsonObj["mask_rle"] = rle;
        jsonObj["iou_pred"] = masks_list[i].iou_pred;
        jsonObj["stability_score"] = masks_list[i].stability_score;

        // 计算边界框
        std::vector<cv::Point> points;
        cv::findNonZero(mask, points);
        if (!points.empty()) {
            cv::Rect bbox = cv::boundingRect(points);
            jsonObj["bbox"] = {{"x", bbox.x}, {"y", bbox.y}, {"width", bbox.width}, {"height", bbox.height}};
        }

        jsonArray.push_back(jsonObj);
    }

    return jsonArray.dump();
}

std::vector<MatInfo> parse_masks_from_json(const std::string& json_str) {
    std::vector<MatInfo> masks_list;
    printf("json_str:%s\n", json_str.c_str());

    try {
        nlohmann::json jsonArray = nlohmann::json::parse(json_str);

        for (const auto& jsonObj : jsonArray) {
            MatInfo matInfo;

            // 获取掩码尺寸
            int mask_width = jsonObj.value("mask_width", 0);
            int mask_height = jsonObj.value("mask_height", 0);

            // 创建对应尺寸的掩码矩阵
            matInfo.mask = cv::Mat::zeros(mask_height, mask_width, CV_8UC1);

            // 获取 RLE 编码数据并解码
            std::vector<int> rle = jsonObj.value("mask_rle", std::vector<int>());
            for (size_t i = 0; i < rle.size(); i += 2) {
                if (i + 1 < rle.size()) {
                    int start_idx = rle[i];
                    int length = rle[i + 1];

                    // 将索引转换为坐标并设置掩码值
                    for (int j = 0; j < length; j++) {
                        int idx = start_idx + j;
                        int y = idx / mask_width;
                        int x = idx % mask_width;

                        if (y < mask_height && x < mask_width) {
                            matInfo.mask.at<uchar>(y, x) = 255;  // 设置前景像素
                        }
                    }
                }
            }

            // 获取其他属性
            matInfo.iou_pred = jsonObj.value("iou_pred", 0.0f);
            matInfo.stability_score = jsonObj.value("stability_score", 0.0f);

            masks_list.push_back(matInfo);
        }
    } catch (const std::exception& e) {
        // 解析失败时返回空的 masks_list
        std::cerr << "Error parsing masks from JSON: " << e.what() << std::endl;
    }

    return masks_list;
}

// 从掩码提取边界框：找到掩码中所有非零像素的最小外接矩形
cv::Rect mask_to_box(const cv::Mat& mask) {
    std::vector<cv::Point> points;
    cv::findNonZero(mask, points);

    if (points.empty()) {
        return cv::Rect(0, 0, 0, 0);
    }

    cv::Rect bounding_rect = cv::boundingRect(points);
    return bounding_rect;
}

// 移除小区域：移除掩码中的小区域或孔洞
// mode_holes=true: 移除小孔洞（掩码内部的空洞）
// mode_holes=false: 移除小岛屿（掩码外部的小区域）
void remove_small_regions(cv::Mat& mask, int area_thresh, bool mode_holes, bool& changed) {
    if (mask.empty()) {
        changed = false;
        return;
    }

    // 转换为二值图像（如果需要）
    cv::Mat binary_mask;
    if (mask.type() != CV_8UC1) {
        mask.convertTo(binary_mask, CV_8UC1);
    } else {
        binary_mask = mask.clone();
    }

    // 二值化（0或255），使用更严格的阈值确保完全二值化
    cv::Mat thresholded;
    cv::threshold(binary_mask, thresholded, 127, 255, cv::THRESH_BINARY);

    if (mode_holes) {
        // ========== 孔洞模式：填充掩码内部的小孔洞 ==========
        // 反转掩码：将孔洞变成前景，便于查找
        cv::Mat inverted = 255 - thresholded;

        // 查找连通组件（在反转后的掩码中）
        cv::Mat labels, stats, centroids;
        int n_labels = cv::connectedComponentsWithStats(inverted, labels, stats, centroids, 8);

        if (n_labels <= 1) {
            // 没有孔洞，不需要处理
            changed = false;
            return;
        }

        // 找到最大的连通区域（这是原掩码的主体部分）
        int max_area = 0;
        int max_label = 0;
        for (int i = 1; i < n_labels; ++i) {
            int area = stats.at<int>(i, cv::CC_STAT_AREA);
            if (area > max_area) {
                max_area = area;
                max_label = i;
            }
        }

        // 查找小孔洞（排除最大的区域，因为那是掩码主体）
        std::vector<int> small_holes;
        for (int i = 1; i < n_labels; ++i) {
            if (i != max_label) {
                int area = stats.at<int>(i, cv::CC_STAT_AREA);
                if (area < area_thresh) {
                    small_holes.push_back(i);
                }
            }
        }

        if (small_holes.empty()) {
            // 没有小孔洞需要填充
            changed = false;
            return;
        }

        // 创建新掩码：保留最大区域，填充小孔洞
        cv::Mat new_mask = cv::Mat::zeros(mask.size(), CV_8UC1);

        // 保留最大区域（原掩码主体）
        cv::Mat max_region = (labels == max_label);
        new_mask.setTo(255, max_region);

        // 填充小孔洞（将它们也设为255，这样反转后就是前景）
        for (int hole_label : small_holes) {
            cv::Mat hole_region = (labels == hole_label);
            new_mask.setTo(255, hole_region);
        }

        // 反转回来：得到填充了孔洞的原始掩码
        new_mask = 255 - new_mask;

        // 检查掩码是否改变
        cv::Mat diff;
        cv::bitwise_xor(thresholded, new_mask, diff);
        changed = cv::countNonZero(diff) > 0;

        if (changed) {
            mask = new_mask;
        }

    } else {
        // ========== 岛屿模式：移除掩码外部的小区域 ==========
        // 直接使用原掩码查找连通组件
        cv::Mat labels, stats, centroids;
        int n_labels = cv::connectedComponentsWithStats(thresholded, labels, stats, centroids, 8);

        if (n_labels <= 1) {
            // 只有一个区域或没有区域，不需要处理
            changed = false;
            return;
        }

        // 获取每个组件的尺寸
        std::vector<std::pair<int, int>> regions;  // (label, area)
        for (int i = 1; i < n_labels; ++i) {
            int area = stats.at<int>(i, cv::CC_STAT_AREA);
            regions.push_back({i, area});
        }

        // 按面积排序，找到最大的区域
        std::sort(regions.begin(), regions.end(),
                  [](const std::pair<int, int>& a, const std::pair<int, int>& b) { return a.second > b.second; });

        // 查找小区域（排除最大的区域）
        std::vector<int> small_islands;
        for (size_t i = 1; i < regions.size(); ++i) {  // 从第二个开始（跳过最大的）
            if (regions[i].second < area_thresh) {
                small_islands.push_back(regions[i].first);
            }
        }

        if (small_islands.empty()) {
            // 没有小岛屿需要移除
            changed = false;
            return;
        }

        // 创建新掩码：只保留大区域（排除小岛屿）
        cv::Mat new_mask = cv::Mat::zeros(mask.size(), CV_8UC1);

        // 保留所有大区域（包括最大的和大于阈值的小区域）
        for (const auto& region : regions) {
            if (region.second >= area_thresh) {
                cv::Mat region_mask = (labels == region.first);
                new_mask.setTo(255, region_mask);
            }
        }

        // 如果所有区域都小于阈值，至少保留最大的一个
        if (new_mask.empty() || cv::countNonZero(new_mask) == 0) {
            if (!regions.empty()) {
                cv::Mat max_region_mask = (labels == regions[0].first);
                new_mask.setTo(255, max_region_mask);
            }
        }

        // 检查掩码是否改变
        cv::Mat diff;
        cv::bitwise_xor(thresholded, new_mask, diff);
        changed = cv::countNonZero(diff) > 0;

        if (changed) {
            mask = new_mask;
        }
    }
}

// 非极大值抑制（NMS）：根据IoU阈值去除重复的边界框
std::vector<int> nms_boxes(const std::vector<cv::Rect>& boxes, const std::vector<float>& scores, float nms_thresh) {
    if (boxes.empty()) return {};

    std::vector<cv::Rect2d> boxes_d;
    for (const auto& box : boxes) {
        boxes_d.emplace_back(box.x, box.y, box.width, box.height);
    }

    std::vector<int> indices;
    std::vector<float> scores_copy = scores;
    cv::dnn::NMSBoxes(boxes_d, scores_copy, 0.0f, nms_thresh, indices);

    return indices;
}

// 后处理小区域：移除小区域和孔洞，然后对剩余区域进行NMS
std::vector<MatInfo> postprocess_small_regions(const std::vector<MatInfo>& masks, int min_area, float nms_thresh) {
    if (masks.empty()) {
        return masks;
    }

    // 过滤小区域和孔洞
    std::vector<MatInfo> processed_masks;
    std::vector<cv::Rect> new_boxes;
    std::vector<float> iou_scores;  // 使用IoU预测值作为NMS的分数

    for (size_t i = 0; i < masks.size(); ++i) {
        cv::Mat mask = masks[i].mask.clone();

        // 先移除小孔洞（填充掩码内部的空洞）
        bool holes_changed = false;
        remove_small_regions(mask, min_area, true, holes_changed);

        // 再移除小岛屿（删除掩码外部的小区域）
        bool islands_changed = false;
        remove_small_regions(mask, min_area, false, islands_changed);

        // 检查掩码是否还有有效内容
        if (cv::countNonZero(mask) == 0) {
            // 掩码被完全清空，跳过这个掩码
            continue;
        }

        // 创建处理后的MatInfo
        MatInfo processed_mask = masks[i];
        processed_mask.mask = mask;
        processed_masks.push_back(processed_mask);

        // 重新计算边界框（因为掩码可能已改变）
        cv::Rect box = mask_to_box(mask);
        new_boxes.push_back(box);

        // 改进的分数策略：结合IoU和修改状态
        // 未修改的掩码给予小幅加分，但仍以IoU为主
        float base_score = masks[i].iou_pred;
        bool unchanged = !holes_changed && !islands_changed;
        float final_score = unchanged ? base_score + 0.05f : base_score;
        // 确保分数在合理范围内
        final_score = std::min(1.0f, std::max(0.0f, final_score));
        iou_scores.push_back(final_score);
    }

    // 如果所有掩码都被过滤掉了，返回空列表
    if (processed_masks.empty()) {
        return processed_masks;
    }

    // 执行NMS去除重复掩码
    std::vector<MatInfo> final_masks;
    if (nms_thresh > 0.0f && !new_boxes.empty()) {
        std::vector<int> keep = nms_boxes(new_boxes, iou_scores, nms_thresh);
        for (int idx : keep) {
            final_masks.push_back(processed_masks[idx]);
        }
    } else {
        final_masks = processed_masks;
    }

    return final_masks;
}

SegTritonClient::SegTritonClient(const string& url) {
    // Create the Triton client once during object construction
    FAIL_IF_ERR(tc::InferenceServerHttpClient::Create(&client_, url, false), "unable to create http client");
    SegRunning_.store(false, std::memory_order_release);
}

void SegTritonClient::setSegmentationResultCallback(const SegmentationResultCallback& callback) {
    segResultCallback_ = std::move(callback);
}

result::AiEngineBaseResult SegTritonClient::segEncodeRequestByPath(const std::string& imagePath) {
    std::unique_lock<std::mutex> lock(segMutex_);
    SegRunning_.store(true, std::memory_order_release);
    vision::ImageSegmentationResult res;
    result::AiEngineBaseResult error;

    // 使用 ifstream 检查文件是否存在
    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        // 如果文件无法打开，返回错误信息
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_FILE_NOT_FOUND,
                                             "[SegTritonClient]: 文件打开失败。");
        SegRunning_.store(false, std::memory_order_release);
        return error;
    }

    std::vector<uchar> data = readFile(imagePath);
    std::string img_format = getFormatFromData(data);

    cv::Mat src_img = loadImageFromData(data, img_format);
    if (src_img.empty()) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_IMAGE_DATA_INVALID,
                                             "[SegTritonClient]: 图片文件解析异常。");
        SegRunning_.store(false, std::memory_order_release);
        return error;
    }
    res = segEncodeRequestByImage(src_img, img_format);
    ImageSegmentationResult resOut;
    resOut.success = res.success;
    resOut.modelName = res.modelName;
    resOut.width = res.width;
    resOut.height = res.height;
    SegRunning_.store(false, std::memory_order_release);

    return error;
}

result::AiEngineBaseResult SegTritonClient::segEncodeRequestByBase64(const std::string& imageData) {
    std::unique_lock<std::mutex> lock(segMutex_);
    SegRunning_.store(true, std::memory_order_release);
    vision::ImageSegmentationResult res;
    result::AiEngineBaseResult error;
    error.success = true;
    if (imageData.empty()) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_VISION_IMAGE_SIZE_INVALID,
                                             "[SegTritonClient]: 文件为空。");
        SegRunning_.store(false, std::memory_order_release);
        return error;
    }
    std::string decoded_data = base64_decode(imageData.c_str());
    if (decoded_data.size() < 8) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_VISION_IMAGE_SIZE_INVALID,
                                             "[SegTritonClient]: 文件为空。");
        SegRunning_.store(false, std::memory_order_release);
        return error;
    }
    std::vector<uchar> data(decoded_data.begin(), decoded_data.end());
    std::string img_format = getFormatFromData(data);
    cv::Mat image = loadImageFromData(data, img_format);
    if (image.empty()) {
        error.success = false;
        error.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_IMAGE_DATA_INVALID,
                                             "[SegTritonClient]: 图片文件解析异常。");
        SegRunning_.store(false, std::memory_order_release);
        return error;
    }
    res = segEncodeRequestByImage(image, img_format);
    SegRunning_.store(false, std::memory_order_release);

    return error;
}

vision::ImageSegmentationResult SegTritonClient::segEncodeRequestByImage(const cv::Mat& image,
                                                                         const std::string& img_format) {
    vision::ImageSegmentationResult res;
    std::string mask_ptr;
    res = encode(image, mask_ptr);
    if (res.success == true) {
        imgFormat_ = img_format;
        segEncoderData_ = mask_ptr;
    }
    return res;
}

result::AiEngineBaseResult SegTritonClient::segDecodeRequest(const std::vector<std::tuple<int, int, bool>>& point,
                                                             uint32_t minArea) {
    std::unique_lock<std::mutex> lock(segMutex_);
    SegRunning_.store(true, std::memory_order_release);
    result::AiEngineBaseResult error;
    vision::ImageSegmentationResult res;
    error.success = true;
    res = decode(point, minArea);
    if (!res.success) {
        error.success = false;
        error.error = res.error;
        SegRunning_.store(false, std::memory_order_release);
        return error;
    }
    if (SegRunning_.load(std::memory_order_acquire) == true) {
        if (segResultCallback_ != nullptr) {
            segResultCallback_(res);
        }
    }
    SegRunning_.store(false, std::memory_order_release);
    return error;
}

void SegTritonClient::init() {
    modelName_ = getConfigValue("seg", "seg");
    if (modelName_.size() < 1) {
        printf("line:%d model_name.size():%ld\n", __LINE__, modelName_.size());
        modelName_ = "seg_sam";
    }
}

std::string SegTritonClient::getConfigValue(const char* key, const char* engine) {
    tc::Error err = client_->LoadModel("model_bank");
    std::string model_name = "model_bank";
    tc::InferOptions options(model_name);
    options.model_version_ = "";
    options.priority_ = 1;

    std::vector<std::string> input_key_data{std::string(key)};
    std::vector<std::string> input_engine_data{std::string(engine)};

    tc::InferInput *input0, *input1;
    std::vector<int64_t> shape{1};
    tc::InferInput::Create(&input0, "key", shape, "BYTES");
    tc::InferInput::Create(&input1, "engine", shape, "BYTES");
    std::shared_ptr<tc::InferInput> input0_ptr, input1_ptr;
    input0_ptr.reset(input0);
    input1_ptr.reset(input1);
    input0_ptr->AppendFromString(input_key_data);
    input1_ptr->AppendFromString(input_engine_data);
    std::vector<tc::InferInput*> inputs = {input0_ptr.get(), input1_ptr.get()};

    tc::InferRequestedOutput* output0;
    tc::Error err_out0 = tc::InferRequestedOutput::Create(&output0, "value");
    std::shared_ptr<tc::InferRequestedOutput> output0_ptr;
    output0_ptr.reset(output0);
    std::vector<const tc::InferRequestedOutput*> outputs = {output0_ptr.get()};

    tc::InferResult* results;
    tc::Headers http_headers;
    tc::Error err_infer = client_->Infer(&results, options, inputs, outputs, http_headers);
    if (!err_infer.IsOk()) {
        std::cout << __LINE__ << " err_infer.Message(): " << err_infer.Message() << std::endl;
        return "";
    }

    std::shared_ptr<tc::InferResult> results_ptr;
    results_ptr.reset(results);

    char* output0_data_ptr;
    size_t recv_output0_byte_size;

    std::vector<std::string> text_output_data;
    tc::Error err_paser = results_ptr->StringData("value", &text_output_data);
    if (!err_paser.IsOk()) {
        std::cout << __LINE__ << " err_paser.Message(): " << err_infer.Message() << std::endl;
        return "";
    }
    return text_output_data[0];
}

vision::ImageSegmentationResult SegTritonClient::encode(const cv::Mat& srcImg, std::string& mask_ptr) {
    vision::ImageSegmentationResult res;
    bool ready = false;
    tc::Error error = client_->IsModelReady(&ready, modelName_);

    if (!loadModelIfNotAvailable(modelName_)) {
        res.success = false;
        res.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_FAILED, "[Seg] model error");
        return res;
    }
    if (!loadModelIfNotAvailable("seg_sam-decoder")) {
        res.success = false;
        res.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_FAILED, "[Seg] model error");
        return res;
    }
    if (!loadModelIfNotAvailable("seg_sam-encoder")) {
        res.success = false;
        res.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_FAILED, "[Seg] model error");
        return res;
    }

    res.success = true;
    // 准备输入数据
    std::vector<int64_t> encoder_input_shape = {srcImg.rows, srcImg.cols, srcImg.channels()};
    std::vector<int64_t> decoder_input_shape = {1};
    std::vector<int64_t> key_input_shape = {1};
    res.modelName = modelName_;
    res.width = srcImg.cols;
    res.height = srcImg.rows;

    std::vector<uint8_t> input_data(srcImg.total() * srcImg.channels());
    std::memcpy(input_data.data(), srcImg.data, input_data.size());

    char default_data = 0;

    tc::InferInput* encoder_input;
    tc::InferInput* decoder_input;
    tc::InferInput* key_input;

    tc::InferInput::Create(&encoder_input, "encoder_input", encoder_input_shape, "UINT8");
    tc::InferInput::Create(&decoder_input, "decoder_input", decoder_input_shape, "UINT8");
    tc::InferInput::Create(&key_input, "KEY", key_input_shape, "UINT8");

    std::shared_ptr<tc::InferInput> encoder_input_ptr;
    std::shared_ptr<tc::InferInput> decoder_input_ptr;
    std::shared_ptr<tc::InferInput> key_input_ptr;
    encoder_input_ptr.reset(encoder_input);
    encoder_input_ptr->SetBinaryData(true);
    encoder_input_ptr->AppendRaw(reinterpret_cast<const uint8_t*>(input_data.data()), input_data.size());
    decoder_input_ptr.reset(decoder_input);
    decoder_input_ptr->SetBinaryData(true);
    decoder_input_ptr->AppendRaw(reinterpret_cast<const uint8_t*>(&default_data), 1);
    key_input_ptr.reset(key_input);
    key_input_ptr->SetBinaryData(true);
    key_input_ptr->AppendRaw(reinterpret_cast<const uint8_t*>(&default_data), 1);

    tc::InferRequestedOutput* encoder_output;

    tc::InferRequestedOutput::Create(&encoder_output, "encoder_output");

    std::shared_ptr<tc::InferRequestedOutput> encoder_output_ptr;
    encoder_output_ptr.reset(encoder_output);

    std::string model_version = "";
    // The inference settings. Will be using default for now.
    tc::InferOptions options(modelName_);
    options.model_version_ = model_version;
    options.priority_ = 1;

    std::vector<tc::InferInput*> inputs = {encoder_input_ptr.get(), decoder_input_ptr.get(), key_input_ptr.get()};
    std::vector<const tc::InferRequestedOutput*> outputs = {encoder_output_ptr.get()};

    tc::InferResult* results;
    tc::Headers http_headers;
    tc::Error err_infer = client_->Infer(&results, options, inputs, outputs, http_headers);
    if (!err_infer.IsOk()) {
        res.success = false;
        res.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_UNKNOWN_ERROR, "[Seg] Infer error");
        // Infer error "unable to run model"
        std::cout << __LINE__ << " res.errorMessage: " << err_infer.Message() << std::endl;
        return res;
    }
    std::shared_ptr<tc::InferResult> results_ptr;
    results_ptr.reset(results);

    char* data_ptr;
    size_t recv_output0_byte_size;
    results_ptr->RawData("encoder_output", (const uint8_t**)&data_ptr, &recv_output0_byte_size);
    mask_ptr = std::string(data_ptr, recv_output0_byte_size);

    return res;
}

vision::ImageSegmentationResult SegTritonClient::decode(const std::vector<std::tuple<int, int, bool>>& points,
                                                        uint32_t minArea) {
    vision::ImageSegmentationResult res;
    res.success = true;

    if (!loadModelIfNotAvailable(modelName_)) {
        res.success = false;
        res.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_FAILED, "[Seg] model error");
        return res;
    }
    if (!loadModelIfNotAvailable("seg_sam-decoder")) {
        res.success = false;
        res.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_FAILED, "[Seg] model error");
        return res;
    }
    if (!loadModelIfNotAvailable("seg_sam-encoder")) {
        res.success = false;
        res.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_FAILED, "[Seg] model error");
        return res;
    }

    if (segEncoderData_.empty()) {
        res.success = false;
        res.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_IMAGE_DECODE_WITHOUT_ENCODE,
                                           "[Seg] Decode without encode");
        return res;
    }
    // 准备输入数据
    nlohmann::json decode_input;
    nlohmann::json click_points;
    nlohmann::json seg_encoder_data_json = nlohmann::json::parse(segEncoderData_);
    decode_input["image_embedding"] = seg_encoder_data_json["image_embedding"];
    decode_input["image_w"] = seg_encoder_data_json["image_w"];
    decode_input["image_h"] = seg_encoder_data_json["image_h"];
    res.modelName = modelName_;
    res.width = seg_encoder_data_json["image_w"];
    res.height = seg_encoder_data_json["image_h"];

    for (auto& point : points) {
        nlohmann::json point_json;
        point_json["x"] = std::get<0>(point);
        point_json["y"] = std::get<1>(point);
        point_json["positive"] = std::get<2>(point);
        if (std::get<0>(point) > res.width || std::get<1>(point) > res.height || std::get<0>(point) < 0 ||
            std::get<1>(point) < 0) {
            res.success = false;
            res.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_IMAGE_COORD_OUT_OF_BOUNDS,
                                               "[Seg] Coord out of bounds");
        }
        click_points.push_back(point_json);
    }

    decode_input["click_points"] = click_points;
    std::string decode_input_str = decode_input.dump();
    std::vector<uint8_t> input_data(decode_input_str.size());
    std::memcpy(input_data.data(), decode_input_str.data(), decode_input_str.size());
    char default_data = 1;

    std::vector<int64_t> encoder_input_shape = {1, 1, 1};
    std::vector<int64_t> decoder_input_shape = {static_cast<int64_t>(input_data.size())};
    std::vector<int64_t> key_input_shape = {1};

    tc::InferInput* encoder_input;
    tc::InferInput* decoder_input;
    tc::InferInput* key_input;

    tc::InferInput::Create(&encoder_input, "encoder_input", encoder_input_shape, "UINT8");
    tc::InferInput::Create(&decoder_input, "decoder_input", decoder_input_shape, "UINT8");
    tc::InferInput::Create(&key_input, "KEY", key_input_shape, "UINT8");

    std::shared_ptr<tc::InferInput> encoder_input_ptr;
    std::shared_ptr<tc::InferInput> decoder_input_ptr;
    std::shared_ptr<tc::InferInput> key_input_ptr;
    encoder_input_ptr.reset(encoder_input);
    encoder_input_ptr->SetBinaryData(true);
    encoder_input_ptr->AppendRaw(reinterpret_cast<const uint8_t*>(&default_data), 1);
    decoder_input_ptr.reset(decoder_input);
    decoder_input_ptr->SetBinaryData(true);
    decoder_input_ptr->AppendRaw(reinterpret_cast<const uint8_t*>(input_data.data()), input_data.size());
    key_input_ptr.reset(key_input);
    key_input_ptr->SetBinaryData(true);
    key_input_ptr->AppendRaw(reinterpret_cast<const uint8_t*>(&default_data), 1);

    tc::InferRequestedOutput* decoder_output;

    tc::InferRequestedOutput::Create(&decoder_output, "decoder_output");

    std::shared_ptr<tc::InferRequestedOutput> decoder_output_ptr;
    decoder_output_ptr.reset(decoder_output);

    std::string model_version = "";
    // The inference settings. Will be using default for now.
    tc::InferOptions options(modelName_);
    options.model_version_ = model_version;
    options.priority_ = 1;

    std::vector<tc::InferInput*> inputs = {encoder_input_ptr.get(), decoder_input_ptr.get(), key_input_ptr.get()};
    std::vector<const tc::InferRequestedOutput*> outputs = {decoder_output_ptr.get()};

    tc::InferResult* results;
    tc::Headers http_headers;
    tc::Error err_infer = client_->Infer(&results, options, inputs, outputs, http_headers);
    if (!err_infer.IsOk()) {
        res.success = false;
        res.error = ai_engine::EngineError((int)AiSdkCommonErrorCode::AI_COMMON_UNKNOWN_ERROR, "[Seg] Infer error");
        // Infer error "unable to run model"
        std::cout << __LINE__ << " res.errorMessage: " << err_infer.Message() << std::endl;
        return res;
    }
    std::shared_ptr<tc::InferResult> results_ptr;
    results_ptr.reset(results);

    char* mask_ptr;
    size_t recv_output0_byte_size;
    results_ptr->RawData("decoder_output", (const uint8_t**)&mask_ptr, &recv_output0_byte_size);

    nlohmann::json mask_json = nlohmann::json::parse(mask_ptr);
    maskJsonResult_.push_back(mask_json);
    nlohmann::json mask_array_json = nlohmann::json::array();
    for (auto& mask_items : maskJsonResult_) {
        for (auto& mask_item : mask_items) {
            mask_array_json.push_back(mask_item);
        }
    }

    std::vector<MatInfo> mask_list = parse_masks_from_json(mask_array_json.dump());
    mask_list = postprocess_small_regions(mask_list, minArea, 0.7);
    std::string mask_list_str = format_masks_to_json(mask_list);
    nlohmann::json mask_list_str_json = nlohmann::json::parse(mask_list_str);

    res.width = seg_encoder_data_json["image_w"];
    res.height = seg_encoder_data_json["image_h"];
    for (auto& item : mask_list_str_json) {
        const auto& mask_rle_json = item["mask_rle"];
        std::vector<uint32_t> raw_rle = mask_rle_json.get<std::vector<uint32_t>>();
        if (raw_rle.size() % 2 != 0) {
            std::cout << __LINE__ << "error mask_rle size :" << raw_rle.size() << std::endl;
            continue;
        }
        for (size_t i = 0; i < raw_rle.size(); i += 2) {
            res.mask.push_back({.index = raw_rle[i], .count = raw_rle[i + 1]});
        }
    }

    return res;
}

void SegTritonClient::segmentationClearCache() { maskJsonResult_.clear(); }

bool SegTritonClient::loadModelIfNotAvailable(const std::string& modelName) {
    bool ready = false;
    tc::Error error = client_->IsModelReady(&ready, modelName);
    if (!error.IsOk()) {
        std::cerr << "[SegTritonClient]:Triton error: unable to get model "
                     "ready status,"
                  << "error message:" << error.Message() << std::endl;
        return false;
    }
    if (!ready) {
        std::cerr << "[SegTritonClient]Model error: model is not ready, we have to LoadModel "
                     "model : "
                  << modelName << std::endl;
        error = client_->LoadModel(modelName);
        if (!error.IsOk()) {
            std::cerr << "[SegTritonClient]:Triton error: unable to LoadModel model." << modelName
                      << "error message:" << error.Message() << std::endl;
            return false;
        }
    }
    return true;
}

result::AiEngineBaseResult SegTritonClient::stopSegmentation() {
    if (SegRunning_.load(std::memory_order_acquire)) {
        SegRunning_.store(false, std::memory_order_release);
        ImageSegmentationResult result;
        result.success = true;
        result.modelName = modelName_;
        result.width = 0;
        result.height = 0;
        if (segResultCallback_ != nullptr) {
            segResultCallback_(result);
        }
    }
    result::AiEngineBaseResult error;
    return error;
}
}  // namespace ai_engine::core_ai::vision