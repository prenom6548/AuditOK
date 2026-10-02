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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QStackedWidget>
#include <QWidget>
#include <QMap>
#include <QMessageBox>

#include <mutex>

#include "themewidget.h"
#include "core/playlist.h"
#include "global/globalsignal.h"
#include "kpushbutton.h"

#define FONT_COLOR_SELECTED "rgb(55,144,250)"
#define AnmationDuration 300

class TextLabel;
class PlayItem;
class PlayList;
class PlayListItemMenu;
class ImageLabel;
class ListLoopMenu;

class QLabel;
class QListWidget;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QListWidgetItem;
class QVariantAnimation;
class QPropertyAnimation;

/// 列表模式-预览模式
enum ListViewMode{LIST, VIEW};

class MarkListItem : public ThemeWidget
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    explicit MarkListItem(QWidget *parent = nullptr);
    ~MarkListItem();

    int getPos(){return m_markPos;}
    void setData(QString path, int duration, int pos, QString desc, QString preview = "");
    void setMode(ListViewMode mode);
    void setEnter();
    void setLeave();
    QString getFilePath() {return m_tooltip;}
    void setWidgetFont(QFont f);

public slots:
    void setNotExit(QString);
    void setFontColor(QColor);
    void setBlackTheme();
    void setDefaultTheme();

signals:
    void sigPlayAndSeek(QString, int);
    void sigRightBtnPressed(int);
    void sigDeleteMark(QString, int);

private:
    void initStyle();
    void updateNameShow();

private:
    int m_markPos;
    bool m_needSeekToPos,   // 双击mark启动视频后，根据这个值判断是否需要seek到指定位置（只有切换了播放文件才需要这么操作）
         m_fileExit;
    QString m_currentPlayingFile;
    QString m_fileName;
    QString m_tooltip;
    QString m_describe;
    QSize m_noPreviewSize, m_previewSize;
    QPixmap *m_pixmapPreview;
    ImageLabel  *m_labPreview;
    QLabel  *m_labFilePath,
            *m_labDuration,
            *m_labMarkPos;
    QWidget *m_dp;

    QVBoxLayout *m_vlay;
    QHBoxLayout *m_hlay;

    QPushButton *m_btnDelete;

    ListViewMode m_mode;

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
    bool event(QEvent *e) override;
};


/// 播放列表条目
class PlayListItem : public ThemeWidget
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    explicit PlayListItem(QWidget *parent = nullptr);
    ~PlayListItem();

    QString getPath() {return m_tip;}
    QString getName() {return m_showName;}
    void setSelected(bool s);
    void setData(QString file, int duration, int lastTime, QString viewPath = "");
    void setDuration(int duration);
    void setEnter();
    void setLeave();
    void toBig();
    void toSmall();
    void reloadView();
    void setMode(ListViewMode mode);
    bool getExist(){return m_isExist;}

    void setBlackTheme();
    void setDefaultTheme();
    void updateHightLightColor();
    void setWidgetFont(const QFont &f);

    // Expose delete button for accessibility helpers
    QPushButton* deleteButton() const { return m_btnDelete; }

signals:
    void sigValChange(int);
    void sigRightBPressed(QString);
    void sigNotExists(QString);
    void sigExistsStateChange(QString, bool);

public slots:
    void slotNotExit(QString);

private:
    QLabel  *m_labSpacing,
            *m_labDuration;
    TextLabel   *m_labName;
    QPixmap *m_pixmapPreview;
    ImageLabel  *m_labPreview;
    QPushButton *m_btnDelete;
    kdk::KPushButton *m_labIcon;

    QVariantAnimation   *m_anmToBig,
                        *m_anmToSmall;

    QString m_tip,
            m_showName,
            m_viewPath;
    bool    m_isSelected,
            m_isExist,
            m_isMark;
    ListViewMode m_mode;

    void initStyle();
    void initDeleteButton();
    void initAnimation();
    void updateNameShow();
    void updateText();

private slots:
    void setFontColor(QColor);
    void deleteItem();

protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    bool event(QEvent *e) override;
};

class PlayListWidget : public ThemeWidget
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    explicit PlayListWidget(QWidget *parent = nullptr);
    ~PlayListWidget();

    bool isShow(){return m_isShow;}
    PlayList *getPlayList(){return m_playList;}

    void setBlackTheme();
    void setDefaultTheme();

public slots:
    void showItemMenu(QString file);
    void addItem(PlayItem item);
    void deleteItem(QString file);
    void clearList();

    void updateMarks(QVector<MarkItem> markvec);
    void deleteMark(QString filepath, int pos);
    void clearMark();

    void reloadView(QString file);
    void resetDuration(QString file, int duration);
    void slotShow();
    void slotHide();

    // 展开按钮更新
    void setShowButton(bool show);
    void updateShowIcon();
    void updateHideIcon();

    void delAvailableIndex(QString file);
    void changeListViewMode();
    void setPlayIndex(int index);
    void selectNone();

    void gSettingChanged(QString key);

signals:
    void sigAddItem(QString, int, int, QString, bool);
    void sigMove(int);

private:
    QString m_icoDir;

    QWidget *m_homePic;
    QString m_theme;
    QString m_currentPlayingFile;
    QString m_selectedFile;
    PlayList *m_playList;
    QListWidget *m_playListWidget,
                *m_markListWidget;

    PlayOrder m_playOrder;          // 播放顺序
    PlayListItemMenu *m_itemMenu;   // 右击列表选项菜单
    QMap<QString, QListWidgetItem*> m_itemMap;  // 列表item map，key:绝对路径
    QMap<QString, QMap<int, QListWidgetItem*>> m_markMap;

    std::mutex m_mux;
    std::mutex m_itemAddMux;
    bool m_isShow;
    bool m_isMouseEnter;
    bool m_canHideLeftButton;
    ListViewMode m_mode;
    QPropertyAnimation *m_showAnm;
    QPropertyAnimation *m_hideAnm;

    QWidget *m_left;
    QPushButton *m_leftButton;

    QWidget *m_right;
    QWidget *m_rightTop;
    QWidget *m_rightTitle;
    QWidget *m_lineDock;
    QWidget *m_line;
    QLabel *m_homeText;
    QListWidget *m_titleList;
    QStackedWidget *m_listStack;
    QPushButton *m_addButton;
    QPushButton *m_deleteButton;
    QPushButton *m_orderButton;
    QPushButton *m_viewButton;

    QMessageBox *m_warningBox { nullptr };

    void initLayout();
    void initMenu();
//    void initStackedWidget();
    void initPlayList();
    void initGlobalSig();
    void initAnimation();
    void initConnection();
    void initDefaultWidget();

    void updateOrderIcon();
    void updateViewIcon();
    void updateAddIcon();
    void updateDeleteIcon();

    void resetText(QListWidgetItem* item);

    void updateHightLightColor();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void moveEvent(QMoveEvent *event) override;
};

#endif // PLAYLISTWIDGET_H
