/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "mylabel.h"
#include <QPalette>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include "../../common/mydefine.h"
#include "../globalbackupinfo.h"
#include "../gsettingswrapper.h"

/**
 * @brief 通用构造Label方法
 * @param parent
 */
MyLabel::MyLabel(QWidget* parent) :
    QLabel(parent),
    m_bAutoTheme(true)
{
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, [=](bool isDark) {
        // 只有黑白两色手动跟随主题，其它颜色不需要程序员手动设置（自动即可）
        if (!m_bAutoTheme) {
            if (isDark) {
                this->setFontColor(Qt::white);
            } else {
                this->setFontColor(Qt::black);
            }
        }
    });

    // 主题模块已经将QLabel改为了跟随主题设置，这里暂时去掉
//    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::fontChanged, this, [=]() {
//        // 字体家族、大小变化需重绘，并且字体大小变化也可能会造成显示不全问题
//        if (this->wordWrap()) {
//            this->setText(m_text);
//        } else {
//            QFontMetrics fontMetrics(this->font());
//            int fontSize = fontMetrics.width(m_text);
//            if (fontSize > this->width()) {
//                this->setText(fontMetrics.elidedText(m_text, Qt::ElideRight, this->width()));
//            } else {
//                this->setText(m_text);
//            }
//        }
//    });
}

/**
 * @brief 构造固定大小、颜色、内容居中等的label，一般用于label控件不会变化重绘的场景
 * @param text
 * @param parent
 * @param align
 */
MyLabel::MyLabel(const QString& text, QWidget* parent /*= nullptr*/, Qt::Alignment align /*= Qt::AlignCenter*/) :
    QLabel(parent),
    m_text(text),
    m_bAutoTheme(true)
{
    this->setAlignment(align);
    this->setDeplayText(text);
    this->setScaledContents(true);

    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, [=](bool isDark) {
        // 只有黑白两色手动跟随主题，其它颜色不需要程序员手动设置（自动即可）
        if (!this->m_bAutoTheme) {
            if (isDark) {
                this->setFontColor(Qt::white);
            } else {
                this->setFontColor(Qt::black);
            }
        }
    });

    // 主题模块已经将QLabel改为了跟随主题设置，这里暂时去掉
//    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::fontChanged, this, [=]() {
//        // 字体家族、大小变化需重绘，并且字体大小变化也可能会造成显示不全问题
//        if (this->wordWrap()) {
//            this->setText(m_text);
//        } else {
//            QFontMetrics fontMetrics(this->font());
//            int fontSize = fontMetrics.width(m_text);
//            if (fontSize > this->width()) {
//                this->setText(fontMetrics.elidedText(m_text, Qt::ElideRight, this->width()));
//            } else {
//                this->setText(m_text);
//            }
//        }
//    });
}

MyLabel::~MyLabel()
{}

void MyLabel::setMaxLenText()
{
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(this->m_text);
    if (this->m_maxWidth > 0 && fontSize >= this->m_maxWidth) {
        this->setFixedWidth(this->m_maxWidth);
        this->setWordWrap(true);
    } else {
        this->setFixedWidth(fontSize);
    }
    this->setText(this->m_text);
}

/**
 * @brief 设置显示文本，根据字体自动设置控件宽度，并限制最大宽度；超过长度换行展示；
 * @param text
 * @param length 最大宽度
 */
void MyLabel::setMaxLenText(const QString& text, int width)
{
    m_text = text;
    m_maxWidth = width;
    setMaxLenText();
}

/**
 * @brief 如果文本超长则显示省略号，不超长正常显示
 * @param text
 * @note  调用此方法前必须先设置label固定大小或最小及其大小策略
 */
void MyLabel::setElidedText(const QString& text, Qt::TextElideMode mode/* = Qt::ElideRight*/)
{
    m_text = text;
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(m_text);
    if (fontSize > this->width()) {
        this->setText(fontMetrics.elidedText(m_text, mode, this->width()));
    } else {
        this->setText(m_text);
    }
}

/**
 * @brief 设置字体颜色，建议颜色使用Qt::black这种模式的
 * @param color
 */
void MyLabel::setFontColor(QColor color)
{
    // 只有黑白两色手动跟随主题，其它颜色不需要程序员手动设置（自动即可）
    if (color == QColor(Qt::black)) {
        m_bAutoTheme = false;
        if (g_GSettingWrapper.isDarkTheme()) {
            color = QColor(Qt::white);
        }
    } else if (color == QColor(Qt::white)) {
        m_bAutoTheme = false;
        if (!g_GSettingWrapper.isDarkTheme()) {
            color = QColor(Qt::black);
        }
    } else {
        m_bAutoTheme = true;
    }
    m_fontColor = color;

    QPalette palette = this->palette();
    if (m_fontColor == QColor(COLOR_BLUE)) {
        palette.setColor(QPalette::WindowText, palette.highlight().color());

        connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeColorChanged, this, [=]() {
            // 强调色更换，高亮等颜色已经改变，所以要重新加载图标。
            QPalette pal(this->palette());
            pal.setColor(QPalette::WindowText, this->palette().highlight().color());
            this->setPalette(pal);
        });
    } else {
        palette.setColor(QPalette::WindowText, color);
    }
    this->setPalette(palette);
}

void MyLabel::setFontSize(int size)
{
    QFont font = this->font();
    font.setPixelSize(size);
    // 默认为大字体粗体显示
    if (size > 20)
        font.setBold(true);
    this->setFont(font);
}

/**
 * @brief 设置字体是否自动换行展示
 * @param on
 * @note 一般用于setGeometry固定label位置和大小的场景
 */
void MyLabel::setFontWordWrap(bool on)
{
    m_bWordWrap = on;
    setWordWrap(on);
    m_width = this->width();
    m_height = this->height();
    m_rect = geometry();
}

//void MyLabel::paintEvent(QPaintEvent *event)
//{
//    // 1、场景一：布局动态变化场景，使用原始的QLabel绘制
//    if (m_isOriginal) {
//        this->setText(m_text);
//        QLabel::paintEvent(event);

//        return ;
//    }

//    // 2、场景二：setGeometry固定label位置和大小的场景
//    QFontMetrics  fontMetrics(this->font());
//    int fontSize = fontMetrics.width(m_text);
//    if (m_bWordWrap && m_width > 0) {
//        // resize(m_width, m_height);
//        // setGeometry(m_rect);
//        // 恢复控件宽度，如果不固定宽度则换行位置不好控制
//        this->setFixedWidth(m_width);
//    }

//    if (fontSize > this->width()) {
//        if (m_bWordWrap) {
//            // 调整控件大小
//            adjustSize();
//            setAlignment(Qt::AlignTop | Qt::AlignLeft);
//            this->setText(m_text);
//        } else {
//            this->setText(fontMetrics.elidedText(m_text, Qt::ElideRight, this->width()));
//        }
//    } else {
//        if (m_bWordWrap) {
//            // 恢复控件大小及位置
//            setGeometry(m_rect);
//            setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
//        }
//        this->setText(m_text);
//    }

//    QLabel::paintEvent(event);
//}
