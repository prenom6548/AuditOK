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
#include "kcomboboxframe.h"

namespace kdk
{

class KComboBoxFramePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KComboBoxFrame)
public:
    KComboBoxFramePrivate(KComboBoxFrame *parent);

private:
    KComboBoxFrame *q_ptr;

    QComboBox *m_pCombox;
    KLabel *m_pSettingName;
    QHBoxLayout *m_pMainLayout;
};

KComboBoxFramePrivate::KComboBoxFramePrivate(KComboBoxFrame *parent)
    : q_ptr(parent)
{
    Q_Q(KComboBoxFrame);
    setParent(parent);

    m_pMainLayout = new QHBoxLayout(q);
    m_pMainLayout->setContentsMargins(16, 0, 16, 0);
    m_pMainLayout->setSpacing(16);
    m_pSettingName = new KLabel(q);
    m_pSettingName->setFixedWidth(SETTING_LABEL_WIDTH);
    m_pMainLayout->addWidget(m_pSettingName);
    m_pCombox = new QComboBox(q);
    m_pMainLayout->addWidget(m_pCombox);

    q->setLayout(m_pMainLayout);
}

KComboBoxFrame::KComboBoxFrame(const QString title, QWidget *parent, BorderRadiusStyle style)
    : KBaseFrame(parent, style, true)
    , d_ptr(new KComboBoxFramePrivate(this))
{
    Q_D(KComboBoxFrame);
    setObjectName("KComboBoxFrame");
    d->m_pSettingName->setText(title);
}

KComboBoxFrame::~KComboBoxFrame()
{
}

QComboBox *KComboBoxFrame::comboBox()
{
    Q_D(KComboBoxFrame);
    return d->m_pCombox;
}

QHBoxLayout *KComboBoxFrame::mainHLayout()
{
    Q_D(KComboBoxFrame);
    return d->m_pMainLayout;
}

KLabel *KComboBoxFrame::titleLabel()
{
    Q_D(KComboBoxFrame);
    return d->m_pSettingName;
}

}

#include "kcomboboxframe.moc"
#include "moc_kcomboboxframe.cpp"
