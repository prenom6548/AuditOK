/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <QWidget>
#include <QKeyEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QListView>
#include <QTreeView>
#include <QFrame>
#include <QX11Info>
#include <QApplication>
#include <QScreen>
#include <QTimer>

#include "UI/search/musicsearchlistview.h"
#include "UI/player/searchedit.h"

class MusicSearchListview;
class SearchEdit;
class SearchResult : public QWidget
{
    Q_OBJECT

public:
    explicit SearchResult(QWidget *parent = nullptr);
    ~SearchResult();

    void autoResize();
    //设置各列表搜索关键字
    void setListviewSearchString(const QString &str);
    //点击列表时设置lineEdit显示文字
    void setLineEditSearchString(const QString &str);
    void selectUp();
    void selectDown();
    int  getCurrentIndex();
    void setSearchEdit(SearchEdit *edit);

    void changeSrearchResultPos(int posX, int posY, int width, int height);

public Q_SLOTS:
    void onReturnPressed();
    void slotLableSetFontSize(int size);
    void slotMusicItemClicked(QModelIndex index);
    void slotSingerItemClicked(QModelIndex index);
    void slotAlbumItemClicked(QModelIndex index);

protected:
#ifndef WHITE_TEST
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event);
#endif
Q_SIGNALS:
    void signalFilePath(QString path);
    void signalSongListBySinger(QString singer);
    void signalSongListByAlbum(QString album);

public:
    MusicSearchListview *m_MusicView  = nullptr;
    MusicSearchListview *m_SingerView = nullptr;
    MusicSearchListview *m_AlbumView  = nullptr;
#ifndef WHITE_TEST
    void setCursorWithXEvent();
#endif
private:
    QVBoxLayout *vlayout = nullptr;

    QLabel *m_MusicLabel = nullptr;
    QLabel *m_SingerLabel = nullptr;
    QLabel *m_AlbumLabel = nullptr;

    int               m_CurrentIndex  = -1;
    int               m_Count         = 0;
    QFrame *m_SingerFrame = nullptr;
    QFrame *m_AlbumFrame  = nullptr;

    SearchEdit          *m_searchEdit = nullptr;

    int m_resultPosX;
    int m_resultPosY;
    int m_resultPosWidth;
    int m_resultPosHeight;
    quint32 m_key;
};

#endif // SEARCHRESULT_H
