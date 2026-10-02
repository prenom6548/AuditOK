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
#include "ksliderframe.h"

#include <QDebug>
namespace kdk
{

class KSliderFramePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KSliderFrame)
public:
    KSliderFramePrivate(KSliderFrame *parent);

private:
    KSliderFrame *q_ptr;

    KSlider *m_pSlider;
    QLabel *m_pLeftLabel;
    QLabel *m_pRightLabel;
    KLabel *m_pSettingName;
    QHBoxLayout *m_pMainLayout;
};

KSliderFramePrivate::KSliderFramePrivate(KSliderFrame *parent)
    : q_ptr(parent)
{
    Q_Q(KSliderFrame);
    setParent(parent);
    m_pSettingName = new KLabel(q);
    m_pSettingName->setFixedWidth(SETTING_LABEL_WIDTH);

    m_pSlider = new KSlider(q);
    m_pLeftLabel = new QLabel(q);
    m_pLeftLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_pRightLabel = new QLabel(q);
    m_pRightLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    m_pMainLayout = new QHBoxLayout(q);
    m_pMainLayout->setContentsMargins(16, 0, 16, 0);
    m_pMainLayout->addWidget(m_pSettingName);
    m_pMainLayout->addSpacing(8);
    m_pMainLayout->addWidget(m_pLeftLabel);
    m_pMainLayout->addWidget(m_pSlider);
    m_pMainLayout->addWidget(m_pRightLabel);

    q->setLayout(m_pMainLayout);
}

KSliderFrame::KSliderFrame(QString title, bool hasNode, QWidget *parent, BorderRadiusStyle style, Qt::Orientation orientation)
    : KBaseFrame(parent, style, true)
    , d_ptr(new KSliderFramePrivate(this))
{
    Q_D(KSliderFrame);
    setObjectName("KSliderFrame");

    d->m_pSettingName->setText(title);
    d->m_pSlider->setNodeVisible(hasNode);
    d->m_pSlider->setOrientation(orientation);
}

KSliderFrame::~KSliderFrame()
{
}

QLabel *KSliderFrame::leftLabel()
{
    Q_D(KSliderFrame);
    return d->m_pLeftLabel;
}

QLabel *KSliderFrame::rightLabel()
{
    Q_D(KSliderFrame);
    return d->m_pRightLabel;
}

KSlider *KSliderFrame::slider()
{
    Q_D(KSliderFrame);
    return d->m_pSlider;
}

QHBoxLayout *KSliderFrame::mainHLayout()
{
    Q_D(KSliderFrame);
    return d->m_pMainLayout;
}

KLabel *KSliderFrame::titleLabel()
{
    Q_D(KSliderFrame);
    return d->m_pSettingName;
}
}

#include "ksliderframe.moc"
#include "moc_ksliderframe.cpp"
