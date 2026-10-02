#ifndef UIMAINPAGE_H
#define UIMAINPAGE_H

#include <kpushbutton.h>
#include <ksearchlineedit.h>
#include <kborderlessbutton.h>
#include <QScrollArea>
#include "plugin.h"
#include "kom-label.h"
#include "flowlayout.h"
#include "frameline.h"

class QLabel;
class UiServiceSupport;
class UIMainIssueTypeItem : public QWidget
{
    Q_OBJECT
public:
    UIMainIssueTypeItem(QWidget *parent = nullptr);
    void setItemText(const QString &str);
    void setItemIssueList(const QStringList &list);

private:
    QLabel *m_textLab = nullptr;
    kdk::KBorderlessButton *m_iconBtn = nullptr;
    QWidget *m_widget = nullptr;
    QVBoxLayout *m_vl = nullptr;
    FlowLayout *m_flowLayou = nullptr;
    QString m_issueTypeStr = "";

protected:
    void paintEvent(QPaintEvent *event) override;
};

class UIMainPageItem : public kdk::KPushButton
{
    Q_OBJECT
public:
    UIMainPageItem(QWidget *parent = nullptr);
    void setItemText(const QString &str);
    void setItemText2(const QString &str);
    void setIconPath(const QString &str);

private:
    QPushButton *m_Icon = nullptr;
    kom::KomLabel *m_Text = nullptr;
    kom::KomLabel *m_Text2 = nullptr;

private slots:
    void onClicked();

signals:
    void onUIMainPageItemClick(const QString &str);
};

class UIMainPage : public QWidget
{
    Q_OBJECT
public:
    static UIMainPage *getInstance();
    void showMainPage();

public slots:
    void onItemClicked(const QString &str);
    static void komCallBack(ClassType classType, QVariant var);
    void onBackButtonClick();

private slots:
    void onFontSizeChange();

private:
    UIMainPage(QWidget *parent = nullptr);
    void initMainPageUI();
    void setItemIcons();
    void onthemeChange();
    static UIMainPage *m_instance;
    UiServiceSupport *m_serviceSupportUI = nullptr;
    QWidget *m_mainPagePri = nullptr;
    UIMainPageItem *m_feedback = nullptr;
    UIMainPageItem *m_self = nullptr;
    UIMainPageItem *m_history = nullptr;
    UIMainPageItem *m_faq = nullptr;
    QLabel *m_title = nullptr;
    kdk::KSearchLineEdit *m_searchLineEdit = nullptr;
    QWidget *m_issueWidget = nullptr;
    UIMainIssueTypeItem *m_appUseIssue = nullptr;
    UIMainIssueTypeItem *m_netConnIssue = nullptr;
    UIMainIssueTypeItem *m_hardwareIssue = nullptr;
    UIMainIssueTypeItem *m_SystemIssue = nullptr;
    FlowLayout *m_flowLayout = nullptr;
    QLabel *m_labelTip = nullptr;
    FrameLine *m_framline = nullptr;
    QScrollArea *m_scrollArea = nullptr;

signals:
    void itemIndexChangeFromString(const QString &str);
    void sigSearchIssue(const QString &str); // 搜索框回车，发出搜索内容信号
    void sigIssueBtnClick(const QString &str);            // 模块中的小按钮点击发出的信号
    void sigUIMainIssueTypeItemClick(const QString &str); // 4个模块点击发出的信号
    
};

#endif // UIMAINPAGE_H
