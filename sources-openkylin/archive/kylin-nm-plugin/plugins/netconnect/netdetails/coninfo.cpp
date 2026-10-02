/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "coninfo.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QColor>
#include <QList>
#include <QComboBox>
#include <QFrame>

LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent)
{
}

LineEdit::~LineEdit()
{
}

void LineEdit::setOobeMode(bool on) {
    if (on) {
        // ensure stylesheet backgrounds are honored
        this->setAttribute(Qt::WA_StyledBackground, true);
        const QString oobeQss =R"(
                QLineEdit {
                    background: #4A4A4A;
                    border-radius: 6px;
                }
                QLineEdit:focus {
                    border: 2px solid #3790FA;
                    background: #000000;
                    color: #ffffff;
                }
                QLineEdit:!focus {
                    border: transparent;
                    background: #4A4A4A;
                    color: #ffffff;
                }  
            )";
        this->setStyleSheet(oobeQss);
        this->setFixedHeight(30);
    }
}

void LineEdit::contextMenuEvent(QContextMenuEvent *event) {
    QMenu *menu = createStandardContextMenu(); // 默认的标准右键菜单
    menu->setPalette(this->palette());
    menu->exec(event->globalPos());
    delete menu;
}

void setFramePalette(QFrame *widget, QPalette &pal) {
    QList<LineEdit *> lineEditList = widget->findChildren<LineEdit *>();
    for (int i = 0; i < lineEditList.count(); ++i) {
        lineEditList.at(i)->setPalette(pal);
        lineEditList.at(i)->setContextMenuPolicy(Qt::DefaultContextMenu);
    }
    QList<QComboBox *> comboBoxList = widget->findChildren<QComboBox *>();
    for (int i = 0; i < comboBoxList.count(); ++i) {
        comboBoxList.at(i)->setPalette(pal);
        if (comboBoxList.at(i)->view()) {
            comboBoxList.at(i)->view()->setPalette(pal);
        }
    }
}

QPalette lightPalette(QWidget *widget)
{
    auto palette = widget->palette();
    //ukui-light palette UKUI3.1
    QColor windowText_at(38, 38, 38),
           windowText_iat(38, 38, 38),
           windowText_dis(166, 166, 166),
           button_at(230, 230, 230),
           button_iat(230, 230, 230),
           button_dis(233, 233, 233),
           light_at(255, 255, 255),
           light_iat(255, 255, 255),
           light_dis(242, 242, 242),
           midlight_at(218, 218, 218),
           midlight_iat(218, 218, 218),
           midlight_dis(230, 230, 230),
           dark_at(77, 77, 77),
           dark_iat(77, 77, 77),
           dark_dis(64, 64, 64),
           mid_at(115, 115, 115),
           mid_iat(115, 115, 115),
           mid_dis(102, 102, 102),
           text_at(38, 38, 38),
           text_iat(38, 38, 38),
           text_dis(140, 140, 140),
           brightText_at(89, 89, 89),
           brightText_iat(89, 89, 89),
           brightText_dis(77, 77, 77),
           buttonText_at(38, 38, 38),
           buttonText_iat(38, 38, 38),
           buttonText_dis(179, 179, 179),
           base_at(255, 255, 255),
           base_iat(245, 245, 245),
           base_dis(237, 237, 237),
           window_at(245, 245, 245),
           window_iat(237, 237, 237),
           window_dis(230, 230, 230),
           shadow_at(0, 0, 0, 16),
           shadow_iat(0, 0, 0, 16),
           shadow_dis(0, 0, 0, 21),
//           shadow_at(214, 214, 214),
//           shadow_iat(214, 214, 214),
//           shadow_dis(201, 201, 201),
           highLight_at(55, 144, 250),
           highLight_iat(55, 144, 250),
           highLight_dis(233, 233, 233),
           highLightText_at(255, 255, 255),
           highLightText_iat(255, 255, 255),
           highLightText_dis(179, 179, 179),
           link_at(55, 144, 250),
           link_iat(55, 144, 250),
           link_dis(55, 144, 250),
           linkVisited_at(114, 46, 209),
           linkVisited_iat(114, 46, 209),
           linkVisited_dis(114, 46, 209),
           alternateBase_at(245, 245, 245),
           alternateBase_iat(245, 245, 245),
           alternateBase_dis(245, 245, 245),
           noRale_at(240, 240, 240),
           noRole_iat(240, 240, 240),
           noRole_dis(217, 217, 217),
           toolTipBase_at(255, 255, 255),
           toolTipBase_iat(255, 255, 255),
           toolTipBase_dis(255, 255, 255),
           toolTipText_at(38, 38, 38),
           toolTipText_iat(38, 38, 38),
           toolTipText_dis(38, 38, 38),
           placeholderText_at(38, 38, 38),
           placeholderText_iat(38, 38, 38),
           placeholderText_dis(38, 38, 38);



    palette.setColor(QPalette::Active, QPalette::WindowText, windowText_at);
    palette.setColor(QPalette::Inactive, QPalette::WindowText, windowText_iat);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, windowText_dis);

    palette.setColor(QPalette::Active, QPalette::Button, button_at);
    palette.setColor(QPalette::Inactive, QPalette::Button, button_iat);
    palette.setColor(QPalette::Disabled, QPalette::Button, button_dis);

    palette.setColor(QPalette::Active, QPalette::Light, light_at);
    palette.setColor(QPalette::Inactive, QPalette::Light, light_iat);
    palette.setColor(QPalette::Disabled, QPalette::Light, light_dis);

    palette.setColor(QPalette::Active, QPalette::Midlight, midlight_at);
    palette.setColor(QPalette::Inactive, QPalette::Midlight, midlight_iat);
    palette.setColor(QPalette::Disabled, QPalette::Midlight, midlight_dis);

    palette.setColor(QPalette::Active, QPalette::Dark, dark_at);
    palette.setColor(QPalette::Inactive, QPalette::Dark, dark_iat);
    palette.setColor(QPalette::Disabled, QPalette::Dark, dark_dis);

    palette.setColor(QPalette::Active, QPalette::Mid, mid_at);
    palette.setColor(QPalette::Inactive, QPalette::Mid, mid_iat);
    palette.setColor(QPalette::Disabled, QPalette::Mid, mid_dis);

    palette.setColor(QPalette::Active, QPalette::Text, text_at);
    palette.setColor(QPalette::Inactive, QPalette::Text, text_iat);
    palette.setColor(QPalette::Disabled, QPalette::Text, text_dis);

    palette.setColor(QPalette::Active, QPalette::BrightText, brightText_at);
    palette.setColor(QPalette::Inactive, QPalette::BrightText, brightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::BrightText, brightText_dis);

    palette.setColor(QPalette::Active, QPalette::ButtonText, buttonText_at);
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, buttonText_iat);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, buttonText_dis);

    palette.setColor(QPalette::Active, QPalette::Base, base_at);
    palette.setColor(QPalette::Inactive, QPalette::Base, base_iat);
    palette.setColor(QPalette::Disabled, QPalette::Base, base_dis);

    palette.setColor(QPalette::Active, QPalette::Window, window_at);
    palette.setColor(QPalette::Inactive, QPalette::Window, window_iat);
    palette.setColor(QPalette::Disabled, QPalette::Window, window_dis);

    palette.setColor(QPalette::Active, QPalette::Shadow, shadow_at);
    palette.setColor(QPalette::Inactive, QPalette::Shadow, shadow_iat);
    palette.setColor(QPalette::Disabled, QPalette::Shadow, shadow_dis);

    palette.setColor(QPalette::Active, QPalette::Highlight, highLight_at);
    palette.setColor(QPalette::Inactive, QPalette::Highlight, highLight_iat);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, highLight_dis);

    palette.setColor(QPalette::Active, QPalette::HighlightedText, highLightText_at);
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, highLightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, highLightText_dis);

    palette.setColor(QPalette::Active, QPalette::Link, link_at);
    palette.setColor(QPalette::Inactive, QPalette::Link, link_iat);
    palette.setColor(QPalette::Disabled, QPalette::Link, link_dis);

    palette.setColor(QPalette::Active, QPalette::LinkVisited, linkVisited_at);
    palette.setColor(QPalette::Inactive, QPalette::LinkVisited, linkVisited_iat);
    palette.setColor(QPalette::Disabled, QPalette::LinkVisited, linkVisited_dis);

    palette.setColor(QPalette::Active, QPalette::AlternateBase, alternateBase_at);
    palette.setColor(QPalette::Inactive, QPalette::AlternateBase, alternateBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, alternateBase_dis);

    palette.setColor(QPalette::Active, QPalette::NoRole, noRale_at);
    palette.setColor(QPalette::Inactive, QPalette::NoRole, noRole_iat);
    palette.setColor(QPalette::Disabled, QPalette::NoRole, noRole_dis);

    palette.setColor(QPalette::Active, QPalette::ToolTipBase, toolTipBase_at);
    palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, toolTipBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, toolTipBase_dis);

    palette.setColor(QPalette::Active, QPalette::ToolTipText, toolTipText_at);
    palette.setColor(QPalette::Inactive, QPalette::ToolTipText, toolTipText_iat);
    palette.setColor(QPalette::Disabled, QPalette::ToolTipText, toolTipText_dis);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setColor(QPalette::Active, QPalette::PlaceholderText, placeholderText_at);
    palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, placeholderText_iat);
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, placeholderText_dis);
#endif

    return palette;

}
