#include"text_rec.h"

TextRecognizer::TextRecognizer(const char* model_location)
{
	string model_path = model_location;
	ifstream ifs(model_path + "ppocr_keys_v1.txt");
	model_path +=  + "model_rec.onnx";

	sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_ALL);
	sessionOptions.SetIntraOpNumThreads(4);

	ort_session = new Session(env, model_path.c_str(), sessionOptions);
	size_t numInputNodes = ort_session->GetInputCount();
	size_t numOutputNodes = ort_session->GetOutputCount();
	for (size_t i = 0; i < numInputNodes; i++) 
	{
    	input_names.push_back(std::string(""));

	}
	for (size_t i = 0; i < numOutputNodes; i++) 
	{
    	output_names.push_back(std::string(""));
	}

	AllocatorWithDefaultOptions allocator;
	for (size_t i = 0; i < numInputNodes; i++)
	{
		auto temp_input_name = ort_session->GetInputNameAllocated(i, allocator);
		input_names[i].append(temp_input_name.get());
		Ort::TypeInfo input_type_info = ort_session->GetInputTypeInfo(i);
		auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
		auto input_dims = input_tensor_info.GetShape();
		input_node_dims.push_back(input_dims);
	}
	for (size_t i = 0; i < numOutputNodes; i++)
	{
		auto temp_output_name = ort_session->GetOutputNameAllocated(i, allocator);
		output_names[i].append(temp_output_name.get());
		Ort::TypeInfo output_type_info = ort_session->GetOutputTypeInfo(i);
		auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
		auto output_dims = output_tensor_info.GetShape();
		output_node_dims.push_back(output_dims);
	}

	string line;
	while (getline(ifs, line))
	{
		this->alphabet.push_back(line);
	}
	this->alphabet.push_back(" ");
	names_len = this->alphabet.size();
}

Mat TextRecognizer::preprocess(Mat srcimg) {
    Mat dstimg;
    int h = srcimg.rows;
    int w = srcimg.cols;
    const float ratio = w / float(h);
    int resized_w = int(ceil((float)this->inpHeight * ratio));

    if (this->bMaxSizeLimit) {
        if (ceil(this->inpHeight * ratio) > this->inpWidth) {
            resized_w = this->inpWidth;
        }
    }

    resize(srcimg, dstimg, Size(resized_w, this->inpHeight), INTER_LINEAR);
    return dstimg;
}

void TextRecognizer::normalize_(Mat img) {
    int row = img.rows;
    int col = img.cols;
    int resize_w = 0;
    int resize_h = 0;
    if (0) {
        resize_w = this->inpWidth;
        resize_h = this->inpHeight;
    } else {
        resize_w = col;
        resize_h = row;
    }

    this->input_image_.resize(resize_h * resize_w * img.channels());
    for (int c = 0; c < 3; c++) {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < resize_w; j++) {
                if (j < col) {
                    float pix = img.ptr<uchar>(i)[j * 3 + c];
                    this->input_image_[c * row * resize_w + i * resize_w + j] =
                        (pix / 255.0 - 0.5) / 0.5;
                } else {
                    this->input_image_[c * row * resize_w + i * resize_w + j] =
                        0;
                }
            }
        }
    }
}

std::pair<string, float> TextRecognizer::predict_text(Mat cv_image) {
    std::pair<string, float> rec_result;
    std::vector<float> byte_scores;
    Mat dstimg = this->preprocess(cv_image);
    if (dstimg.cols < this->inpHeight || dstimg.rows < this->inpHeight) {
        rec_result.first = "";
        rec_result.second = 0.0f;
        return rec_result;
    }
    this->normalize_(dstimg);
    array<int64_t, 4> input_shape_{};
    if (0) {
        input_shape_ = {1, 3, this->inpHeight, this->inpWidth};
    } else {
        input_shape_ = {1, 3, dstimg.rows, dstimg.cols};
    }

    auto allocator_info =
        MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    Value input_tensor_ = Value::CreateTensor<float>(
        allocator_info, input_image_.data(), input_image_.size(),
        input_shape_.data(), input_shape_.size());

    // 开始推理
    // ector<Value> ort_outputs = ort_session->Run(RunOptions{ nullptr },
    // &input_names[0], &input_tensor_, 1, output_names.data(),
    // output_names.size());   // 开始推理

    const std::array<const char *, 1> inputNames = {input_names[0].c_str()};
    const std::array<const char *, 1> outNames = {output_names[0].c_str()};

    std::vector<Ort::Value> ort_outputs;
    try {
        ort_outputs = ort_session->Run(Ort::RunOptions{nullptr},
                                       inputNames.data(), &input_tensor_, 1,
                                       outNames.data(), outNames.size());
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    const float *pdata = ort_outputs[0].GetTensorMutableData<float>();

    int i = 0, j = 0;
    int h = ort_outputs.at(0).GetTensorTypeAndShapeInfo().GetShape().at(2);
    int w = ort_outputs.at(0).GetTensorTypeAndShapeInfo().GetShape().at(1);

    preb_label.resize(w);
    for (i = 0; i < w; i++) {
        int one_label_idx = 0;
        float max_data = -10000;
        for (j = 0; j < h; j++) {
            float data_ = pdata[i * h + j];
            if (data_ > max_data) {
                max_data = data_;
                one_label_idx = j;
            }
        }
        byte_scores.push_back(max_data);
        preb_label[i] = one_label_idx;
    }

    vector<int> no_repeat_blank_label;
    for (int elementIndex = 0; elementIndex < w; ++elementIndex) {
        if (preb_label[elementIndex] != 0 &&
            !(elementIndex > 0 &&
              preb_label[elementIndex - 1] == preb_label[elementIndex])) {
            no_repeat_blank_label.push_back(preb_label[elementIndex] - 1);
        }
    }

    int len_s = no_repeat_blank_label.size();
    string plate_text;
    for (i = 0; i < len_s; i++) {
        plate_text += alphabet[no_repeat_blank_label[i]];
    }
    rec_result.first = plate_text;
    for (size_t i = 0; i < byte_scores.size(); i++) {
        rec_result.second += byte_scores[i];
    }
    if (byte_scores.size() > 0) {
        rec_result.second = rec_result.second / (float)byte_scores.size();
    }

    return rec_result;
}
