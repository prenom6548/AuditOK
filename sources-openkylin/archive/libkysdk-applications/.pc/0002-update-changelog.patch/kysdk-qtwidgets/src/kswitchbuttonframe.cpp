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
#include "kswitchbuttonframe.h"
#include "themeController.h"
#include <QDebug>
namespace kdk
{

class KSwitchButtonFramePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KSwitchButtonFrame)
public:
    KSwitchButtonFramePrivate(KSwitchButtonFrame *parent);

private:
    KSwitchButtonFrame *q_ptr;

    QVBoxLayout *m_pVLayout;
    KLabel *m_pHintLabel;
    KSwitchButton *m_pSwitchButton;
    KLabel *m_pSettingName;
    QHBoxLayout *m_pMainLayout;
};

KSwitchButtonFramePrivate::KSwitchButtonFramePrivate(KSwitchButtonFrame *parent)
    : q_ptr(parent)
{
    Q_Q(KSwitchButtonFrame);
    setParent(parent);

    m_pSettingName = new KLabel(q);

    m_pHintLabel = new KLabel(q);
    m_pHintLabel->setVisible(false);
    m_pHintLabel->setFontColor(ThemeController::getPaletteColorFromDT("placeholdertext"));
    m_pVLayout = new QVBoxLayout();
    m_pVLayout->addWidget(m_pSettingName);
    m_pVLayout->addWidget(m_pHintLabel);

    m_pSwitchButton = new KSwitchButton();

    m_pMainLayout = new QHBoxLayout(q);
    m_pMainLayout->setContentsMargins(16, 0, 16, 0);
    m_pMainLayout->addLayout(m_pVLayout);
    m_pMainLayout->addStretch();
    m_pMainLayout->addWidget(m_pSwitchButton);

    q->setLayout(m_pMainLayout);
}

KSwitchButtonFrame::KSwitchButtonFrame(const QString title, QWidget *parent, BorderRadiusStyle style)
    : KBaseFrame(parent, style, true)
    , d_ptr(new KSwitchButtonFramePrivate(this))
{
    Q_D(KSwitchButtonFrame);
    d->setObjectName("KSwitchButtonFrame");

    d->m_pSettingName->setText(title);
}

KSwitchButtonFrame::~KSwitchButtonFrame()
{
}

void KSwitchButtonFrame::setHintText(QString &hintText)
{
    Q_D(KSwitchButtonFrame);
    d->m_pHintLabel->setText(hintText);
    d->m_pHintLabel->setVisible(true);
}

KSwitchButton *KSwitchButtonFrame::switchButton()
{
    Q_D(KSwitchButtonFrame);
    return d->m_pSwitchButton;
}

QHBoxLayout *KSwitchButtonFrame::mainHLayout()
{
    Q_D(KSwitchButtonFrame);
    return d->m_pMainLayout;
}

KLabel *KSwitchButtonFrame::titleLabel()
{
    Q_D(KSwitchButtonFrame);
    return d->m_pSettingName;
}

}

#include "kswitchbuttonframe.moc"
#include "moc_kswitchbuttonframe.cpp"
