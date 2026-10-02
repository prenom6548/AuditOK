/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Sisheng He <hesisheng@kylinos.cn>
 *
 */
#include "klineeditframe.h"

#include <QDebug>
namespace kdk
{

class KLineEditFramePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KLineEditFrame)
public:
    KLineEditFramePrivate(KLineEditFrame *parent);

private:
    KLineEditFrame *q_ptr;
    QLineEdit *m_pLineEdit;
    KLabel *m_pSettingName;
    QHBoxLayout *m_pMainLayout;
};

KLineEditFramePrivate::KLineEditFramePrivate(KLineEditFrame *parent)
    : q_ptr(parent)
{
    Q_Q(KLineEditFrame);
    setParent(parent);
    m_pMainLayout = new QHBoxLayout(q);
    m_pMainLayout->setContentsMargins(16, 0, 16, 0);
    m_pMainLayout->setSpacing(16);
    m_pSettingName = new KLabel(q);
    m_pSettingName->setFixedWidth(SETTING_LABEL_WIDTH);
    m_pLineEdit = new QLineEdit(q);
    m_pMainLayout->addWidget(m_pSettingName);
    m_pMainLayout->addWidget(m_pLineEdit);
    q->setLayout(m_pMainLayout);
}

KLineEditFrame::KLineEditFrame(const QString title, QWidget *parent, BorderRadiusStyle style)
    : KBaseFrame(parent, style, true)
    , d_ptr(new KLineEditFramePrivate(this))
{
    Q_D(KLineEditFrame);
    setObjectName("KLineEditFrame");
    d->m_pSettingName->setText(title);
}

KLineEditFrame::~KLineEditFrame()
{
}

QLineEdit *KLineEditFrame::lineEdit()
{
    Q_D(KLineEditFrame);
    return d->m_pLineEdit;
}

QHBoxLayout *KLineEditFrame::mainHLayout()
{
    Q_D(KLineEditFrame);
    return d->m_pMainLayout;
}

KLabel *KLineEditFrame::titleLabel()
{
    Q_D(KLineEditFrame);
    return d->m_pSettingName;
}
}

#include "klineeditframe.moc"
#include "moc_klineeditframe.cpp"
