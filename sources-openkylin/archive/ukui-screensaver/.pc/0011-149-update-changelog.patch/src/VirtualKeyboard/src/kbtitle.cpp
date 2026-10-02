/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "kbtitle.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "commondef.h"
#include <QDebug>

KBTitle::KBTitle(QWidget *parent /* = nullptr*/) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground); //背景透明
    initUI();
    initConnections();
}

KBTitle::~KBTitle() {}

void KBTitle::initUI()
{
    setFixedHeight(KEYBOARD_TITLE_DEFAULT_HEIGHT);
    QString strBtnStyle = "QPushButton{ text-align:center; color: rgba(255, 255, 255, 255); border: none; "
                          "border-radius: 4px; outline: none;}"
                          "QPushButton:hover{ background-color: rgba(255,255,255,15%); }"
                          "QPushButton::pressed { background-color: rgba(255,255,255,40%); }"
                          "QPushButton::checked { background-color: rgba(255,255,255,40%); }";

    m_btnFloat = new QPushButton(this);
    m_btnFloat->setFlat(true);
    m_btnFloat->setFocusPolicy(Qt::NoFocus);
    m_btnFloat->setIcon(QIcon(":/images/images/float.svg"));
    m_btnFloat->setToolTip(tr("Suspended state"));
    m_btnFloat->setObjectName("btn_float");
    m_btnFloat->setIconSize(KEYBOARD_FIXED_DEFAULT_ICONSIZE);
    m_btnFloat->setGeometry(1484, 8, KEYBOARD_TITLEBTN_DEFAULT_WIDTH, KEYBOARD_TITLEBTN_DEFAULT_HEIGHT);
    m_btnFloat->setStyleSheet(strBtnStyle);
    m_mapSubGeometrys[m_btnFloat] = m_btnFloat->geometry();

    m_btnClose = new QPushButton(this);
    m_btnClose->setFlat(true);
    m_btnClose->setFocusPolicy(Qt::NoFocus);
    m_btnClose->setIcon(QIcon(":/images/images/close.svg"));
    m_btnClose->setToolTip(tr("Close"));
    m_btnClose->setIconSize(KEYBOARD_FIXED_DEFAULT_ICONSIZE);
    m_btnClose->setObjectName("btn_close");
    m_btnClose->setGeometry(1548, 8, KEYBOARD_TITLEBTN_DEFAULT_WIDTH, KEYBOARD_TITLEBTN_DEFAULT_HEIGHT);
    m_btnClose->setStyleSheet(strBtnStyle);
    m_mapSubGeometrys[m_btnClose] = m_btnClose->geometry();
}

void KBTitle::initConnections()
{
    connect(m_btnFloat, &QPushButton::clicked, this, &KBTitle::onBtnClicked);
    connect(m_btnClose, &QPushButton::clicked, this, &KBTitle::onBtnClicked);
}

void KBTitle::onBtnClicked()
{
    QObject *obj = sender();
    QPushButton *btn = static_cast<QPushButton *>(obj);
    QString objName = btn->objectName();
    int lastUnderline = objName.lastIndexOf('_');
    int start = strlen("btn_");
    int keyLength = lastUnderline - start;
    QString keyName = objName.mid(start, keyLength);
    if (keyName == BTN_FLOAT) {
        if (floatStatus) {
            btn->setIcon(QIcon(":/images/images/float.svg"));
            btn->setToolTip(tr("Suspended state"));
        } else {
            btn->setIcon(QIcon(":/images/images/float-restore.svg"));
            btn->setToolTip(tr("Welt status"));
        }
        floatStatus = !floatStatus;
    }

    Q_EMIT btnClicked(keyName);
}

void KBTitle::adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical /* = false*/, bool floatStatus)
{
    setFixedHeight(KEYBOARD_TITLE_DEFAULT_HEIGHT * lfHeightScale);
    QMap<QPushButton *, QRect>::iterator itGeometry = m_mapSubGeometrys.begin();
    for (; itGeometry != m_mapSubGeometrys.end(); itGeometry++) {
        QPushButton *button = itGeometry.key();
        if (button) {
            QRect oldGeometry = itGeometry.value();
            QRect newGeometry = oldGeometry;
            if (floatStatus) {
                newGeometry.setX(oldGeometry.x() * lfWidthScale * KEYBOARD_FLOAT_PERCENTAGE);
                newGeometry.setY(oldGeometry.y() * lfHeightScale);
                newGeometry.setWidth(oldGeometry.width() * lfWidthScale * KEYBOARD_FLOAT_PERCENTAGE);
                newGeometry.setHeight(oldGeometry.height() * lfHeightScale);
            } else {
                newGeometry.setX(oldGeometry.x() * lfWidthScale);
                newGeometry.setY(oldGeometry.y() * lfHeightScale);
                newGeometry.setWidth(oldGeometry.width() * lfWidthScale);
                newGeometry.setHeight(oldGeometry.height() * lfHeightScale);
            }
            button->setGeometry(newGeometry);
        }
    }
}
