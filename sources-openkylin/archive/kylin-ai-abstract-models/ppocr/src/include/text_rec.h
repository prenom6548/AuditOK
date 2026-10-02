#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <numeric>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
//#include <cuda_provider_factory.h>
#include <onnxruntime_cxx_api.h>

using namespace cv;
using namespace std;
using namespace Ort;

class TextRecognizer
{
public:
	TextRecognizer(const char* model_location);
	std::pair<string, float> predict_text(Mat cv_image);

private:
	Mat preprocess(Mat srcimg);
	void normalize_(Mat img);
	const int iShortSideThresh = 3;
	const int inpWidth = 2048;
    const int inpHeight = 48;
	bool bUseStableSize = false;
	bool bMaxSizeLimit = true;

	vector<float> input_image_;
	vector<string> alphabet;
	int names_len;
	vector<int> preb_label;
	Env env = Env(ORT_LOGGING_LEVEL_ERROR, "CRNN");
	Ort::Session* ort_session = nullptr;
	SessionOptions sessionOptions = SessionOptions();
	vector<std::string> input_names;
	vector<std::string> output_names;
	vector<vector<int64_t>> input_node_dims; // >=1 outputs
	vector<vector<int64_t>> output_node_dims; // >=1 outputs
};