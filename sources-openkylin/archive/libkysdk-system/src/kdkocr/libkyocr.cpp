/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkyocr.hpp"

kdk::TOcrResult::TOcrResult() : imagePath(""),
                            hasGotRect(false),
                            textRects(),
                            texts()
{
}

void kdk::TOcrResult::reset()
{
    imagePath = "";
    hasGotRect = false;
//    textRects.clear();
//    texts.clear();
    std::vector<std::vector<std::vector<int>>>().swap(textRects);
    std::vector<std::string>().swap(texts);
}

bool kdk::TOcrResult::isNewPath(const std::string &strImagePath) const
{
    return imagePath != strImagePath;
}

kdk::kdkOCR::kdkOCR() : objOcrRet() {

}


#include <iostream>
#include <string>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>



std::vector<std::vector<std::vector<int>>> kdk::kdkOCR::getRect(const std::string &imagePath)
{
    if (imagePath.empty())
    {
        return std::vector<std::vector<std::vector<int>>>();
    }

    handleImage(imagePath);
    
    return objOcrRet.textRects;
}

std::vector<std::string> kdk::kdkOCR::getCls(const std::string &imagePath, int nums)
{
    if (imagePath.empty())
    {
        return std::vector<std::string>();
    }

    handleImage(imagePath);
    return objOcrRet.texts;
}

void kdk::kdkOCR::handleImage(const std::string &imagePath)
{
    if (imagePath.empty())
    {
        return;
    }
    if (!objOcrRet.isNewPath(imagePath))
    {
        if (objOcrRet.hasGotRect)
        {
            return;
        }
    }

    objOcrRet.reset();

    // tesseract识别
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    if (!api->Init(NULL, "chi_sim"))
    {
        Pix *image = pixRead(imagePath.c_str());
        if (image)
        {
            api->SetImage(image);

            Boxa *boxes = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
            if (boxes != nullptr) {
                //std::cout << "boxes cnt : " << boxaGetCount(boxes) << std::endl;
                int boxCount = boxaGetCount(boxes);
                for (int i = 0; i < boxCount; ++i)
                {
                    BOX *box = boxaGetBox(boxes, i, L_CLONE);
                    if (box == nullptr)
                    {
                        continue;
                    }

                    l_int32 bx = 0, by = 0, bw = 0, bh = 0;
                    boxGetGeometry(box, &bx, &by, &bw, &bh);

                    // 文字位置收集
                    std::vector<int> posLeftTop;
                    posLeftTop.push_back(bx);
                    posLeftTop.push_back(by);
                    std::vector<int> posRightTop;
                    posRightTop.push_back(bx + bw);
                    posRightTop.push_back(by);
                    std::vector<int> posRightBottom;
                    posRightBottom.push_back(bx + bw);
                    posRightBottom.push_back(by + bh);
                    std::vector<int> posLeftBottom;
                    posLeftBottom.push_back(bx);
                    posLeftBottom.push_back(by + bh);

                    std::vector<std::vector<int>> rcItem;
                    rcItem.push_back(posLeftTop);
                    rcItem.push_back(posRightTop);
                    rcItem.push_back(posRightBottom);
                    rcItem.push_back(posLeftBottom);

                    objOcrRet.textRects.push_back(rcItem);

                    // 文字识别
                    int rx = bx;
                    int ry = (by > 0) ? by - 1 : 0;
                    int rw = bw;
                    int rh = bh + 1;
                    api->SetRectangle(rx, ry, rw, rh);
                    char* ocrResult = api->GetUTF8Text();
                    objOcrRet.texts.push_back(std::string(ocrResult));
                    delete [] ocrResult;

                    boxDestroy(&box);
                }
                boxaDestroy(&boxes);
            }

            pixDestroy(&image);
        }

        api->End();
        delete api;
    }

    objOcrRet.hasGotRect = true;
    objOcrRet.imagePath = imagePath;
}
