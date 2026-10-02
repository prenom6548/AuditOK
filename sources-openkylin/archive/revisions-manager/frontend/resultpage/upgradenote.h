#ifndef UPGRADENOTE_H
#define UPGRADENOTE_H

#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QTextEdit>

class UpgradeNote : public QWidget
{
    Q_OBJECT
public:
    explicit UpgradeNote(QWidget *parent = nullptr);

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

    QWidget* midwidget;
    QHBoxLayout* midlayout;
    QLabel* midlabel;
    QPushButton* midbtn;
    QLabel* suggestionlabel;

    void initui();
    void dbusconnection();

    void showdeblist();

public slots:
    void exitapp();

signals:
    void exitappsignal();
    void showdeblistsignal();

};

#endif // UPGRADENOTE_H
