#ifndef UISERVICESUPPORT_H
#define UISERVICESUPPORT_H

#include <QWidget>
#include <ktabbar.h>
#include "service_support_base.h"

class QStackedWidget;
class UiServiceSupport : public QWidget
{
    Q_OBJECT
public:
    UiServiceSupport(QWidget *parent = nullptr);
    ServiceSupportBase *CurrentDisplayInstance();

public slots:
    void itemIndexChange(int index);
    void itemIndexChangeFromString(const QString &str);

private:
    void translations();
    QStackedWidget *m_stackWidget = nullptr;
    kdk::KTabBar *m_mainTabBar = nullptr;

signals:
    void indexChanged(QString);
    void sigSearchIssue(const QString &str); // 搜索框回车，发出搜索内容信号
    void sigUIMainIssueTypeItemClick(const QString &str); // 4个模块点击发出的信号
};

#endif // UISERVICESUPPORT_H
