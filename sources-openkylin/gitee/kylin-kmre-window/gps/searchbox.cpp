/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
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

#include "searchbox.h"
namespace kmre_gps
{
SearchBox::SearchBox(QWidget *parent) : QLineEdit(parent)
{
    this->setFocusPolicy(Qt::ClickFocus);
    this->resize(526, 36);

    //搜索框的底部的搜索图片
    m_searchBottom = new QPushButton(this);
    m_searchBottom->resize(30, 30);
    m_searchBottom->setFlat(true);
    m_searchBottom->move(200, (this->height() - m_searchBottom->height()) / 2);
    m_searchBottom->setIcon(QIcon(":/res/search_light.png"));
    m_searchBottom->setIconSize(QSize(15, 15));
    // m_searchBottom->lower();
    m_searchBottom->setEnabled(false);

    m_searchText = new QLabel(this);
    m_searchText->setText(tr("entertext"));
    m_searchText->move(m_searchBottom->x() + m_searchBottom->width() + 8,
                       (this->height() - m_searchText->height()) / 2 + 2);
    QPalette pal;
    pal.setColor(QPalette::Text, Qt::lightGray);
    m_searchText->setPalette(pal);
    m_searchText->lower();

    QMargins margins = this->textMargins();
    this->setTextMargins(23, margins.top(), m_searchBottom->width(), margins.bottom());
}

bool SearchBox::event(QEvent *e)
{
    if (e->type() == QEvent::FocusOut) {
        m_isFocus = false;
        Q_EMIT siglistHide();
        this->clearFocus();
        if (this->text() == "") {
            this->m_searchBottom->move(200, (this->height() - this->m_searchBottom->height()) / 2);
            this->m_searchText->show();
            this->m_searchBottom->show();
        }
        dealIconText(m_isFocus);
        return QLineEdit::event(e);
    } else if (e->type() == QEvent::FocusIn) {
        m_isFocus = true;
        dealIconText(m_isFocus);
        return QLineEdit::event(e);
    }
    return QLineEdit::event(e);
}

void SearchBox::dealIconText(bool isFoucs)
{
    if (isFoucs) {
        m_searchBottom->move(5, (this->height() - m_searchBottom->height()) / 2);
        m_searchText->hide();
    }
}

SearchBox::~SearchBox() {}

} // namespace kmre_gps
