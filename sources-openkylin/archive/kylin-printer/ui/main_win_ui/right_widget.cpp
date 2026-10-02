/*
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
 */
#include "right_widget.h"
#include "size_para.h"
#include "gsettings_monitor.h"
#include "common_ui.h"

RightWidget::RightWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainWin(parent)
{
    if(parent == nullptr)
    {
        return;
    }
    initExhbitionSectorQSS(parent);

    this->setLayout(m_VLayoutExhbition);
}

void RightWidget::initPropertySectorQSS()
{
    m_deviceExhbitionMyWidget = new MainWinPropertyWidget(m_mainWin,this);
    m_deviceExhbitionMyWidget->setObjectName("deviceExhbitionWidget");
    m_deviceExhbitionMyWidget->setFixedSize(DEVICE_PROPERTY_SECTION);

    m_emptyWidget = new EmptyWidget(this);
    m_emptyWidget->setObjectName("emptyWidget");
    m_emptyWidget->setFixedSize(DEVICE_PROPERTY_SECTION);

    m_HLayoutProperty->setMargin(0);
    m_HLayoutProperty->setSpacing(0);
    m_HLayoutProperty->addWidget(m_deviceExhbitionMyWidget);
    m_HLayoutProperty->addWidget(m_emptyWidget);
    m_HLayoutProperty->addStretch();
}

void RightWidget::initExhbitionSectorQSS(QWidget *parent)
{
    if(parent == nullptr)
    {
        return;
    }
    //总体垂直布局
    m_VLayoutExhbition = new QVBoxLayout(this);

    //属性展示区布局
    m_HLayoutProperty = new QHBoxLayout();

    initPropertySectorQSS();

    m_VLayoutExhbition->setSpacing(0);
    m_VLayoutExhbition->setMargin(0);
    m_VLayoutExhbition->addSpacing(4);
    m_VLayoutExhbition->addLayout(m_HLayoutProperty);
    m_VLayoutExhbition->addStretch();


}

void RightWidget::updateExhbitionSector(const UkuiUsbPrinter& printer)
{
    m_deviceExhbitionMyWidget->updateText(printer);
    m_deviceExhbitionMyWidget->show();
    m_emptyWidget->hide();
}

void RightWidget::hideExhbitionSector()
{
    m_emptyWidget->show();
    m_deviceExhbitionMyWidget->hide();
//    this->update();
}

void RightWidget::setQSSFontSize(QFont curFont)
{

}

void RightWidget::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}

EmptyWidget::EmptyWidget(QWidget *parent)
{
    QFont ft;
    ft.setPixelSize(14);
//    showWindowFrame(this);
    m_first = new QLabel(this);
    // m_first->setText(tr("No Valid Printer,"));
    m_first->setAlignment(Qt::AlignHCenter);
    m_second = new QLabel(this);
    m_second->setText(tr("Please click \"+\" button to add a printer."));
    m_second->setAlignment(Qt::AlignHCenter);
    m_icon = new QPushButton(this);
    m_icon->setProperty("useIconHighlightEffect", 0x0);
    m_icon->setIconSize(QSize(128,128));
    m_icon->setFixedSize(128,128);
    m_icon->setFlat(true);
    m_icon->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QString style = GsettingsMonitor::getInstance()->sysStyleNameGet();
    if(style == "ukui-dark" || style == "ukui-black") {
        m_icon->setIcon(QIcon(":/pic/无打印机_深色@1x.png"));
    } else {
        m_icon->setIcon(QIcon(":/pic/无打印机_浅色@1x.png"));
    }

    connect(GsettingsMonitor::getInstance(),&GsettingsMonitor::sysStyleNameUpdate, [this] (const QString &style) {
        if(style == "ukui-dark" || style == "ukui-black") {
            m_icon->setIcon(QIcon(":/pic/无打印机_深色@1x.png"));
        } else {
            m_icon->setIcon(QIcon(":/pic/无打印机_浅色@1x.png"));
        }
    });
    m_VLayoutExhbition = new QVBoxLayout(this);
    m_VLayoutExhbition->setSpacing(0);
    m_VLayoutExhbition->setMargin(0);
    m_VLayoutExhbition->addSpacing(224 - 128);
    m_VLayoutExhbition->addWidget(m_first);
    m_VLayoutExhbition->addSpacing(10);
    m_VLayoutExhbition->addWidget(m_icon, 0, Qt::AlignHCenter);
    m_VLayoutExhbition->addSpacing(8);
    m_VLayoutExhbition->addWidget(m_second);
    m_VLayoutExhbition->addStretch();

    this->setLayout(m_VLayoutExhbition);
    this->setBackgroundRole(QPalette::Button);
}
void EmptyWidget::setQSSFontSize(QFont curFont)
{

}

void EmptyWidget::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}
