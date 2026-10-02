/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef VIEWFILTERFILES_H
#define VIEWFILTERFILES_H

#include "src/model/filefiltermodel.h"
#include "src/view/windowtitle.h"

#include "kwidget.h"
#include "kdialog.h"
#include "kpushbutton.h"

#include <QWidget>
#include <QObject>
#include <QDialog>
#include <QMutex>
#include <QPoint>
#include <QHeaderView>

class QLabel;
class QComboBox;
class QVBoxLayout;
class QHBoxLayout;
class QSpacerItem;
class QTreeWidget;
class QTreeView;
class QTreeWidgetItem;

using namespace kdk;

class UkuiWindowHelper;
class FileFilterRules;
class ViewFilterFiles : public QDialog
{
    Q_OBJECT
Q_SIGNALS:
    void delNode(int);
    void allFilesRestored(QList<void *> hiddens, QList<void *> brokens, QList<void *> replaces);
public:
    static ViewFilterFiles *instance(QWidget *parent = nullptr);
public:
    void updatePosition(QPoint);
public:
    ViewFilterFiles(const ViewFilterFiles*)=delete;
    ViewFilterFiles *operator =(const ViewFilterFiles *)=delete;
public:
    bool filtered();
    int currentIndex();
    QList<void *> recoveryHiddenData(int, int);
    QList<void *> recoveryBrokenData(int, int);
    QList<void *> recoveryRepleaceData(int, int);
public slots:
    void show();
    void newFilterRoot(void *);
    void filterHidden(int, void *);
    void filterBroken(int, void *);
    void repleace(int, void *);
    void updateIndex(int);
    void deleteNode(int);
private slots:
    void slotRestoreFiles();
public slots:
    void slotsChangeComboDiskIndex(void*);

    void slotChangeUIFontSize(int fontSize);

    void slotShowFilterRules();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
    explicit ViewFilterFiles(QWidget *parent = nullptr);
    ~ViewFilterFiles();

    void initDialog();
    void initMember();
    void initRootLayout();
    void initTitleLayout();
    void initSignalConnect();

    /**
     * @brief 设置TreeWidget的样式
     */
    void initTreeWidget();


private:
    static ViewFilterFiles *m_oInstance;
    static QMutex           m_oMutex;

    UkuiWindowHelper       *m_windowHelper;
    QPoint                  m_oPosition;    /* 调整界面位置居中 */
    QComboBox              *mComboBoxDisk;  /* 界面调整后沿用之前逻辑而特意保留但不在界面显示的下拉列表框 */
    QList<void*>            mCDROMList;

    WindowTitle            *mWindowTitle;
    /* 以下是界面控件及其布局 */
    QWidget                 *mMainWdiget;
    QVBoxLayout             *mMainLayout;
    QHBoxLayout             *mTitleLayout;
    QHBoxLayout             *mButtonLayout;
    QHBoxLayout             *mFileTreeLayout;

    //TitleLayout
    QLabel                  *mTitleLabel;

    //ButtonLayout
    int                      btn1Width;
    int                      btn2Width;
    KPushButton             *mFilterOptionBtn;
    QSpacerItem             *mButtonSpacer1;
    KPushButton             *mRecoverAllBtn;
    QSpacerItem             *mButtonSpacer2;

    //FileTreeLayout
    QTreeView               *mTreeView;
    QList<QTreeWidgetItem*>  mTreeWidgetItemList;
    FileFilterModel        *m_oModel;
    FileFilterRules        *m_fileFilterRules;
};
#endif // VIEWFILTERFILES_H
