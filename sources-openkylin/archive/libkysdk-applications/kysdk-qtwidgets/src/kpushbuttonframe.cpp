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
#include "kpushbuttonframe.h"

#include <QDebug>

namespace kdk
{

class KPushButtonFramePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KPushButtonFrame)
public:
    KPushButtonFramePrivate(KPushButtonFrame *parent);

private:
    KPushButtonFrame *q_ptr;
    KPushButton *m_pPushButton;
    KLabel *m_pSettingName;
    QHBoxLayout *m_pMainLayout;
};

KPushButtonFramePrivate::KPushButtonFramePrivate(KPushButtonFrame *parent)
    : q_ptr(parent)
{
    Q_Q(KPushButtonFrame);
    setParent(parent);
    m_pMainLayout = new QHBoxLayout(q);
    m_pMainLayout->setContentsMargins(16, 0, 16, 0);
    m_pMainLayout->setSpacing(16);

    m_pSettingName = new KLabel(q);

    m_pPushButton = new KPushButton();
    m_pPushButton->setFixedWidth(100);

    m_pMainLayout->addWidget(m_pSettingName);
    m_pMainLayout->addStretch();
    m_pMainLayout->addWidget(m_pPushButton);
    q->setLayout(m_pMainLayout);
}

KPushButtonFrame::KPushButtonFrame(const QString title, QWidget *parent, BorderRadiusStyle style)
    : KBaseFrame(parent, style, true)
    , d_ptr(new KPushButtonFramePrivate(this))
{
    Q_D(KPushButtonFrame);
    setObjectName("KPushButtonFrame");

    d->m_pSettingName->setText(title);
}

KPushButtonFrame::~KPushButtonFrame()
{
}

KPushButton *KPushButtonFrame::pushButton()
{
    Q_D(KPushButtonFrame);
    return d->m_pPushButton;
}

QHBoxLayout *KPushButtonFrame::mainHLayout()
{
    Q_D(KPushButtonFrame);
    return d->m_pMainLayout;
}

KLabel *KPushButtonFrame::titleLabel()
{
    Q_D(KPushButtonFrame);
    return d->m_pSettingName;
}

}

#include "kpushbuttonframe.moc"
#include "moc_kpushbuttonframe.cpp"
