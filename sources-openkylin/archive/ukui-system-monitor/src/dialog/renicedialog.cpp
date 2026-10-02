/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "renicedialog.h"
#include "../util.h"
#include "../macro.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsDropShadowEffect>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QPainterPath>

ReniceDialog::ReniceDialog(const QString &procName, const QString &procId, QWidget *parent)
    : KDialog(parent)
{
    this->setFixedSize(480, 260);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowIcon(QIcon::fromTheme("ukui-system-monitor"));
    m_strProcName = procName;
    m_strProcId = procId;
    this->setWindowTitle(tr("Change Priority of Process %1 (PID: %2)").arg(m_strProcName).arg(m_strProcId));

    m_titleLabel = new KLabel(this);
    m_titleLabel->setFixedWidth(90);
    m_titleLabel->setText(tr("Nice value:"));
    m_valueLabel = new KLabel(this);
    m_slider = new KSlider(Qt::Horizontal, this);
    m_slider->setSliderType(KSliderType::SmoothSlider);
    m_slider->setNodeVisible(false);
    m_slider->setRange(-20, 19);
    m_slider->setSingleStep(1);

    h_layout = new QHBoxLayout();
    h_layout->setSpacing(10);
    h_layout->setContentsMargins(0, 0, 0, 0);
    h_layout->addWidget(m_titleLabel);
    h_layout->addWidget(m_slider);
    h_layout->addWidget(m_valueLabel);

    m_valueStrLabel = new KLabel(this);
    m_valueStrLabel->setAlignment(Qt::AlignCenter);
    m_valueStrLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_tipLabel = new KLabel(this);
    m_tipLabel->setText(tr("Note:The priority of a process is given by its nice value. A lower nice value corresponds to a higher priority."));

    m_cancelbtn = new KPushButton(this);
    m_cancelbtn->setFixedWidth(184);
    m_cancelbtn->setObjectName("cancelButton");
    m_cancelbtn->setFocusPolicy(Qt::NoFocus);
    m_cancelbtn->setText(tr("Cancel"));
    m_changeBtn = new KPushButton(this);
    m_changeBtn->setFixedWidth(184);
    m_changeBtn->setObjectName("confirmButton");
    m_changeBtn->setFocusPolicy(Qt::NoFocus);
    m_changeBtn->setText(tr("Change Priority"));

    btn_layout = new QHBoxLayout();
    btn_layout->setSpacing(10);
    btn_layout->setContentsMargins(0, 0, 0, 0);
    btn_layout->addWidget(m_cancelbtn);
    btn_layout->addStretch();
    btn_layout->addWidget(m_changeBtn);

    QVBoxLayout *v_layout = new QVBoxLayout();
    v_layout->setMargin(0);
    v_layout->setSpacing(15);
    v_layout->setContentsMargins(24, 10, 24, 24);
    v_layout->addLayout(h_layout, 0);
    v_layout->addWidget(m_valueStrLabel, 0, Qt::AlignHCenter);
    v_layout->addWidget(m_tipLabel, 0);
    v_layout->addLayout(btn_layout, 0);

    mainWidget()->setLayout(v_layout);

    connect(m_slider, &QSlider::valueChanged, [=] (int value) {
        m_valueLabel->setText(QString::number(value));
        QString levelStr = getNiceLevelWithPriority(value);
        m_valueStrLabel->setText(levelStr);
    });

    connect(m_cancelbtn, &KPushButton::clicked, this, [=](){
        this->close();
    });
    connect(m_changeBtn, &QPushButton::clicked, [=] (bool b) {
        Q_UNUSED(b);
        emit this->resetReniceValue(m_slider->value());
        this->close();
    });
}

ReniceDialog::~ReniceDialog()
{
}

void ReniceDialog::loadData(int nice)
{
    m_slider->setValue(nice);
    m_valueLabel->setText(QString::number(nice));

    QString levelStr = getNiceLevelWithPriority(nice);
    m_valueStrLabel->setText(levelStr);
}


