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


#ifndef BURNDATA_H
#define BURNDATA_H

#include "src/model/logicburndata.h"
#include "src/view/multiselectionfiledialog.h"
#include "src/process/processopenfile.h"
#include "inputchange.h"
#include "tipschange.h"

#include "klineframe.h"
#include "kbreadcrumb.h"
#include "ktoolbutton.h"

#include <QWidget>
#include <QMutex>
#include <QAction>
#include <QScopedPointer>
#include <QStyledItemDelegate>

class MyMenu;
class LogicBurnData;
namespace Ui {
class BurnData;
}

class CustomSelectionLineEdit : public QLineEdit{
    friend class BurnDataTest;
    Q_OBJECT
public:
    CustomSelectionLineEdit(QWidget* parent = nullptr) : QLineEdit(parent){
        m_basename_pos = -1;
    }
    ~CustomSelectionLineEdit(){}

    void setBasename_pos(int basename_pos){
       m_basename_pos = basename_pos;
    }

private:
    int m_basename_pos;
protected:
    void focusInEvent(QFocusEvent* event){

        //set init selection
        if(m_basename_pos > 0){
            this->deselect();
            this->setSelection(0,m_basename_pos);
        }

        QLineEdit::focusInEvent(event);
    }
};

class CustomDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    CustomDelegate(QObject* parent = nullptr);
    ~CustomDelegate();
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

class MyTreeView : public QTreeView
{
    friend class BurnDataTest;
    Q_OBJECT
public:
    explicit MyTreeView(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void emitScrollStatusSignal(QScrollBar *scrollBar, bool isVisible);

Q_SIGNALS:
    void verticalScrollBarVisibleChanged(bool visible);
    void horizontalScrollBarVisibleChanged(bool visible);
};

class BurnData : public QWidget
{
    friend class BurnDataTest;
    friend class MainWindowTest;
    Q_OBJECT
public:
    static BurnData *instance();
public:
    BurnData(const BurnData*)=delete;
    BurnData *operator =(const BurnData*)=delete;
public:
    void refresh();
    static bool m_bDirReNameing; // 是否在重命名中
public:
    void clearHistory() {
        m_back_list.clear();
        m_forward_list.clear();
    }

    bool canGoBack();
    bool canGoForward();
    void delModelIndex(QModelIndex);

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
private:
    explicit BurnData(QWidget *parent = nullptr);
    ~BurnData();
private:
    void initAction();
    void linkEntry(QModelIndex);
    void dataViewChanged(QModelIndex& idx);
    void autoResetCrumb();
    void updateDisplayList(int count);
    void clearCrumb();
    void resetText(QPushButton* item);
    void setAccessibleName();
    void resetTreeViewSize(bool);
    bool m_isAutoReset = false;
    int m_offsetRemovedTabCount = 0;

public slots:
    void slotChangeUIFontSize(int fontSize);
    void slotChangeUIFontname(QString fontName);
    void slotStopLoadingData();

    void slotsFilterHidden(bool);
    void slotsFilterBroken(bool);
    void slotsRepleaceSymbolLinks(bool);

private slots:
    void slotSetDefaultAddWindowVisible(bool isVisible);
    void slotsEntry(QModelIndex);
    void slotsSize(unsigned long);
    void slotsSelectionChanged();
    void slotsAddFiles(bool);
    void slotsDeleteFiles(bool);
    void slotsCleanFiles(bool);
    void slotsRemoveAllFiles();
    void slotsNewFolder(bool);
    void slotsParent();
    void slotsOpen();
    void slotsOpenLocal();
    void slotsRename();
    void slotsProperties();
    void slotsTreeMenuByRightKey(const QPoint &pos);
    void slotsTreeMenuByShortcutKey();
    void slotsDeleteFilesByKeyPress(bool);
    void slotsVerticalScrollVisible(bool);

    void goBack();
    void goForward();
    void directoryChangeSlot(int);
    void slotsBurnerIndexChanged();
    void slotsShowAddFilesDlg(bool enable);

private:
    static BurnData          *m_oInstance;
    static QMutex             m_oMutex;
    bool                      m_bIsWayland;
    QLabel *title;
    kdk::KHLineFrame *bottomLine;
private:
    LogicBurnData            *m_oModel;
    MyMenu                   *m_oMenu;
    QAction                  *m_oOpen;
    QAction                  *m_oOpenLocal;
    QAction                  *m_oRename;
    QAction                  *m_oDelete;
    QAction                  *m_oNewFolder;
    QAction                  *m_oProperties;
    ProcessOpenFile          *m_oOpenFile;
    QModelIndex               m_oAddIndex;

    kdk::KBreadCrumb         *mDirectoryCrumb;
    kdk::KToolButton *m_go_back;
    kdk::KToolButton *m_go_forward;
    QMap<int, QModelIndex> *mDirectoryMap;   //目录映射，便于快速切换目录与QTreeView显示内容
                                                 //TODO: const QModelIndex传递为什么有问题?
    QModelIndexList m_back_list;
    QModelIndexList m_forward_list;
    QModelIndex               m_rootIndex;
    QStringList m_textList;
    QString last_path = QDir::homePath();
private:
    Ui::BurnData *ui;
Q_SIGNALS:
    void changeCurrentIndex(int);
    void burnDataToDiscEnabled(bool);
    void updateDisk(int, void *);
    void newNode(void *);
    void delData(int);
    void viewFileterFile(bool);
};

#endif // BURNDATA_H
