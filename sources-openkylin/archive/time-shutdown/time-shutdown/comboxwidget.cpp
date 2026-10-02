/*
 * Ukui-shutdown-timer
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 *
 * Authors:  liushanwen <liushanwen@kylinos.cn>
 *
 */

#include "comboxwidget.h"
#include <QPainterPath>
#include <QDebug>

comBoxWidget::comBoxWidget(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(312, 34);
    this->setProperty("isWindowButton", 0x1);
    initMemberVariable();
    initLayout();

    //监控字体变化
    if (QGSettings::isSchemaInstalled(QString("org.ukui.style").toLocal8Bit())) {
        m_gsetting = new QGSettings(QString("org.ukui.style").toLocal8Bit(), QByteArray(), this);
        connect(m_gsetting, &QGSettings::changed, this, [this](const QString &key) {
            if (key != "systemFont" && key != "systemFontSize") {
                return;
            }
            updateFrequencyTitleLabel();
            m_pLabel_2->setText(SetFormatBody(m_sStroyLabel_2_Text, this->m_pLabel_2));
        });
    }
}

void comBoxWidget::initMemberVariable()
{
    m_pLabel_1 = new QLabel();
    m_pLabel_1->setAlignment(Qt::AlignVCenter);
    m_pLabel_1->setFixedWidth(149);
    m_frequencyTitleLabelText = QObject::tr("Shutdown");
    updateFrequencyTitleLabel();

    m_pLabel_2 = new QLabel();
    m_pLabel_2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_pLabel_2->setFixedWidth(99);
    m_pLabel_3 = new QLabel();
    QIcon labelIcon = QIcon::fromTheme("ukui-down.symbolic");
    m_pLabel_3->setPixmap(labelIcon.pixmap(QSize(16, 16)));
    m_pLabel_3->setProperty("useIconHighlightEffect", 0x1);
    m_pHcomBoxWidgetLayout = new QHBoxLayout();
    m_pHcomBoxWidgetLayout->setSpacing(0);
    m_pHcomBoxWidgetLayout->setContentsMargins(0, 0, 0, 0);
    this->setContentsMargins(0, 0, 0, 0);
}

void comBoxWidget::initLayout()
{
    m_pHcomBoxWidgetLayout->addItem(new QSpacerItem(16, 1));
    m_pHcomBoxWidgetLayout->addWidget(m_pLabel_1, Qt::AlignVCenter);
    m_pLabel_1->setAlignment(Qt::AlignVCenter);
    m_pHcomBoxWidgetLayout->addItem(new QSpacerItem(16, 1, QSizePolicy::Fixed));
    m_pHcomBoxWidgetLayout->addWidget(m_pLabel_2, Qt::AlignVCenter);
    m_pHcomBoxWidgetLayout->addItem(new QSpacerItem(8, 1, QSizePolicy::Fixed));
    m_pHcomBoxWidgetLayout->addWidget(m_pLabel_3, Qt::AlignVCenter);
    m_pHcomBoxWidgetLayout->addItem(new QSpacerItem(8, 1, QSizePolicy::Fixed));

    this->setLayout(m_pHcomBoxWidgetLayout);
}

//获取关机频率栏的文字显示
void comBoxWidget::setLabelWeekSelect(QString text)
{
    m_sStroyLabel_2_Text = text;
    this->m_pLabel_2->setText(SetFormatBody(m_sStroyLabel_2_Text, this->m_pLabel_2));
}

void comBoxWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void comBoxWidget::mouseReleaseEvent(QMouseEvent *event)
{
    emit comBoxWidgetClicked();
    QWidget::mouseReleaseEvent(event);
    return;
}

/* 设置...字样 */
QString comBoxWidget::SetFormatBody(QString text, QLabel *label)
{
    QFontMetrics fontMetrics(label->font());
    int LableWidth = label->width();
    int fontSize = fontMetrics.width(text);
    QString formatBody = text;
    if(fontSize > (LableWidth - 10)) {
        QStringList list = formatBody.split("\n");
        if (list.size() >= 2) {
            //当有几行时，只需要截取第一行就行，在第一行后面加...
            // 判断第一行是否是空行
            formatBody = judgeBlankLine(list);
            formatBody = formatBody + "aa";
            int oneFontSize = fontMetrics.width(formatBody);
            if (oneFontSize > (LableWidth - 10)) {
                formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, LableWidth - 10);
                return formatBody;
            } else {
                if (!substringSposition(formatBody, list)) {
                    int oneFontSize = fontMetrics.width(formatBody);
                    formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, oneFontSize - 1);
                    return formatBody;
                }
            }
        } else {
            //说明只存在一行，在最后面加...就行
            formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, LableWidth - 10);
            return formatBody;
        }
    } else {
        QStringList list = formatBody.split("\n");
        if (list.size() >= 2) {
            //取得当前的有字符串的子串
            formatBody = judgeBlankLine(list);
            formatBody = formatBody + "aa";
            if (!substringSposition(formatBody, list)) {
                int oneFontSize = fontMetrics.width(formatBody);
                formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, oneFontSize - 1);
            }
        }
    }
    return formatBody;
}

/* 去除掉空行，显示有字体的行 */
QString comBoxWidget::judgeBlankLine(QStringList list)
{
    int tmp = list.count();
    for (int i = 0; i < tmp; i++) {
        QString dest = list.at(i);
        dest = dest.trimmed();
        if (dest.size() != 0) {
           return list.at(i);
        }
    }
    return list.at(0);
}

/* 判断当前子串位置，后面是否还有子串 */
bool comBoxWidget::substringSposition(QString formatBody, QStringList list)
{
    int tmp = list.count();
    for (int i = 0; i < tmp; i++) {
        QString dest = list.at(i);
        if (dest == formatBody && i == tmp - 1) {
            return true;
        }
    }
    return false;
}

void comBoxWidget::leaveEvent(QEvent *e)
{
    Q_UNUSED(e);
    m_mouseEnter = false;
    repaint();
}

void comBoxWidget::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    m_mouseEnter = true;
    repaint();
}

void comBoxWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);

    QColor color;
    if (m_mouseEnter && this->isEnabled()) {
        color = opt.palette.color(QPalette::Midlight);
    } else {
        color = opt.palette.color(QPalette::Button);
    }
    color.setAlpha(m_transparency * 255 / 100);
    p.setBrush(color);

    p.setOpacity(1);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(opt.rect, 6, 6);
    p.setRenderHint(QPainter::Antialiasing); //反锯齿
}

void comBoxWidget::updateFrequencyTitleLabel()
{
    QFontMetrics metrics(m_pLabel_1->font());
    QString elidedText = metrics.elidedText(m_frequencyTitleLabelText, Qt::ElideRight, m_pLabel_1->width());
    m_pLabel_1->setText(elidedText);

    if (elidedText != m_frequencyTitleLabelText) {
        m_pLabel_1->setToolTip(m_frequencyTitleLabelText); // 缩略时显示完整文本
    } else {
        m_pLabel_1->setToolTip(""); // 未被缩略时不显示toolTip
    }
}
