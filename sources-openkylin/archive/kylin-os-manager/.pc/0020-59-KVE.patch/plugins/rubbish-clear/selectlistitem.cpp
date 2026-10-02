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

#include "selectlistitem.h"
namespace KylinRubbishClear
{

SelectListItem::SelectListItem(QWidget *parent, QString description, QString tipMsg, bool hasTip, int itemWidth,
                               bool check)
    : QWidget(parent), m_description(description), m_tip(tipMsg), m_hasTip(hasTip)
{
    this->setFixedHeight(30);
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(12);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_checkBox = new QCheckBox(this);
    m_checkBox->setFixedSize(20, 20);
    m_checkBox->setFocusPolicy(Qt::NoFocus);
    if (check)
        m_checkBox->setChecked(true);
    else
        m_checkBox->setChecked(false);

    connect(m_checkBox, &QCheckBox::clicked, [=](bool checked) {
        Q_EMIT this->selectedSignal(checked, this->itemDescription());
    });

    int maxWidth = itemWidth - m_checkBox->width() - 12;
    m_descLabel = new CustomLabel(this);
    m_descLabel->setFixedWidth(maxWidth);
    m_descLabel->setText(description);

    //    QFont ft;
    //    QFontMetrics fm(ft);
    //    QString elided_text = fm.elidedText(description, Qt::ElideRight, 470);
    //    m_descLabel->setToolTip(description);
    //    m_descLabel->setText(elided_text);
    //    m_descLabel->setStyleSheet("color:black;");

    /*QFont font;
    font.setPixelSize(12);
    const QFontMetrics fm(font);
    QString msg = fm.elidedText(description, Qt::ElideMiddle, maxWidth);
//    m_descLabel->setWordWrap(true);
    m_descLabel->setText(msg);*/

    m_mainLayout->setAlignment(Qt::AlignLeft);
    m_mainLayout->addWidget(m_checkBox, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_mainLayout->addWidget(m_descLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
}

SelectListItem::~SelectListItem() {}

bool SelectListItem::itemIsChecked()
{
    return m_checkBox->isChecked();
}

QString SelectListItem::itemDescription()
{
    if (m_hasTip)
        return this->m_tip;
    else
        return this->m_description;
}

} // namespace KylinRubbishClear
