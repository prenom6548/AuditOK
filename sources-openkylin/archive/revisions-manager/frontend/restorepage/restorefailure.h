#ifndef RESTOREFAILURE_H
#define RESTOREFAILURE_H

#include <QDebug>
#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIcon>

class restorefailure : public QWidget
{
    Q_OBJECT
public:
    explicit restorefailure(QWidget *parent = nullptr);

    QLabel* iconlabel;
    QIcon* icon;
    QLabel* title;
    QLabel* description;
    QLabel* suggestion;

    QPushButton* confirm;

    QVBoxLayout *mainlayout;

    QWidget* topwid;
    QWidget* midwid;

    QVBoxLayout* toplayout;
    QVBoxLayout* midlayout;


    void initui();
    void dbusconnection();
    void exitapp();

signals:
    void exitappsignal();
};

#endif // RESTOREFAILURE_H
