#ifndef UPGRADEFAILURE_H
#define UPGRADEFAILURE_H

#include <QDebug>
#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QTextEdit>

class upgradefailure: public QWidget
{
    Q_OBJECT
public:
    explicit upgradefailure(QWidget *parent = nullptr);

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

#endif // UPGRADEFAILURE_H
