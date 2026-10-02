#ifndef UIMAINPAGE_H
#define UIMAINPAGE_H

#include <kpushbutton.h>
#include "plugin.h"
#include "kom-label.h"

class QLabel;
class UiServiceSupport;

class UIMainPageItem : public kdk::KPushButton
{
    Q_OBJECT
public:
    UIMainPageItem(QWidget *parent = nullptr);
    void setItemText(const QString &str);
    void setIconPath(const QString &str);

private:
    QPushButton *m_Icon = nullptr;
    kom::KomLabel *m_Text = nullptr;

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

public slots:
    void onItemClicked(const QString &str);
    static void komCallBack(ClassType classType, QVariant var);

private slots:
    void onFontSizeChange();

private:
    UIMainPage(QWidget *parent = nullptr);
    void showMainPage();
    void initMainPageUI();
    void setItemIcons();
    void onthemeChange();
    static UIMainPage *m_instance;
    UiServiceSupport *m_serviceSupportUI = nullptr;
    QWidget *m_mainPagePri = nullptr;
    UIMainPageItem *m_feedback = nullptr;
    UIMainPageItem *m_self = nullptr;
    UIMainPageItem *m_history = nullptr;
    QLabel *m_title = nullptr;

signals:
    void itemIndexChangeFromString(const QString &str);
};

#endif // UIMAINPAGE_H
