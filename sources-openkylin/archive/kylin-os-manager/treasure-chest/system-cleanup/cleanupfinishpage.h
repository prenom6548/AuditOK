#ifndef CLEANUPFINISHPAGE_H
#define CLEANUPFINISHPAGE_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "component/text_label.h"

namespace systemcleanup
{

class CleanupFinishPage : public QWidget
{
    Q_OBJECT

public:
    CleanupFinishPage(QWidget *parent = nullptr);
    ~CleanupFinishPage();
    void setCleanUpSucc(QString size);
    void setCleanUpFail(QStringList list);

private:
    QLabel *m_labelIcon = nullptr;
    K::TextLabel *m_labelType = nullptr;
    K::TextLabel *m_labelInfo1 = nullptr;
    K::TextLabel *m_labelInfo2 = nullptr;
    K::TextLabel *m_labelInfo3 = nullptr;
    QPushButton *m_btn = nullptr;
    QPushButton *m_btnBack = nullptr;
    QPushButton *m_btnRetry = nullptr;

    QString m_strTypeSucc;
    QString m_strInfo1;
    QString m_strInfo2;
    QString m_strInfo3;
    QString m_strBtn;
    QString m_strTypeFail;
    QString m_strBtnBack;
    QString m_strBtnRetry;

public Q_SLOTS:
    void slotClickBtn();
Q_SIGNALS:
    void signalNextPage();
    void signalBtnBack();
    void signalBtnRetry();
};

}

#endif 