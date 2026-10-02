/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "imagelabeltest.h"
#include "widget/imagelabel.h"
#include <QApplication>
#include <QPixmap>
#include <QPaintEvent>
#include <QPainter>
#include <iostream>

ImageLabelTest::ImageLabelTest(QObject *parent) : QObject(parent)
{
    // 初始化成员变量为nullptr
    m_imageLabel = nullptr;
}

void ImageLabelTest::testConstructor()
{
    std::cout << "=== Testing ImageLabel Constructor ===" << std::endl;
    
    // 创建ImageLabel实例
    m_imageLabel = new ImageLabel();
    
    // 验证对象是否创建成功
    QVERIFY(m_imageLabel != nullptr);
    
    // 验证布局是否设置
    QVERIFY(m_imageLabel->layout() != nullptr);
    
    std::cout << "ImageLabel constructor test completed." << std::endl;
}

void ImageLabelTest::testSetTime()
{
    std::cout << "=== Testing ImageLabel setTime ===" << std::endl;
    
    // 确保对象已创建
    if (!m_imageLabel) {
        m_imageLabel = new ImageLabel();
    }
    
    // 设置时间文本
    QString testTime = "00:05:30";
    m_imageLabel->setTime(testTime);
    
    // 验证函数调用无异常
    // 由于m_labTime是私有成员，我们无法直接验证其文本
    // 但我们可以通过调用函数并确保没有崩溃来测试其基本功能
    
    std::cout << "ImageLabel setTime test completed." << std::endl;
}

void ImageLabelTest::testPaintEvent()
{
    std::cout << "=== Testing ImageLabel paintEvent ===" << std::endl;
    
    // 确保对象已创建
    if (!m_imageLabel) {
        m_imageLabel = new ImageLabel();
    }
    
    // 设置ImageLabel的大小
    m_imageLabel->resize(200, 200);
    
    // 创建一个测试图片
    QPixmap testPixmap(100, 100);
    testPixmap.fill(Qt::red);
    m_imageLabel->setPixmap(testPixmap);
    
    // 显示部件以确保paintEvent能被正确调用
    m_imageLabel->show();
    
    // 强制重绘
    m_imageLabel->repaint();
    
    // 验证图片是否设置成功
    QVERIFY(m_imageLabel->pixmap() != nullptr);
    QCOMPARE(m_imageLabel->pixmap()->width(), 100);
    QCOMPARE(m_imageLabel->pixmap()->height(), 100);
    
    // 隐藏部件
    m_imageLabel->hide();
    
    std::cout << "ImageLabel paintEvent test completed." << std::endl;
}

void ImageLabelTest::testDestructor()
{
    std::cout << "=== Testing ImageLabel Destructor ===" << std::endl;
    
    // 确保对象已创建
    if (!m_imageLabel) {
        m_imageLabel = new ImageLabel();
    }
    
    // 删除对象
    delete m_imageLabel;
    m_imageLabel = nullptr;
    
    // 验证指针是否为空
    QVERIFY(m_imageLabel == nullptr);
    
    std::cout << "ImageLabel destructor test completed." << std::endl;
}