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

#ifndef TableOne_H
#define TableOne_H

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QVBoxLayout>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QScrollArea>
#include <QScrollBar>
#include <QProcess>

#include "UIControl/base/musicDataBase.h"
#include "UIControl/tableview/musiclistmodel.h"
#include "UIControl/player/player.h"
#include "customTreeViewDelegate.h"
#include "customTreeView.h"
#include "UIControl/base/musicfileinformation.h"
#include "UI/base/customLabel.h"
#include "UI/base/widgetstyle.h"
#include "UI/base/popupDialog.h"
#include "UI/dbusadapter.h"

class TableOne : public QWidget
{
    Q_OBJECT

    friend class unitMainWindow;

public:
    explicit TableOne(QString listName,QWidget *parent = nullptr);
    ~TableOne();

    /**
     * 经验一(Tip1)：基于MVC设计模式设计的, C(controllor)控制在Qt中被弱化，与View合并到一起。
     *
     * 只有一个数据集 `MusicListModel`，和一个显示视图 `CustomTreeView`；
     * 但有2种显示方式，分别是在  `tableone.cpp` 和 `tablehistory.cpp`
     * 2种实现方式是通过设置 `CustomTreeView` 的不同属性来实现的，具体差异可看这两个个文件。
     *
     * 其中，`CustomTreeView` 又通过 `CustomTreeViewDelegate` 来设置不同的行宽。
     */
    MusicListModel *m_model = nullptr; // 数据模型，不能单独显示出来。可以设置数据文本对齐方式
    CustomTreeView *m_view = nullptr; // 视图，要来显示数据模型 `MusicListModel`， 歌单列表，包括歌单标题和内容

    QLabel *listTitleLabel = nullptr; // 歌单名称
    QString nowListName;
    MusicInfoDialog *infoDialog = nullptr;
    QString m_playbackStatus = "Stopped";
    void initStyle();
    void initUI();
    void initConnect();
    void initRightMenu();
    void showRightMenu(const QPoint &pos);

    void tableViewDoubleClicked();
    void changeNumber();
    void initTableViewStyle();
    void setHightLightAndSelect();
    void getMusicList();
    void playMusicforIndex(QString listName,int index);
    void showTitleText(QString listName);

    void importFinished(int successCount, int failCount, int allCount);
    void importFailed(int successCount, int failCount, int allCount);

    void isDeleteSongs();
    void isDeleteLocalSongs();
    void deleteSongs();
    void deleteLocalSongs();
    void clearSearchList();

    void playSongs();
    void showInfo();

    void addMusicToDatebase(QStringList fileNames);
    void addToOtherList(QAction *listNameAction);
    void addMusicSlot();
    void addDirMusicSlot();
    template<typename ButtonType>
    void displayBtn(ButtonType *btn, QString btnTooltip);

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
#ifndef WHITE_TEST
    void dragEnterEvent(QDragEnterEvent *event)Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event)Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event)Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
#endif
public Q_SLOTS:
    void slotSearchTexts(QString text);
    void slotSearchReturnPressed(QString listName);
    void slotLableSetFontSize(int size);
    void slotFilePath(QString path);
    void slotSongListBySinger(QString singer);
    void slotSongListByAlbum(QString album);

    void selectListChanged(QString listname);
    void playListRenamed(QString oldName,QString newName);
    void getHightLightIndex(int index, QString listName);

    void playAll(QString listName);
    void playAllButtonClicked();
    void slotReturnText(QString text);

Q_SIGNALS:
    void sendPathToPlayer(QString fp);
    void countChanges();
    void hoverIndexChanged(QModelIndex index);
    void refreshHistoryListSignal();
    void addILoveFilepathSignal(QString filePath);  //传递我喜欢歌单中添加歌曲的信号
    void removeILoveFilepathSignal(QString filePath);  //传递我喜欢歌单中删除歌曲的信号
    void signalListSearch();    //取消侧边栏所有按钮的选中状态
    void signalSongListHigh();  //高亮左侧歌曲列表按钮
    void sigAddToRList(QString path, QString owner ); 
    void sigRemoveFromRList(QString Path);

    void sigShowMenu();
private:
    bool moveFileToTrashNative(const QString &filePath);
private:
    QMenu *m_menu = nullptr;  //新建一个Menu属性
    QSqlQueryModel *tableModel = nullptr;

    int showScrollbarNumber = 10;
    QString m_text;
    QMap<int,QString> m_map;

    QAction * playRow = nullptr;
    QAction * removeRow = nullptr;
    QAction * removeLocalRow = nullptr;
    QAction * showInfoRow = nullptr;
    QMenu * addToOtherListMenu = nullptr;
    QMap<int,QString> getSelectedTaskIdList();

    QMenu *add_menu = nullptr;
    QAction *addMusicFileAction = nullptr;
    QAction *addDirMusicAction = nullptr;

    CustomTreeViewDelegate *delegate = nullptr;

    QHBoxLayout *listTitleHBoxLayout = nullptr;
    QHBoxLayout *tableHBoxLayout = nullptr;
    QToolButton *addMusicButton = nullptr;  //添加歌曲按钮
    QPushButton *playAllButton = nullptr;

    QLabel *listTotalNumLabel = nullptr; // 统计歌单中歌曲总数
    int heightLightIndex = -1;
    QString nowPlayListName;  //后端播放器中正在播放的列表名
    static QString m_lastSavePath;

    QWidget *titleWid = nullptr;

    QWidget *nullPageWidget = nullptr;
    QVBoxLayout *nullPageVLayout = nullptr;
    QHBoxLayout *nullPageHLayout = nullptr;
    QPushButton *n_addMusicButton = nullptr;
    QPushButton *n_addDirMusicButton = nullptr;
    QLabel *nullPageIconLabel = nullptr;
    QLabel *nullPageTextLabel = nullptr;
    bool playFirst = false;       //为true时，表示在stopped状态下，调用了DBus的Play接口

    /**
     * @brief musicWidget 存放音乐列表treeview的布局，并可根据主题设置样式
     *
     * 经验二(Tip2)：当想把一个m_view控件放到一个layout布局中，直接设置layout，可能会出现背景和设计图不符情况。
     * 这时，可以通过构造一个中间widget来设置layout，这个中间widget可以设置设计师要求的样式，
     * 并将这个widget添加到主要的布局mainlayout中。
     */
    QWidget *musicWidget = nullptr;
    QVBoxLayout *m_historyLayout = nullptr;
    QMessageBox *m_warningBox = nullptr;
};

#endif // TableOne_H
