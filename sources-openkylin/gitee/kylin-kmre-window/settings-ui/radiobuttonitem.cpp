/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "radiobuttonitem.h"
#include "global.h"
#include "messagebox.h"
#include "utils.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMessageBox>
#include <QDBusInterface>
#include <QThread>
#include <QProcess>
#include <unistd.h>
using namespace Global;
RadioButtonItem::RadioButtonItem(QButtonGroup *btnGroup, int index, QFrame *parent)
    : SettingsItem(parent)
    , m_label(new QLabel)
    , m_radioBtn(new QRadioButton)
    , m_confirmBtn(new QPushButton)
    , m_index(index)
{
    btnGroup->addButton(m_radioBtn, m_index);
    btnGroup->setId(m_radioBtn, m_index);
//    m_radioBtn->setStyleSheet("QRadioButton{spacing: 16px;font-size: 16px;color: rgb(0, 0, 0, 217);outline: none;background-color:transparent;}"
//                        "QRadioButton::indicator{width: 16px;height: 16px;}"
//                        "QRadioButton::indicator::unchecked{image: url(:/res/select.png);}"
//                        "QRadioButton::indicator::checked{image: url(:/res/selected.png);}"
//                        "QRadioButton::indicator::unchecked:disabled{image: url(:/res/select_disable.png);}"
//                        "QRadioButton::indicator::checked:disabled{image: url(:/res/selected_disable.png);}");
//    m_confirmBtn->setStyleSheet("QPushButton{border-radius: 4px;font: 14px;color: rgb(0, 0, 0, 217);background-color: #E9E9E9;padding-left: 10px;padding-right: 10px;}"
//                        "QPushButton:hover{border-radius: 4px;font: 14px;color: rgb(255, 255, 255, 217);background-color: #3D6BE5;}"
//                        "QPushButton:pressed{border-radius: 4px;font: 14px;color: rgb(255, 255, 255, 217);background-color: #3257CA;}"
//                        "QPushButton:disabled{border-radius: 4px;font: 14px;color: rgb(0, 0, 0, 77);background-color: #E9E9E9;}");
    m_confirmBtn->setStyleSheet("QPushButton{background-color: rgb(61,107,229);color:rgb(255,255,255);border-radius:3px;}"
                        "QPushButton:hover{background-color: rgb(100,149,237);}"
                        "QPushButton:checked{background-color: rgb(65,105,225);}");//font-size:12px;

//    m_radioBtn->setFixedSize(130, 36);
    m_radioBtn->setFixedHeight(36);
    m_radioBtn->setAutoExclusive(true);
    m_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_label->setMinimumWidth(350);
    m_label->setWordWrap(true);
    m_label->adjustSize();
    //m_label->setStyleSheet("QLabel{color: rgb(0, 0, 0, 166);background-color: transparent;}");//font-size: 14px;
    m_confirmBtn->setFixedSize(50, 28);
    m_confirmBtn->setText(tr("Confirm"));
    m_confirmBtn->setVisible(false);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_radioBtn, 0, Qt::AlignVCenter | Qt::AlignLeft);
    layout->addWidget(m_label, 0, Qt::AlignVCenter | Qt::AlignLeft);
    layout->addStretch();
    layout->addWidget(m_confirmBtn, 0, Qt::AlignVCenter | Qt::AlignRight);

//    QWidget *leftw = new QWidget;
//    QHBoxLayout *m_lLayout = new QHBoxLayout(leftw);
//    m_lLayout->setContentsMargins(2, 0, 0, 0);
//    m_lLayout->setSpacing(2);
//    m_lLayout->setSizeConstraint(QLayout::SetMinimumSize);
//    layout->addWidget(leftw, 1, Qt::AlignLeft);
//    m_lLayout->addWidget(m_radioBtn, 0, Qt::AlignVCenter | Qt::AlignLeft);
//    m_lLayout->addWidget(m_label, 0, Qt::AlignVCenter | Qt::AlignLeft);

//    QWidget *rightw = new QWidget;
//    QHBoxLayout *m_rLayout = new QHBoxLayout(rightw);
//    m_rLayout->setContentsMargins(0, 0, 2, 0);
//    m_rLayout->setSpacing(2);
//    layout->addWidget(rightw, 0, Qt::AlignRight);
//    m_rLayout->addWidget(m_confirmBtn, 0, Qt::AlignVCenter | Qt::AlignRight);

    this->setLayout(layout);
//    this->setFixedHeight(64);

    m_radioBtn->setFocusPolicy(Qt::NoFocus);
    m_confirmBtn->setFocusPolicy(Qt::NoFocus);

    connect(m_confirmBtn, &QPushButton::clicked, this, [=] () {
        emit this->radioBtnClicked(m_index);
        m_confirmBtn->setVisible(false);
//        KylinUI::MessageBox::information(this, tr("Tips"), tr("The modification takes effect after you restart the system"));
        if (KylinUI::MessageBox::restartEnv(this)) {
            QDBusInterface interface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus());
            interface.call("StopContainer", kmre::utils::getUserName(), (int32_t)getuid());
            emit restartEnv();
        }
    });

//    connect(m_radioBtn, &QRadioButton::clicked, this, [=](bool checked) {
//    });
}

void RadioButtonItem::setTitle(const QString &name, const QString &desc)
{
    m_name = name;
    m_radioBtn->setText(name);
    m_label->setText(desc);
}

void RadioButtonItem::setChecked()
{
    blockSignals(true);
    m_radioBtn->setChecked(true);
    blockSignals(false);
}

void RadioButtonItem::setConfirmButtonVisible(bool b)
{
    if (b) {
        m_confirmBtn->setVisible(true);
    }
    else {
        m_confirmBtn->setVisible(false);
    }
}

QString RadioButtonItem::getName() const
{
    return m_name;
}
