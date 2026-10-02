#ifndef FILEMANAGEWIN_H
#define FILEMANAGEWIN_H

#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <kbreadcrumb.h>
#include <ktoolbutton.h>
#include "ksearchlineedit.h"
#include "loadanimation.h"
#include "clicklabel.h"

class FileManageWin : public QWidget
{
    Q_OBJECT
public:
    FileManageWin(QWidget *parent = nullptr);

    enum TabType {
        Default = 0,        // 默认
        AndroidHomePage,    // 安卓设备首页
        AppHomePage,        // QQ微信首页
        SearchResultsPage,  // 搜索结果
        Load,               // 加载页面
        Edit,               // 编辑模式
    };
    Q_ENUM(TabType)

    enum BtnType {
        GoBacktBtn = 0,   // 后退按钮
        GoForwardBtn,     // 前进按钮
        ReturnBtn,        // 返回按钮
        EditBtn,          // 编辑按钮
        ListModeBtn,      // 列表模式切换
        IconModeBtn,      // 图标模式切换
        RefreshBtn,       // 刷新按钮
        SelectBtn,        // 全选按钮
        CancelSelectBtn,  // 取消全选
        EditFinishBtn,    // 编辑完成按钮
    };

    enum CrumbTag {
        FileList = 0,  // 手机文件列表
        Picture,       // 图片
        Video,         // 视频
        Music,         // 音乐
        Doc,           // 文档
        WeChat,        // 微信
        QQ,            // QQ
        Storage,       // 手机存储
    };

    void setEditType(bool isSelectAll);
    void setWidget(QWidget *win, FileManageWin::TabType type);
    void pushCrumb(QString str);
    void clearCrumb();
    void load();
    void setGoBackEnabled(bool isEnabled);
    void setGoForwardEnabled(bool isEnabled);
    QStringList getPathList() const;
    QString crumbText(CrumbTag type);
    void changeFontSize(double fontSize);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void changeTabMode(FileManageWin::TabType type);
    void hideSearchLine();
    void generateDisplayTextList();
    void autoResetCrumb();
    void resetCrumbMaxWidth();

Q_SIGNALS:
    void sigCrumbIndexChange(QString, QStringList &);
    void sigBtnCliked(FileManageWin::BtnType);
    void sigSearchTextChanged(QString);

public Q_SLOTS:
    void slotCrumbIndexChange(int index);
    void slotKdkBtnClicked();
    void slotBtnClicked();
    void slotAnimationValueChanged(const QVariant &value);
    void slotSearchTextChanged();

private:
    QString m_fileListTag = tr("List of Files");
    QString m_pictureTag = tr("Picture");
    QString m_videoTag = tr("Video");
    QString m_musicTag = tr("Music");
    QString m_docTag = tr("Doc");
    QString m_qqTag = tr("QQ");
    QString m_wechatTag = tr("WeChat");
    QString m_storageTag = tr("Storage");

    enum PageIndex {
        LoadPage = 0,                                     // 加载页面
        InfoPage,                                         // 文件信息页面
    };
    kdk::KToolButton *m_retreatBtn = nullptr;             // 后退按钮
    kdk::KToolButton *m_forwardBtn = nullptr;             // 前进按钮
    QLabel *m_statusLab = nullptr;                        // 当前状态描述， 选择文件 | 搜索文件
    kdk::KBreadCrumb *m_crumb = nullptr;                  // 面包屑
    kdk::KToolButton *m_searchBtn = nullptr;              // 搜索按钮
    kdk::KSearchLineEdit *m_searchLine = nullptr;         // 搜索框
    kdk::KToolButton *m_editBtn = nullptr;                // 选择文件按钮
    ClickLabel *m_selectBtn = nullptr;                    // 全选按钮
    QFrame *m_verticalLine = nullptr;                     // 分隔线
    kdk::KToolButton *m_modeBtn = nullptr;                // 模式切换
    kdk::KToolButton *m_refreshBtn = nullptr;             // 刷新按钮
    ClickLabel *m_finishBtn = nullptr;                    // 编辑完成
    QPropertyAnimation *m_searchLineAnimation = nullptr;  // 搜索栏展示动画
    LoadAnimation *m_loadAnimation = nullptr;             // 加载动画
    QStackedWidget *m_stackedWin = nullptr;               // 切换窗口
    QLabel *m_lab1 = nullptr;                             // 间距1
    QLabel *m_lab2 = nullptr;                             // 间距2
    QLabel *m_lab3 = nullptr;                             // 间距3
    QLabel *m_lab4 = nullptr;                             // 间距4
    QLabel *m_lab5 = nullptr;                             // 间距5
    QLabel *m_lab6 = nullptr;                             // 间距6

    FileManageWin::TabType m_type = FileManageWin::TabType::Default;  // 当前显示模式
    bool m_isSearchLineHiddenIn = false;
    bool m_isListMode = false;
    bool m_isSearch = false;
    QStringList m_crumbList;
    QStringList m_textList;
    QMap<QString, QString> m_textMap;
    double m_fontSize = 10.0;
    int m_crumbMaxWidth = 640;
    bool m_isAutoReset = false;
    int m_offset = 0;
};

#endif  // FILEMANAGEWIN_H
