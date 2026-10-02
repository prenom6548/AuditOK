#ifndef RESTOREATTENTIONPAGE_H
#define RESTOREATTENTIONPAGE_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

class restorenotification : public QWidget
{
    Q_OBJECT
public:
    explicit restorenotification(QWidget *parent = nullptr);

    QLabel* title;
    QTextEdit *content;
    QPushButton *nextbtn;
    QPushButton *backbtn;
    QCheckBox *check;


    QHBoxLayout *titlelayout;
    QVBoxLayout *main;
    QHBoxLayout *btnlayout;

    void initui();
    void dbusconnection();

//    void statechange();
    void gotofirstpage();
    void nextpage();
public slots:
    void statechange(int num);

signals:
    void gotofistpagesignal();
    void nextpagesignal();
};

#endif // RESTOREATTENTIONPAGE_H
