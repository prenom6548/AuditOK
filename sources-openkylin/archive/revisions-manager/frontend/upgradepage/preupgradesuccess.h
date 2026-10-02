#ifndef PREUPGRADESUCCESS_H
#define PREUPGRADESUCCESS_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include <kprogresscircle.h>
using namespace kdk;
class preupgradesuccess : public QWidget
{
    Q_OBJECT
public:
    explicit preupgradesuccess(QWidget *parent = nullptr);

    QLabel* title1;
    QLabel* title2;
    QVBoxLayout *mainlayout;
    KProgressCircle* progress;
    QTimer *m_timer;

    int value = 2;


    void initui();
    void connection();
public slots:
    void nextpage();

signals:
    void nextpagesignal();

};

#endif // PREUPGRADESUCCESS_H
