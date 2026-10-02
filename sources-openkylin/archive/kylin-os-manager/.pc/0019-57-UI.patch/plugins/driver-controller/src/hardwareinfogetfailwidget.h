#ifndef HARDWAREINFOGETFAILWIDGET_H
#define HARDWAREINFOGETFAILWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <kysdk/applications/gsettingmonitor.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class HardwareInfoGetFailWidget : public QWidget {
Q_OBJECT
public:
    HardwareInfoGetFailWidget(QWidget *parent = nullptr);

    ~HardwareInfoGetFailWidget();

private:
    QLabel *m_failIcon = nullptr;
    QLabel *m_failLabel = nullptr;
    QPushButton *m_retryBtn = nullptr;
    QHBoxLayout *m_iconLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    void initConnect();

    void initGUi();

    void themeSetting();
};


#endif //HARDWAREINFOGETFAILWIDGET_H
