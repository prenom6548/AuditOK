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

#include "item_widget.h"
#include <QDebug>
#include "detailbutton.h"
#include <QHBoxLayout>

ItemWidget::ItemWidget(QWidget *parent) : QWidget(parent)
{
    setFixedWidth(716);
    checkPoint = new CustomLabel(this);
    checkPoint->setFixedSize(140, 36);
    checkPoint->setAlignment(Qt::AlignVCenter);

    checkInfo = new CustomLabel(this);
    checkInfo->setMaximumWidth(520);
    checkInfo->setAlignment(Qt::AlignVCenter);

    showContentBtn = new KBorderlessButton(this);
    showContentBtn->setText(tr("Detail"));
    connect(showContentBtn, &QPushButton::clicked, this, [=]() {
        if (m_isArrowDown)
            m_isArrowDown = false;
        else
            m_isArrowDown = true;

        emit showContent();
    });
    showContentBtn->hide();

    checkRes = new CustomLabel(this);
    checkRes->setFixedSize(55, 36);
    checkRes->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(checkPoint);
    layout->setAlignment(checkPoint, Qt::AlignVCenter);
    layout->addSpacing(10);
    layout->addWidget(checkInfo);
    layout->setAlignment(checkInfo, Qt::AlignVCenter);
    layout->addSpacing(2);
    layout->addWidget(showContentBtn);
    layout->setAlignment(showContentBtn, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(checkRes);
    layout->setAlignment(checkRes, Qt::AlignVCenter);
    this->setLayout(layout);

    initSettings();
}

ItemWidget::~ItemWidget() {}

void ItemWidget::setDetailMode(bool trigger)
{
    m_isArrowDown = true;
    if (trigger) {
        showContentBtn->show();
        checkInfo->setMaximumWidth(440);
    } else {
        showContentBtn->hide();
        checkInfo->setMaximumWidth(520);
    }
}

void ItemWidget::setStatusCheck(CheckStatus type)
{
    m_curType = type;
    switch (static_cast<int>(type)) {
    case static_cast<int>(CheckStatus::INIT):
        setInitStyle();
        checkRes->setText("");
        break;
    case static_cast<int>(CheckStatus::CHECKING):
        setCheckingStyle();
        checkRes->setText(tr("Checking"));
        break;
    case static_cast<int>(CheckStatus::EVERTHING_IS_OK):
        setSucceedStyle();
        checkRes->setText(tr("OK"));
        break;
    case static_cast<int>(CheckStatus::ERROR):
        setFailedStyle();
        checkRes->setText(tr("ERR"));
        break;
    case static_cast<int>(CheckStatus::WARNING):
        setWarningStyle();
        checkRes->setText(tr("WARNING"));
        break;
    case static_cast<int>(CheckStatus::EXTRANET_OK):
        setSucceedStyle();
        checkRes->setText(tr("OK"));
        break;
    case static_cast<int>(CheckStatus::EXTRANET_ERR):
        setFailedStyle();
        checkRes->setText(tr("ERR"));
        break;
    case static_cast<int>(CheckStatus::INTRANET_OK):
        setSucceedStyle();
        checkRes->setText(tr("OK"));
        break;
    case static_cast<int>(CheckStatus::INTRANET_ERR):
        setFailedStyle();
        checkRes->setText(tr("ERR"));
        break;
    case static_cast<int>(CheckStatus::NET_WARNING):
        setWarningStyle();
        checkRes->setText(tr("WARNING"));
        break;
    default:
        break;
    }
    this->update();
}
void ItemWidget::setCheckPoint(QString checkPointName)
{
    checkPoint->setText(checkPointName);
}
void ItemWidget::setCheckInfo(QString checkDetailInfo)
{
    checkInfo->setText(checkDetailInfo);
}
void ItemWidget::setCheckRes(const QString &checkDetailInfo, const QString &checkResult)
{
    checkInfo->setText(checkDetailInfo);
    checkRes->setText(checkResult);
}

void ItemWidget::setInitStyle()
{
    if (m_curStyle == "ukui-dark" || m_curStyle == "ukui-black") {
        QPalette pe1;
        pe1.setColor(QPalette::Text, QColor(255, 255, 255));
        checkInfo->setPalette(pe1);

        QPalette pe2;
        pe2.setColor(QPalette::Text, QColor(255, 255, 255));
        checkRes->setPalette(pe2);
    } else {
        QPalette pe1;
        pe1.setColor(QPalette::Text, QColor(38, 38, 38));
        checkInfo->setPalette(pe1);

        QPalette pe2;
        pe2.setColor(QPalette::Text, QColor(38, 38, 38));
        checkRes->setPalette(pe2);
    }

    this->update();
    this->show();
}

void ItemWidget::setCheckingStyle()
{
    if (m_curStyle == "ukui-dark" || m_curStyle == "ukui-black") {
        QPalette pe1;
        pe1.setColor(QPalette::Text, QColor(255, 255, 255));
        checkInfo->setPalette(pe1);

        QPalette pe2;
        pe2.setColor(QPalette::Text, QColor(255, 255, 255));
        checkRes->setPalette(pe2);
    } else {
        QPalette pe1;
        pe1.setColor(QPalette::Text, QColor(38, 38, 38));
        checkInfo->setPalette(pe1);

        QPalette pe2;
        pe2.setColor(QPalette::Text, QColor(38, 38, 38));
        checkRes->setPalette(pe2);
    }

    this->update();
    this->show();
}
void ItemWidget::setFailedStyle()
{
    QPalette pe1;
    pe1.setColor(QPalette::Text, QColor(243, 34, 45));
    checkInfo->setPalette(pe1);

    QPalette pe2;
    pe2.setColor(QPalette::Text, QColor(243, 34, 45));
    checkRes->setPalette(pe2);

    this->update();
    this->show();
}

void ItemWidget::setSucceedStyle()
{
    QPalette pe1;
    pe1.setColor(QPalette::Text, QColor(166, 166, 166));
    checkInfo->setPalette(pe1);

    QPalette pe2;
    pe2.setColor(QPalette::Text, QColor(82, 196, 41));
    checkRes->setPalette(pe2);

    this->update();
    this->show();
}

void ItemWidget::setWarningStyle()
{
    QPalette pe1;
    pe1.setColor(QPalette::Text, QColor(246, 140, 39));
    checkInfo->setPalette(pe1);

    QPalette pe2;
    pe2.setColor(QPalette::Text, QColor(246, 140, 39));
    checkRes->setPalette(pe2);

    this->update();
    this->show();
}

void ItemWidget::initSettings()
{
    const QByteArray idd(UKUI_STYLE);
    if (QGSettings::isSchemaInstalled(idd)) {
        m_themeSettings = new QGSettings(idd);
    }
    if (m_themeSettings) {
        connect(m_themeSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == UKUI_THEME) {
                m_curStyle = m_themeSettings->get(UKUI_THEME).toString();
                setThemeStyle();
            }
        });
        m_curStyle = m_themeSettings->get(UKUI_THEME).toString();
    }
    setThemeStyle();
}
void ItemWidget::setThemeStyle()
{
    setStatusCheck(m_curType);
}

void ItemWidget::statusChanged(const statusStruct &curStatus)
{
    checkPoint->setText(curStatus.m_projectName);
    checkInfo->setText(curStatus.m_projectDetail);
    checkRes->setText(curStatus.m_projectRes);
    setStatusCheck(curStatus.m_curStutus);
    setDetailMode(curStatus.m_detailMode);

    this->update();
}

void ItemWidget::setSystemFontSizeChange(int size)
{
    //    showContentBtn->setSystemFontSizeChange(size);
}
