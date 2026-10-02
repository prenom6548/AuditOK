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
#include "kradiobuttonframe.h"

#include <QDebug>
namespace kdk
{

class KRadioButtonFramePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KRadioButtonFrame)
public:
    KRadioButtonFramePrivate(KRadioButtonFrame *parent);

private:
    KRadioButtonFrame *q_ptr;

    QButtonGroup *m_pButtonGroup;
    QHBoxLayout *m_pRadioLayout;
    KLabel *m_pSettingName;
    QHBoxLayout *m_pMainLayout;
};

KRadioButtonFramePrivate::KRadioButtonFramePrivate(KRadioButtonFrame *parent)
    : q_ptr(parent)
{
    Q_Q(KRadioButtonFrame);
    m_pMainLayout = new QHBoxLayout(q);
    m_pMainLayout->setContentsMargins(16, 0, 16, 0);
    m_pMainLayout->setSpacing(16);
    m_pSettingName = new KLabel(q);
    m_pSettingName->setFixedWidth(SETTING_LABEL_WIDTH);

    m_pButtonGroup = new QButtonGroup(q);
    m_pRadioLayout = new QHBoxLayout();
    m_pRadioLayout->setContentsMargins(0, 0, 0, 0);
    m_pRadioLayout->setSpacing(80);
    m_pMainLayout->addWidget(m_pSettingName);
    m_pMainLayout->addLayout(m_pRadioLayout);
    m_pMainLayout->addStretch();
    q->setLayout(m_pMainLayout);
}

KRadioButtonFrame::KRadioButtonFrame(const QString title, QWidget *parent, BorderRadiusStyle style)
    : KBaseFrame(parent, style, true)
    , d_ptr(new KRadioButtonFramePrivate(this))
{
    Q_D(KRadioButtonFrame);
    setObjectName("KRadioButtonFrame");

    d->m_pSettingName->setText(title);
}

KRadioButtonFrame::~KRadioButtonFrame()
{
}

void KRadioButtonFrame::addButton(QRadioButton *button, int id, bool addStretch)
{
    Q_D(KRadioButtonFrame);
    d->m_pButtonGroup->addButton(button, id);
    d->m_pRadioLayout->addWidget(button, 0, Qt::AlignLeft | Qt::AlignVCenter);
    if (addStretch) {
        d->m_pRadioLayout->addStretch();
    }
}

QHBoxLayout *KRadioButtonFrame::radioHLayout()
{
    Q_D(KRadioButtonFrame);
    return d->m_pRadioLayout;
}

QButtonGroup *KRadioButtonFrame::buttonGroup()
{
    Q_D(KRadioButtonFrame);
    return d->m_pButtonGroup;
}

QHBoxLayout *KRadioButtonFrame::mainHLayout()
{
    Q_D(KRadioButtonFrame);
    return d->m_pMainLayout;
}

KLabel *KRadioButtonFrame::titleLabel()
{
    Q_D(KRadioButtonFrame);
    return d->m_pSettingName;
}

}

#include "kradiobuttonframe.moc"
#include "moc_kradiobuttonframe.cpp"
