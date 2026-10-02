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

#ifndef SEARCHBOX_H
#define SEARCHBOX_H
#include <QLineEdit>
#include <QLabel>
#include <QEvent>
#include <QPushButton>

namespace kmre_gps
{
class SearchBox : public QLineEdit
{
    Q_OBJECT
public:
    SearchBox(QWidget *parent = nullptr);
    ~SearchBox();
    QPushButton *m_searchBottom = nullptr;
    QLabel *m_searchText = nullptr;

private:
    bool m_isFocus = false;
    void dealIconText(bool isFoucs);

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void siglistHide();
};
} // namespace kmre_gps

#endif // SEARCHBOX_H
