#ifndef PREUPGRADEFAILURE_H
#define PREUPGRADEFAILURE_H

#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QTextEdit>
#include <QApplication>

//#include "upgradepage/upgradepage2.h"
class preupgradefailure : public QWidget
{
    Q_OBJECT
public:
    explicit preupgradefailure(QWidget *parent = nullptr);

    QLabel* iconlabel;
    QIcon* icon;
    QLabel* warning;
    QLabel* detail;
    QLabel* tips;
    QTextEdit* content;
    QPushButton* btn;

    QVBoxLayout* mainlayout;
    QVBoxLayout* iconlayout;
    QVBoxLayout* warninglayout;
    QVBoxLayout* titlelayout;
    QVBoxLayout* detaillayout;
    QVBoxLayout* btnlayoutlayout;

    QWidget* btnwidget;

    void initui();
    void dbusconnection();

public slots:
    void exitapp();

signals:
    void exitappsignal();
};

#endif // PREUPGRADEFAILURE_H
