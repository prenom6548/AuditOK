#ifndef DRIVERGETFAILWIDGET_H
#define DRIVERGETFAILWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <kysdk/applications/gsettingmonitor.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class DriverGetFailWidget : public QWidget {
Q_OBJECT
public:
    DriverGetFailWidget(QWidget *parent = nullptr);

    ~DriverGetFailWidget();

private:
    QLabel *m_failIcon = nullptr;
    QLabel *m_failLabel = nullptr;
    QPushButton *m_retryBtn = nullptr;
    QHBoxLayout *m_iconLayout= nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    void initConnect();

    void initGUi();

    void themeSetting();

};


#endif //DRIVERGETFAILWIDGET_H
