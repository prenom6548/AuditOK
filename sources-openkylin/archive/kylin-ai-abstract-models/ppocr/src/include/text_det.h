#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
//#include <cuda_provider_factory.h>
#include <onnxruntime_cxx_api.h>
#include <map>
#include <array>
using namespace cv;
using namespace std;
using namespace Ort;

class TextDetector
{
public:

	TextDetector(const char* model_location);
	vector< vector<Point2f> > detect(Mat& srcimg);
	void draw_pred(Mat& srcimg, vector< vector<Point2f> > results);
	Mat get_rotate_crop_image(const Mat& frame, vector<Point2f> vertices);

	// const int MERGE_THRESHOLD = 10; // 可以调整这个阈值
    //  定义目标框结构体
    struct BoundingBox {
        Point2f topLeft;
        Point2f bottomRight;

        // 构造函数
        BoundingBox(Point2f tl, Point2f br) : topLeft(tl), bottomRight(br) {}

        // 检查两个框是否可以合并
        bool canMerge(const BoundingBox &other) const {

            bool xmerge = false;
            int iMaxHeight = max(abs(topLeft.y - bottomRight.y),
                                 abs(other.topLeft.y - other.bottomRight.y));
            int iMinHeight = min(abs(topLeft.y - bottomRight.y),
                                 abs(other.topLeft.y - other.bottomRight.y));
            int iMergeTh_x = iMinHeight;
            int iMergeTh_y = iMaxHeight / 2;

            if (topLeft.x < other.topLeft.x) {
                xmerge = (bottomRight.x + iMergeTh_x > other.topLeft.x);
            } else {
                xmerge = (topLeft.x - iMergeTh_x < other.bottomRight.x);
            }
            return (fabs(topLeft.y - other.topLeft.y) <= iMergeTh_y) && xmerge;
        }
    };

    std::vector<std::vector<Point2f>>
    mergeBoundingBoxes(const std::vector<std::vector<Point2f>> &detections);


private:
	float binaryThreshold;
	float polygonThreshold;
	float unclipRatio;
	int maxCandidates;
	const int longSideThresh = 3;
	const int short_size = 832;
    const int long_size = 1440;

	const float meanValues[3] = { 0.485, 0.456, 0.406 };
	const float normValues[3] = { 0.229, 0.224, 0.225 };
	float contourScore(const Mat& binary, const vector<Point>& contour);
	void unclip(const vector<Point2f>& inPoly, vector<Point2f>& outPoly, Mat &src);
	vector< vector<Point2f> > order_points_clockwise(vector< vector<Point2f> > results);
	Mat preprocess(Mat srcimg);
	vector<float> input_image_;
	void normalize_(Mat img);

	Session* net;
	Env env = Env(ORT_LOGGING_LEVEL_ERROR, "DBNet");
	SessionOptions sessionOptions = SessionOptions();
	vector<string> input_names;
	vector<string> output_names;
};
