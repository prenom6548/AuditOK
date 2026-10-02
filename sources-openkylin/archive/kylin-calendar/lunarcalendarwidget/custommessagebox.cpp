/*
* Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVariant>
#include <QDebug>
#include <QDialog>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGSettings>
#include <QToolButton>
#include "custommessagebox.h"

#define SHADOW_WIDTH 15    // 窗口阴影宽度;
#define TRIANGLE_WIDTH 15  // 小三角的宽度;
#define TRIANGLE_HEIGHT 10 // 小三角的高度;
#define BORDER_RADIUS 5    // 窗口边角的弧度;
#define THEME_SCHAME "org.ukui.style"
#define COLOR_THEME "styleName"
CustomMessageBox::CustomMessageBox(int type, QWidget *parent) : QDialog(parent)
{
    setAccessibleName("kylin-calendar-plugin_QDialog_custom_messageBox");
    setAccessibleDescription("this is a messagebox in operate the schedule");
    m_type = type;

    QString m_buttonstylelight = "QPushButton {"
                                 "    background-color: #E6E6E6;"
                                 "    color: black;"
                                 "}";
//                                 "QPushButton:hover {"
//                                 "    color: white; /* 修改鼠标悬停时的文字颜色为白色 */"
//                                 "}";
    QString m_buttonstyledark = "QPushButton {"
                                "   background-color: #373737;"
                                "    color: white;"
                                "}";
    QGSettings *styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id))
    {
        styleGsettings = new QGSettings(style_id, QByteArray(), this);
        QVBoxLayout *layout = new QVBoxLayout(this);

        this->setWindowTitle(tr("Schedule"));
        this->setWindowIcon(QIcon::fromTheme("kylin-calendar"));
        QLabel *warning = new QLabel(this);
        QIcon icon = QIcon::fromTheme("dialog-warning");
        QPixmap pixmap = icon.pixmap(QSize(25, 25));
        warning->setPixmap(pixmap);
        QHBoxLayout *summury = new QHBoxLayout(this);

        if(type == 1)
        {
            lbl_textLabel = new QLabel(tr("Are you sure you want to delete this event?"), this);
            lbl_textLabel->setWordWrap(true); // 启用自动换行
            lbl_textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); // 设置尺寸策略
        }
        else if(type == 0)
        {
            lbl_textLabel = new QLabel(tr("The end time cannot be earlier than the start time, please re-enter it!"), this);
            lbl_textLabel->setWordWrap(true); // 启用自动换行
            lbl_textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); // 设置尺寸策略
        }
        else if(type == 2)
        {
            lbl_textLabel = new QLabel( tr("The time entered is not in the correct format, please make sure that the hours are between 0-23 and the minutes are between 0-59."), this);
            lbl_textLabel->setWordWrap(true); // 启用自动换行
            lbl_textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); // 设置尺寸策略
        }
        else if(type == 3)
        {
            lbl_textLabel = new QLabel(tr("text is empty."), this);
            lbl_textLabel->setWordWrap(true); // 启用自动换行
            lbl_textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); // 设置尺寸策略
        }
        warning->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        lbl_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        lbl_textLabel->setAccessibleName("kylin-calendar-plugin_QLabel_custom_messadgebox_label");
        lbl_textLabel->setAccessibleDescription("Description of messadgebox");
        warning->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        summury->addSpacing(20);
        summury->addWidget(warning);
        summury->addSpacing(2);
        summury->addWidget(lbl_textLabel);

        layout->addLayout(summury);
        QHBoxLayout *buttonLayout = new QHBoxLayout(this);
        QPushButton *yesButton = new QPushButton(tr("Yes"), this);
        QPushButton *noButton = new QPushButton(this);
        yesButton->setFixedSize(100, 30);
        buttonLayout->addStretch(5);
        buttonLayout->addWidget(yesButton, 0, Qt::AlignRight | Qt::AlignBottom);
        if(type == 1)
        {
            noButton->setVisible(false);
            noButton = new QPushButton(tr("No"), this);
            noButton->setFixedSize(100, 30);
            buttonLayout->addWidget(noButton, 0, Qt::AlignRight | Qt::AlignBottom);
            connect(noButton, &QPushButton::clicked, this, &CustomMessageBox::noClicked);
        }
        else
        {
            noButton->setVisible(false);
        }

        buttonLayout->setContentsMargins(0, 0, 20, 20);
        QWidget *buttonWidget = new QWidget();
        buttonWidget->setLayout(buttonLayout);
        layout->addWidget(buttonWidget);
        connect(yesButton, &QPushButton::clicked, this, &CustomMessageBox::yesClicked);

        const QByteArray style_id(THEME_SCHAME);
        if (QGSettings::isSchemaInstalled(style_id))
        {
            QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
            if(currentTheme == "ukui-default" || currentTheme == "ukui-light" )
            {
                this->setStyleSheet("background-color:#FFFFFF;color:black;");
                yesButton->setStyleSheet(m_buttonstylelight);
                noButton->setStyleSheet(m_buttonstylelight);
                setLayout(layout);
                setFixedSize(480, 170);
                if(type == 2)
                {
                    setFixedSize(480, 250);
                }
            }
            else
            {
                this->setStyleSheet("background-color: rgb(29,29,29);color:white;");
                yesButton->setStyleSheet(m_buttonstyledark);
                noButton->setStyleSheet(m_buttonstyledark);
                setLayout(layout);
                setFixedSize(480, 170);
                if(type == 2)
                {
                    setFixedSize(480, 250);
                }
            }
        }

        connect(styleGsettings, &QGSettings::changed, [ = ](QString key)
        {
            if ("styleName" == key)
            {
                const QByteArray style_id(THEME_SCHAME);
                if (QGSettings::isSchemaInstalled(style_id))
                {
                    QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
                    if(currentTheme == "ukui-default" || currentTheme == "ukui-light" )
                    {
                        this->setStyleSheet("background-color:#FFFFFF;color:black;");
                        yesButton->setStyleSheet(m_buttonstylelight);
                        noButton->setStyleSheet(m_buttonstylelight);
                        setLayout(layout);
                        setFixedSize(480, 170);
                        if(type == 2)
                        {
                            setFixedSize(480, 250);
                        }
                    }
                    else
                    {
                        this->setStyleSheet("background-color: rgb(29,29,29);color:white;");
                        yesButton->setStyleSheet(m_buttonstyledark);
                        noButton->setStyleSheet(m_buttonstyledark);
                        setLayout(layout);
                        setFixedSize(480, 170);
                        if(type == 2)
                        {
                            setFixedSize(480, 250);
                        }
                    }
                }
            }
        });
    }
}
void CustomMessageBox::paintEvent(QPaintEvent *)
{
}
QString CustomMessageBox::getText()
{
    return m_text;
}
void CustomMessageBox::SetText(QString text)
{
    m_text = text;
}
