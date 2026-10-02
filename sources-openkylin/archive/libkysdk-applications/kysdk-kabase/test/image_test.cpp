/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QDebug>
#include <QDir>

#include "image_test.hpp"
#include "kylinimagecodec.hpp"

ImageTest::ImageTest()
{
    initWidget();
    getAllImagePath(m_imagePath->text());

    connect(m_nextImageLabel, &QPushButton::clicked, this, &ImageTest::showImage);
}

ImageTest::~ImageTest()
{
}

QPixmap ImageTest::loadImage(QString path)
{
    kdk::kabase::MatResult temp = kdk::kabase::KylinImageCodec::loadImageToMat(path);

    return kdk::kabase::KylinImageCodec::converFormat(temp.mat);
}

void ImageTest::getAllImagePath(QString dirPath)
{
    QDir dir(dirPath);

    QList<QString> fileList = dir.entryList();
    for (int i = 0; i < fileList.count(); i++) {
        m_imageList << dirPath + fileList.at(i);
    }

    return;
}

void ImageTest::initWidget(void)
{
    m_showImageLabel = new QLabel(this);
    m_imagePath = new QLineEdit(this);
    m_imagePath->setText(QString("/home/snow/temp/image/"));
    m_nextImageLabel = new QPushButton(this);

    m_vLayout = new QVBoxLayout;
    m_vLayout->addWidget(m_showImageLabel);
    m_vLayout->addWidget(m_imagePath);
    m_vLayout->addWidget(m_nextImageLabel);

    setLayout(m_vLayout);

    return;
}

void ImageTest::showImage(void)
{
    static int i = 0;

    if (i == m_imageList.count()) {
        i = 0;
    }

    qDebug() << "++" << m_imageList.at(i);
    m_showImageLabel->setPixmap(loadImage(m_imageList.at(i++)));

    return;
}
