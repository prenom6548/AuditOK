#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "kpushbutton.h"
#include "kborderbutton.h"
#include "../component/text_label.h"
class StatusWidget : public QWidget
{
    Q_OBJECT
public:
    StatusWidget(QWidget *parent = nullptr);
    ~StatusWidget();
    void setHighColorStatus();
    void setNomarlStatus();
    void setStatusText(QString str1, QString str2);

private:
    void initUI();
    K::TextLabel *m_label = nullptr;
    K::TextLabel *m_labelInfo = nullptr;
    kdk::KPushButton *m_btnHighColor = nullptr;
    kdk::KBorderButton *m_btnNormal = nullptr;
    // QPushButton *m_btnNormal = nullptr;

    QVBoxLayout *m_vlayout = nullptr;
    

};
#endif