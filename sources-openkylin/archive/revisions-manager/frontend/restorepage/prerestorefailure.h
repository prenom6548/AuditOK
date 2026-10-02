#ifndef PRERESTOREFAIL_H
#define PRERESTOREFAIL_H

#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QTextEdit>

class prerestorefailure : public QWidget
{
    Q_OBJECT
public:
    explicit prerestorefailure(QWidget *parent = nullptr);

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

#endif // PRERESTOREFAIL_H
