/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "mycheckbox.h"

MyCheckBox::MyCheckBox(const QString &text, QWidget *parent) :
    QCheckBox(text, parent)
{}

MyCheckBox::~MyCheckBox()
{}

void MyCheckBox::setFontSize(int size)
{
    QFont font = this->font();
    font.setPixelSize(size);
    // 默认为大字体粗体显示
    if (size > 20)
        font.setBold(true);
    this->setFont(font);
}
