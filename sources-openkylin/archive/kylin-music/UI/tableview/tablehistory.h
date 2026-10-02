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

#ifndef TABLEHISTORY_H
#define TABLEHISTORY_H

#include <QDialog>
#include <QWidget>
#include <QListView>
#include <QLabel>
#include <QToolButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSqlRelationalTableModel>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QVBoxLayout>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>

#include "customTreeView.h"
#include "UIControl/tableview/musiclistmodel.h"
#include "UIControl/base/musicDataBase.h"
#include "UIControl/player/player.h"
#include "UI/base/widgetstyle.h"


/**
 * @brief The TableHistory class
 *
 * fix bug#162535 【音乐】播放历史窗口位于整个桌面的最顶层
 *
 * 继承于QWidget不好用，会出现以下情况：
 *      1. 当前窗口在任务栏中显示
 *      2. 焦点失真
 *      3. 即便设置QTool， subwindow和也不行
 */
class TableHistory : public QDialog
{
    Q_OBJECT

    friend class unitMainWindow;

public:
    explicit TableHistory(QWidget *parent = nullptr);

    MusicListModel *m_model = nullptr;
    CustomTreeView *m_view = nullptr;

    bool isHightLight = false;
    QMap <QString ,QString> removeList = {{"",""}}; // 移除列表

    void initRightMenu();
    void initSetModel();
    void initConnect();
    void initStyle();
    void initTableStyle();

    void changeNumber();
    void setHighlight(int index);
    void refreshHistoryTable();
    void noRefreshHistory();
    void deleteAllClicked();
    void deleteAllPlaylist();
    void changePlayHistoryPos(int posX, int posY, int width, int height);
    void displayBtn(QToolButton *btn, QString btnTooltip);

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
    void changeEvent(QEvent* event) override;

public Q_SLOTS:
    void showHistroryPlayList();
    void slotPlayIndexChanged(int index, QString listname);
    void slotPlayPathChanged(QString songPath);
    void slotLableSetFontSize(int size);
    void playSongsDoubleClicked(QModelIndex index);
    void playSongs();
    void deleteSongs();
    void showRightMenu(const QPoint &pos);
    void slotAddToRList(QString path,QString owner);
    void slotRemoveFromRList(QString path);


Q_SIGNALS:
    void signalHistoryPlaying();
    void signalHistoryBtnChecked(bool checked);

private:
    QVBoxLayout *mainVLayout = nullptr;
    QHBoxLayout *titleHBoxLayout = nullptr;
    QLabel *listCountLabel = nullptr;
    QLabel *historyTitileLabel = nullptr;
    QToolButton *deleteAllBtn = nullptr;
    QWidget *historyTitileWidget = nullptr;

    QAction *playRow = nullptr;
    QAction *removeRow = nullptr;
    QAction *playNextRow = nullptr;
    QMenu *m_menu = nullptr;
    QString nowListName = HISTORY;
    QString nowPlayListName = "";
    int nowPlayIndex = -1;

    QWidget *nullPageWidget = nullptr;
    QVBoxLayout *nullPageLayout = nullptr;
    QLabel *nullIconLabel = nullptr;
    QLabel *nullTextLabel = nullptr;

    QHeaderView *horizonHeader = nullptr;
    QWidget *tableWidget = nullptr; // 歌曲列表Widget,用来包含tableHBoxLayout
    QHBoxLayout *tableHBoxLayout = nullptr; // 用来包含m_tableHistory

    int playHistoryPosX;
    int playHistoryPosY;
    int playHistoryPosWidth;
    int playHistoryPosHeight;
};

#endif // TABLEHISTORY_H
