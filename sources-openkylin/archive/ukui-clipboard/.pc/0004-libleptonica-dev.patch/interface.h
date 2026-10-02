#ifndef INTERFACE_H
#define INTERFACE_H

#include <vector>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <fstream>
#include <boundscheck/securec.h>
#include <boundscheck/securectype.h>

namespace  {

std::vector<std::string> getText(tesseract::TessBaseAPI *api) {
    std::vector<std::string> textResult;
    if (!api) {
        fprintf(stderr, "ocr get text error: api is nullptr !\n");
        return textResult;
    }
    Boxa *boxes = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
    if (!boxes) {
        fprintf(stderr, "ocr get text error: boxes is nullptr !\n");
        return textResult;
    }
    for (int i = 0; i < boxes->n; ++i) {
        BOX *box = boxaGetBox(boxes, i, L_CLONE);
        if (!box) {
            fprintf(stderr, "ocr get text error: box is nullptr !\n");
            return textResult;
        }

        // 文字识别
        api->SetRectangle(box->x, box->y-1, box->w, box->h+1);
        char* ocrResult = api->GetUTF8Text();
        textResult.push_back(std::string(ocrResult));
        delete [] ocrResult;
        boxDestroy(&box);
    }
    boxDestroy(boxes->box);
    delete boxes;
    return textResult;
}

std::vector<std::string> getImageText(const char *imageData, int size) {
    if (imageData == nullptr) {
        fprintf(stderr, "ocr get image text error: imageData is nullptr !\n");
        return std::vector<std::string>();
    }
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();

    //返回表示0初始化成功，非0值表示不同类型的错误
    if (api->Init(NULL, "chi_sim") && api->Init(NULL, "eng")) {
        fprintf(stderr, "ocr get image text error: TessBaseAPI init failed !\n");
        return std::vector<std::string>();
    }

    Pix *image = pixReadMem(reinterpret_cast<const l_uint8*>(imageData), size);
    if (!image) {
        fprintf(stderr, "ocr get image text error: image data is nullptr !\n");
        pixDestroy(&image);
        api->End();
        delete api;
        return std::vector<std::string>();
    }
    api->SetImage(image);

    std::vector<std::string> text = getText(api);

    pixDestroy(&image);
    api->End();
    delete api;
    return text;
}
}

const char* getTextFromImageFile(const char* image_file) {
    std::fstream file(image_file);
    if (!file.is_open()) {
        fprintf(stderr, "ocr get text from image file error: image file not exists!\n");
        return nullptr;
    }

    std::string resultString;

    std::ifstream is(image_file, std::ifstream::in | std::ios::binary);
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);
    char *buffer = new char[length];
    is.read(buffer, length);

    std::vector<std::string> result = getImageText(buffer, length);
    is.clear();
    for(int i = 0; i < result.size(); i++){
        resultString += result.at(i);
    }

    size_t lenght = strlen(resultString.c_str())+1;
    char * strc = new char[lenght];
    //strcpy(strc, resultString.c_str());
    strcpy_s(strc, lenght, resultString.c_str());

    return strc;
}

const char* getTextFromImageData(const char* image_data,
                                 unsigned int image_data_length) {
    std::string resultString;
    if (image_data == nullptr || image_data_length == 0) {
        fprintf(stderr, "ocr get text from image data error: image data is nullptr!\n");
        return nullptr;
    }

    std::vector<std::string> result = getImageText(image_data, image_data_length);
    for(int i = 0; i < result.size(); i++){
        resultString += result.at(i);
    }

    size_t lenght = strlen(resultString.c_str())+1;
    char * strc = new char[lenght];
    //strcpy(strc, resultString.c_str());
    strcpy_s(strc, lenght, resultString.c_str());

    return strc;
}

#endif // INTERFACE_H
