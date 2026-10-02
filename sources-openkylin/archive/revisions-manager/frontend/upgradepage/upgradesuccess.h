#ifndef UPGRADESUCCESS_H
#define UPGRADESUCCESS_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>

class upgradesuccess : public QWidget
{
    Q_OBJECT
public:
    explicit upgradesuccess(QWidget *parent = nullptr);

//    QLabel* title1;
//    QLabel* title2;
//    QWidget* btnwid;
//    QWidget* buttonbox;
//    QPushButton* confirm;
//    QPushButton* later;
//    QHBoxLayout *btnlayout;
//    QVBoxLayout *mainlayout;

        QLabel* title1;
        QLabel* title2;
        QVBoxLayout *mainlayout;

    QTimer *m_timer;

    int value = 10;


    void initui();
    void connection();

signals:
    void rebootsignal();
    void exitappsignal();

public slots:
    void reboot();
    void exitapp();
};

#endif // UPGRADESUCCESS_H
