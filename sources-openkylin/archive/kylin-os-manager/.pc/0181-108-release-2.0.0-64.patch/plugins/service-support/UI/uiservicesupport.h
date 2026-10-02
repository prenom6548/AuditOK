#ifndef UISERVICESUPPORT_H
#define UISERVICESUPPORT_H

#include <QWidget>
#include <ktabbar.h>

class QStackedWidget;
class UiServiceSupport : public QWidget
{
    Q_OBJECT
public:
    UiServiceSupport(QWidget *parent = nullptr);

public slots:
    void itemIndexChange(int index);
    void itemIndexChangeFromString(const QString &str);

private:
    void translations();
    QStackedWidget *m_stackWidget = nullptr;
    kdk::KTabBar *m_mainTabBar = nullptr;

signals:
    void indexChanged(QString);
};

#endif // UISERVICESUPPORT_H
