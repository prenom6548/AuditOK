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

#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H
#include <QEvent>
#include <QWebEngineView>
#include "locationlistwidget.h"
#include "searchbox.h"
namespace kmre_gps
{
class SearchView : public QWidget
{
    Q_OBJECT
public:
    SearchView(QWidget *parent = 0);
    ~SearchView();
    LocationListWidget *m_locationlistWidget = nullptr;
    QWebEngineView *m_view = nullptr;
    SearchBox *m_searchBox = nullptr;
    QPushButton *m_nowLocationBtn = nullptr;

private:
    //    QPushButton *m_searchBtn = nullptr;
    QLabel *m_searchText = nullptr;
    void initWidget();
    void initConnection();

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void searchClicked();
    void slotcurrentShow();
    void slotlistHide();
    void slotNowClicked();

Q_SIGNALS:
    void sigcurrentShow();
    void sigupdateCurrentShow();
};
} // namespace kmre_gps

#endif // SEARCHRESULT_H
