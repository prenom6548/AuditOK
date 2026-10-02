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
#include "cmessagebox.h"

#include <QDebug>

CMessageBox::CMessageBox(QWidget *parent) :
    QDialog(parent)
{
    init();
}

CMessageBox::~CMessageBox()
{
    delete m_btnLayout;
    m_btnLayout = nullptr;
    delete m_buttonBox;
    m_buttonBox = nullptr;
    delete m_labelLayout;
    m_labelLayout = nullptr;
    delete m_textLabel;
    m_textLabel = nullptr;
    delete m_iconLabel;
    m_iconLabel = nullptr;
    delete m_btnClose;
    m_btnClose = nullptr;
    delete m_vLayout;
    m_vLayout = nullptr;
    delete m_widget;
    m_widget = nullptr;
}

void CMessageBox::init()
{
    this->setFixedSize(430,160);

//    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog); // 隐藏标题栏
    this->setWindowModality(Qt::ApplicationModal);
    kdk::UkuiStyleHelper::self()->removeHeader(this);

    m_widget = new QWidget(this);
    m_widget->setAccessibleName("kylin-video_QWidget_m_widget");
    m_widget->resize(this->size());
    m_vLayout = new QVBoxLayout(this);
    m_vLayout->setObjectName("kylin-video_QVBoxLayout_m_vLayout");
    m_vLayout->setContentsMargins(20, 25, 20, 25);


    // 关闭按钮
    m_btnClose = new QPushButton(this);
    m_btnClose->setAccessibleName("kylin-video_QPushButton_m_btnClose");
    m_btnClose->setFixedSize(30, 30);
    m_btnClose->setProperty("isWindowButton", 0x2);
    m_btnClose->setProperty("useIconHighlightEffect", 0x8);
    m_btnClose->setFlat(true);
    m_btnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_btnClose->setToolTip(tr("Close"));
    m_btnClose->move(width() - m_btnClose->width() - 5, 5);
    connect(m_btnClose, &QPushButton::clicked, this, &CMessageBox::reject);

    // 图标和文本
    m_iconLabel = new QLabel(this);
    m_iconLabel->setAccessibleName("kylin-video_QLabel_m_iconLabel");
    m_iconLabel->setFixedSize(30,30);
    m_iconLabel->setAlignment(Qt::AlignVCenter);
    QPixmap iconPixmap(":/ico/icon-warning.png");
    m_iconLabel->setPixmap(iconPixmap.scaled(m_iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_textLabel = new QLabel(this);
    m_textLabel->setAccessibleName("kylin-video_QLabel_m_textLabel");
    m_textLabel->setFixedSize(350,30);
    m_textLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
   // m_textLabel->setStyleSheet("font-size:13px;");

    m_labelLayout = new QHBoxLayout;
    m_labelLayout->setObjectName("kylin-video_QHBoxLayout_m_labelLayout");
    m_labelLayout->addWidget(m_iconLabel);
    m_labelLayout->addWidget(m_textLabel);
    m_labelLayout->setAlignment(Qt::AlignBaseline);
    m_labelLayout->setSpacing(5);
    m_labelLayout->setStretchFactor(m_iconLabel, 0);
    m_labelLayout->setStretchFactor(m_textLabel, 1);

    // 确定和取消按钮
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_buttonBox->setAccessibleName("kylin-video_QDialogButtonBox_m_buttonBox");
    m_buttonBox->setAccessibleName("kylin-video_cmessagebox_QDialogButtonBox_m_buttonBox");
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &CMessageBox::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &CMessageBox::reject);
    m_buttonBox->button(QDialogButtonBox::Ok)->setProperty("isImportant", true); // 默认高亮显示
    m_buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon());
    m_buttonBox->button(QDialogButtonBox::Ok)->setIconSize(QSize());
    m_buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QIcon());
    m_buttonBox->button(QDialogButtonBox::Cancel)->setIconSize(QSize());

    m_btnLayout = new QHBoxLayout(this);
    m_btnLayout->setObjectName("kylin-video_QHBoxLayout_m_btnLayout");
    m_btnLayout->addWidget(m_buttonBox);
    m_vLayout->addLayout(m_labelLayout);
    m_vLayout->addLayout(m_btnLayout);
    m_widget->setLayout(m_vLayout);
}

void CMessageBox::setText(const QString &text)
{
    m_tip = text;
    resetText();
}
void CMessageBox::resetText()
{
    QFontMetrics fontWidth(font());//得到每个字符的宽度
    QString show_name = fontWidth.elidedText(m_tip, Qt::ElideRight, width() - 100);
    m_textLabel->setText(show_name);
    if (show_name.endsWith("…"))
        setToolTip(m_tip);
    else
        setToolTip("");
}
