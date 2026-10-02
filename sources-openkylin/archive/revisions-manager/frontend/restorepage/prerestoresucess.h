#ifndef PRERESTORESUCESS_H
#define PRERESTORESUCESS_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>

class prerestoresucess : public QWidget
{
    Q_OBJECT
public:
    explicit prerestoresucess(QWidget *parent = nullptr);

    QLabel* title1;
    QLabel* title2;
    QVBoxLayout *mainlayout;

    QTimer *m_timer;

    int value = 2;


    void initui();
    void connection();
public slots:
    void nextpage();

signals:
    void nextpagesignal();

};

#endif // PRERESTORESUCESS_H
