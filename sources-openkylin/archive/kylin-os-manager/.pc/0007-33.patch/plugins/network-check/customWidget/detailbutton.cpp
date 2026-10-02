/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "detailbutton.h"
#include <QHBoxLayout>

DetailButton::DetailButton(QWidget *parent) : QPushButton(parent)
{
    mLabelText = new CustomLabel(this);
    QPalette pe1;
    //    pe1.setColor(QPalette::Text,QColor(0,0,255));
    //    mLabelText->setPalette(pe1);
    QString style = "QLabel{color: #3790FA;}";
    mLabelText->setStyleSheet(style);
    mLabelText->setText(tr("detail"));
    mLabelText->setFixedSize(60, 36);
    mLabelText->setAlignment(Qt::AlignCenter);
    QFont font;
    font.setPixelSize(12);
    mLabelText->setFont(font);

    mLabelIcon = new QLabel(this);
    mLabelIcon->setFixedSize(22, 22);
    QIcon icon(":/data/ukui-down-symbolic.svg");
    QPixmap pix = icon.pixmap(icon.actualSize(QSize(22, 22)));
    mLabelIcon->setPixmap(pix);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mLabelText);
    layout->addWidget(mLabelIcon);
    this->setLayout(layout);
}

void DetailButton::setButtonStyle(bool isDown)
{
    if (isDown) {
        QIcon icon(":/data/ukui-down-symbolic.svg");
        QPixmap pix = icon.pixmap(icon.actualSize(QSize(22, 22)));
        mLabelIcon->setPixmap(pix);

    } else {
        QIcon icon(":/data/ukui-up-symbolic.svg");
        QPixmap pix = icon.pixmap(icon.actualSize(QSize(22, 22)));
        mLabelIcon->setPixmap(pix);
    }
}
void DetailButton::setSystemFontSizeChange(int size)
{
    QFont font;
    int labelSize = 12 * size / 11;
    font.setPixelSize(labelSize);
    mLabelText->setFont(font);
}
