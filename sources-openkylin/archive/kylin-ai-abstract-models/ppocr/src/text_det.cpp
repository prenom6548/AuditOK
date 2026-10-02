#include "text_det.h"

TextDetector::TextDetector(const char* model_location)
{
	this->binaryThreshold = 0.3;
	this->polygonThreshold = 0.5;
	this->unclipRatio = 1.6;
	this->maxCandidates = 1000;

	string model_path = model_location;
	model_path +=  + "model_det.onnx";


	//std::wstring widestr = std::wstring(model_path.begin(), model_path.end());
	//OrtStatus* status = OrtSessionOptionsAppendExecutionProvider_CUDA(sessionOptions, 0);  ////gpu
	sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_ALL);
	sessionOptions.SetIntraOpNumThreads(4);
	net = new Session(env, model_path.c_str(), sessionOptions);
	size_t numInputNodes = net->GetInputCount();
	size_t numOutputNodes = net->GetOutputCount();
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
		auto temp_input_name = net->GetInputNameAllocated(i, allocator);
		input_names[i].append(temp_input_name.get());
	}
	for (size_t i = 0; i < numOutputNodes; i++)
	{
		auto temp_output_name = net->GetOutputNameAllocated(i, allocator);
		output_names[i].append(temp_output_name.get());
	}
}

Mat TextDetector::preprocess(Mat srcimg)
{
	Mat dstimg;
	cvtColor(srcimg, dstimg, COLOR_BGR2RGB);
	int h = srcimg.rows;
	int w = srcimg.cols;
	float scale_h = 1;
	float scale_w = 1;
	int iDst_w = 0;
	int iDst_h = 0;
	if (h < w)
	{
		scale_h = (float)this->short_size / (float)h;
		scale_h = min(scale_h, 4.00f);
		iDst_w = scale_h * dstimg.cols;
		iDst_w = iDst_w - (int)iDst_w % 32 + 32;
		iDst_h = scale_h * dstimg.rows;
		iDst_h = iDst_h - (int)iDst_h % 32 + 32;
	}
	else
	{
		scale_w = (float)this->short_size / (float)w;
		scale_w = min(scale_w, 4.00f);
		iDst_w = scale_w * dstimg.cols;
		iDst_w = iDst_w - (int)iDst_w % 32 + 32;
		iDst_h = scale_w * dstimg.rows;
		iDst_h = iDst_h - (int)iDst_h % 32 + 32;
	}

	resize(dstimg, dstimg, Size(int(iDst_w), int(iDst_h)), INTER_LINEAR);
	return dstimg;
}

void TextDetector::normalize_(Mat img)
{
	int row = img.rows;
	int col = img.cols;
	this->input_image_.resize(row * col * img.channels());
	for (int c = 0; c < 3; c++)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				float pix = img.ptr<uchar>(i)[j * 3 + c];
				this->input_image_[c * row * col + i * col + j] = (pix / 255.0 - this->meanValues[c]) / this->normValues[c];
			}
		}
	}
}

// 合并目标框的函数
std::vector<std::vector<Point2f>> TextDetector::mergeBoundingBoxes(
    const std::vector<std::vector<Point2f>> &detections) {
    std::vector<TextDetector::BoundingBox> boxes;

    // 将检测结果转换为BoundingBox结构
    for (const auto &detection : detections) {
        if (!detection.empty()) {
            Point2f topLeft(detection[0].x, detection[0].y);
            Point2f bottomRight(detection[2].x, detection[2].y);

            BoundingBox box(topLeft, bottomRight);
            boxes.push_back(box);
        }
    }

    // 排序目标框
    std::sort(boxes.begin(), boxes.end(),
              [](const BoundingBox &a, const BoundingBox &b) {
                  return a.topLeft.x < b.topLeft.x;
              });
    std::vector<BoundingBox> mergedBoxes;

    for (size_t i = 0; i < boxes.size(); i++) {
        bool bMerged = false;
        if (mergedBoxes.empty()) {
            mergedBoxes.push_back(boxes[i]);
        } else {
            for (size_t j = 0; j < mergedBoxes.size(); j++) {
                if (mergedBoxes[j].canMerge(boxes[i])) {
                    // 更新框的右下角坐标
                    mergedBoxes[j].bottomRight.x = std::max(
                        mergedBoxes[j].bottomRight.x, boxes[i].bottomRight.x);
                    mergedBoxes[j].bottomRight.y = std::max(
                        mergedBoxes[j].bottomRight.y, boxes[i].bottomRight.y);

                    bMerged = true;
                    break;
                }
            }
            if (bMerged == false) {
                mergedBoxes.push_back(boxes[i]);
            }
        }
    }

    std::map<int, std::vector<BoundingBox>> boxesByRow;

    // 将BoundingBox对象按行分组
    int row_index = 0;
    for (const auto &box : mergedBoxes) {
        if (boxesByRow.empty()) {
            row_index = box.topLeft.y;
            boxesByRow[row_index].push_back(box);
        } else {
            int iHeight_th = 0;
            int iFindRow = 0;
            for (const auto &row : boxesByRow) {
                if ((box.bottomRight.y - box.topLeft.y) <
                    (row.second.front().bottomRight.y -
                     row.second.front().topLeft.y)) {
                    iHeight_th = (row.second.back().bottomRight.y -
                                  row.second.front().topLeft.y) /
                                 3;
                } else {
                    iHeight_th = (box.bottomRight.y - box.topLeft.y) / 3;
                }
                if (fabs(box.topLeft.y - row.second.front().topLeft.y) <
                    iHeight_th) {
                    boxesByRow[row.first].push_back(box);
                    iFindRow = 1;
                    break;
                }
            }

            if (iFindRow == 0) {
                row_index = box.topLeft.y;
                boxesByRow[row_index].push_back(box);
            }
        }
    }

    // 清空mergedBoxes，准备重新填充
    mergedBoxes.clear();

    // 对每一行进行排序并合并回mergedBoxes
    for (auto &row : boxesByRow) {
        std::sort(row.second.begin(), row.second.end(),
                  [](const BoundingBox &a, const BoundingBox &b) {
                      return a.topLeft.x < b.topLeft.x;
                  });
        // 合并排序好的行
        mergedBoxes.insert(mergedBoxes.end(), row.second.begin(),
                           row.second.end());
    }

    std::vector<std::vector<Point2f>> results;
    // 结果转换为std::vector<Point2f>
    for (auto &mergebox : mergedBoxes) {
        std::vector<Point2f> box;
        Point2f p1 = mergebox.topLeft;
        Point2f p2(mergebox.bottomRight.x, mergebox.topLeft.y);
        Point2f p3(mergebox.topLeft.x, mergebox.bottomRight.y);
        Point2f p4 = mergebox.bottomRight;

        box.push_back(p3);
        box.push_back(p1);
        box.push_back(p2);
        box.push_back(p4);

        results.push_back(box);
    }

    return results;
}

vector< vector<Point2f> > TextDetector::detect(Mat& srcimg)
{
	int h = srcimg.rows;
	int w = srcimg.cols;
	Mat dstimg = this->preprocess(srcimg);
	this->normalize_(dstimg);
	array<int64_t, 4> input_shape_{ 1, 3, dstimg.rows, dstimg.cols };

	auto allocator_info = MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	Value input_tensor_ = Value::CreateTensor<float>(allocator_info, input_image_.data(), input_image_.size(), input_shape_.data(), input_shape_.size());

	const std::array<const char*, 1> inputNames = { input_names[0].c_str() };
	const std::array<const char*, 1> outNames = { output_names[0].c_str() };

 	std::vector<Ort::Value> ort_outputs;
    try {
        ort_outputs = net->Run(Ort::RunOptions{ nullptr }, inputNames.data(), &input_tensor_, 1, outNames.data(), outNames.size());
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

	const float* floatArray = ort_outputs[0].GetTensorMutableData<float>();
	int outputCount = 1;
	for (size_t i = 0; i < ort_outputs.at(0).GetTensorTypeAndShapeInfo().GetShape().size(); i++)
	{
		int dim = ort_outputs.at(0).GetTensorTypeAndShapeInfo().GetShape().at(i);
		outputCount *= dim;
	}

	Mat binary(dstimg.rows, dstimg.cols, CV_32FC1);
	memcpy(binary.data, floatArray, outputCount * sizeof(float));

	// Threshold
	Mat bitmap;
	threshold(binary, bitmap, binaryThreshold, 255, THRESH_BINARY);

	// Scale ratio
	float scaleHeight = (float)(h) / (float)(binary.size[0]);
	float scaleWidth = (float)(w) / (float)(binary.size[1]);
	// Find contours
	vector< vector<Point> > contours;

	bitmap.convertTo(bitmap, CV_8UC1);
	findContours(bitmap, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	// Candidate number limitation
	size_t numCandidate = min(contours.size(), (size_t)(maxCandidates > 0 ? maxCandidates : INT_MAX));
	vector<float> confidences;
	vector< vector<Point2f> > results;
	for (size_t i = 0; i < numCandidate; i++)
	{
		vector<Point>& contour = contours[i];

		// Calculate text contour score
		if (contourScore(binary, contour) < polygonThreshold)
			continue;

		// Rescale
		vector<Point> contourScaled; contourScaled.reserve(contour.size());
		for (size_t j = 0; j < contour.size(); j++)
		{
			contourScaled.push_back(Point(int(contour[j].x * scaleWidth),
				int(contour[j].y * scaleHeight)));
		}

		// Unclip
		RotatedRect box = minAreaRect(contourScaled);
		float longSide = std::max(box.size.width, box.size.height);
		float shortSide = std::min(box.size.width, box.size.height);
		if (longSide < longSideThresh || shortSide < longSideThresh)
		{
			continue;
		}

		// minArea() rect is not normalized, it may return rectangles with angle=-90 or height < width
		const float angle_threshold = 60;  // do not expect vertical text, TODO detection algo property
		bool swap_size = false;
		if (box.size.width < box.size.height)  // horizontal-wide text area is expected
			swap_size = true;
		else if (fabs(box.angle) >= angle_threshold)  // don't work with vertical rectangles
			swap_size = true;
		if (swap_size)
		{
			swap(box.size.width, box.size.height);
			if (box.angle < 0)
				box.angle += 90;
			else if (box.angle > 0)
				box.angle -= 90;
		}

		Point2f vertex[4];
		box.points(vertex);  // order: bl, tl, tr, br
		vector<Point2f> approx;
		for (int j = 0; j < 4; j++)
			approx.emplace_back(vertex[j]);
		vector<Point2f> polygon;
		unclip(approx, polygon, srcimg);

		box = minAreaRect(polygon);
		longSide = std::max(box.size.width, box.size.height);
		if (longSide < longSideThresh + 2)
		{
			continue;
		}

		results.push_back(polygon);
	}
	confidences = vector<float>(contours.size(), 1.0f);
	vector<vector<Point2f>> order_points = this->order_points_clockwise(results);
    return order_points;
	/*vector< vector<Point2f> > order_points = this->order_points_clockwise(results);
	return order_points;*/
}

vector< vector<Point2f> > TextDetector::order_points_clockwise(vector< vector<Point2f> > results)
{
	vector< vector<Point2f> > order_points(results);
	for (size_t i = 0; i < results.size(); i++)
	{
		float max_sum_pts = -10000;
		float min_sum_pts = 10000;
		float max_diff_pts = -10000;
		float min_diff_pts = 10000;

		int max_sum_pts_id = 0;
		int min_sum_pts_id = 0;
		int max_diff_pts_id = 0;
		int min_diff_pts_id = 0;
		for (int j = 0; j < 4; j++)
		{
			const float sum_pt = results[i][j].x + results[i][j].y;
			if (sum_pt > max_sum_pts)
			{
				max_sum_pts = sum_pt;
				max_sum_pts_id = j;
			}
			if (sum_pt < min_sum_pts)
			{
				min_sum_pts = sum_pt;
				min_sum_pts_id = j;
			}

			const float diff_pt = results[i][j].y - results[i][j].x;
			if (diff_pt > max_diff_pts)
			{
				max_diff_pts = diff_pt;
				max_diff_pts_id = j;
			}
			if (diff_pt < min_diff_pts)
			{
				min_diff_pts = diff_pt;
				min_diff_pts_id = j;
			}
		}
		order_points[i][0].x = results[i][min_sum_pts_id].x;
		order_points[i][0].y = results[i][min_sum_pts_id].y;
		order_points[i][2].x = results[i][max_sum_pts_id].x;
		order_points[i][2].y = results[i][max_sum_pts_id].y;

		order_points[i][1].x = results[i][min_diff_pts_id].x;
		order_points[i][1].y = results[i][min_diff_pts_id].y;
		order_points[i][3].x = results[i][max_diff_pts_id].x;
		order_points[i][3].y = results[i][max_diff_pts_id].y;
	}
	return order_points;
}

void TextDetector::draw_pred(Mat& srcimg, vector< vector<Point2f> > results)
{
	for (size_t i = 0; i < results.size(); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			circle(srcimg, Point((int)results[i][j].x, (int)results[i][j].y), 2, Scalar(0, 0, 255), -1);
			if (j < 3)
			{
				line(srcimg, Point((int)results[i][j].x, (int)results[i][j].y), Point((int)results[i][j + 1].x, (int)results[i][j + 1].y), Scalar(0, 255, 0));
			}
			else
			{
				line(srcimg, Point((int)results[i][j].x, (int)results[i][j].y), Point((int)results[i][0].x, (int)results[i][0].y), Scalar(0, 255, 0));
			}
		}
	}
}

float TextDetector::contourScore(const Mat& binary, const vector<Point>& contour)
{
	Rect rect = boundingRect(contour);
	int xmin = max(rect.x, 0);
	int xmax = min(rect.x + rect.width, binary.cols - 1);
	int ymin = max(rect.y, 0);
	int ymax = min(rect.y + rect.height, binary.rows - 1);

	Mat binROI = binary(Rect(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1));

	Mat mask = Mat::zeros(ymax - ymin + 1, xmax - xmin + 1, CV_8U);
	vector<Point> roiContour;
	for (size_t i = 0; i < contour.size(); i++) {
		Point pt = Point(contour[i].x - xmin, contour[i].y - ymin);
		roiContour.push_back(pt);
	}
	vector<vector<Point>> roiContours = { roiContour };
	fillPoly(mask, roiContours, Scalar(1));
	float score = mean(binROI, mask).val[0];
	return score;
}

void TextDetector::unclip(const vector<Point2f>& inPoly, vector<Point2f>& outPoly, Mat &srcimg)
{
	float area = contourArea(inPoly);
	float length = arcLength(inPoly, true);
	float distance = area * unclipRatio / length;

	size_t numPoints = inPoly.size();
	vector<vector<Point2f>> newLines;
	for (size_t i = 0; i < numPoints; i++)
	{
		vector<Point2f> newLine;
		Point pt1 = inPoly[i];
		Point pt2 = inPoly[(i - 1) % numPoints];
		Point vec = pt1 - pt2;
		float unclipDis = (float)(distance / norm(vec));
		Point2f rotateVec = Point2f(vec.y * unclipDis, -vec.x * unclipDis);
		newLine.push_back(Point2f(pt1.x + rotateVec.x, pt1.y + rotateVec.y));
		newLine.push_back(Point2f(pt2.x + rotateVec.x, pt2.y + rotateVec.y));
		newLines.push_back(newLine);
	}

	size_t numLines = newLines.size();
	for (size_t i = 0; i < numLines; i++)
	{
		Point2f a = newLines[i][0];
		Point2f b = newLines[i][1];
		Point2f c = newLines[(i + 1) % numLines][0];
		Point2f d = newLines[(i + 1) % numLines][1];
		Point2f pt;
		Point2f v1 = b - a;
		Point2f v2 = d - c;
		float cosAngle = (v1.x * v2.x + v1.y * v2.y) / (norm(v1) * norm(v2));

		if (fabs(cosAngle) > 0.7)
		{
			pt.x = (b.x + c.x) * 0.5;
			pt.y = (b.y + c.y) * 0.5;
		}
		else
		{
			float denom = a.x * (float)(d.y - c.y) + b.x * (float)(c.y - d.y) +
				d.x * (float)(b.y - a.y) + c.x * (float)(a.y - b.y);
			float num = a.x * (float)(d.y - c.y) + c.x * (float)(a.y - d.y) + d.x * (float)(c.y - a.y);
			float s = num / denom;

			pt.x = a.x + s * (b.x - a.x);
			pt.y = a.y + s * (b.y - a.y);
		}
		if (pt.x < 0)
            pt.x = 0;
        if (pt.x > srcimg.cols)
            pt.x = srcimg.cols - 1;

        if (pt.y < 0)
            pt.y = 0;
        if (pt.y > srcimg.rows)
            pt.y = srcimg.rows - 1;
		outPoly.push_back(pt);
	}
}

Mat TextDetector::get_rotate_crop_image(const Mat& frame, vector<Point2f> vertices)
{
	Rect rect = boundingRect(Mat(vertices));
	if (rect.x < 0) rect.x = 0;
	if (rect.x + rect.width > frame.cols) rect.x = frame.cols - rect.width;
	if (rect.y < 0) rect.y = 0;
	if (rect.y + rect.height > frame.rows) rect.y = frame.rows - rect.height;
	Mat crop_img = frame(rect);

	const Size outputSize = Size(rect.width, rect.height);

	vector<Point2f> targetVertices{ Point2f(0, outputSize.height),Point2f(0, 0), Point2f(outputSize.width, 0), Point2f(outputSize.width, outputSize.height) };
	//vector<Point2f> targetVertices{ Point2f(0, 0), Point2f(outputSize.width, 0), Point2f(outputSize.width, outputSize.height), Point2f(0, outputSize.height) };

	for (int i = 0; i < 4; i++)
	{
		vertices[i].x -= rect.x;
		vertices[i].y -= rect.y;
	}

	Mat rotationMatrix = getPerspectiveTransform(vertices, targetVertices);
	Mat result;
	warpPerspective(crop_img, result, rotationMatrix, outputSize, cv::BORDER_REPLICATE);
	return result;
}