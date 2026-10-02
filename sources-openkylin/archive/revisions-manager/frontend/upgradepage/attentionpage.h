#ifndef ATTENTIONPAGE_H
#define ATTENTIONPAGE_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFile>

class attentionpage: public QWidget
{
    Q_OBJECT
public:
    explicit attentionpage(QWidget *parent = nullptr);

    QLabel* title;
    QTextEdit *content;
    QPushButton *btn;

    QHBoxLayout *titlelayout;
    QVBoxLayout *main;

    void initui();
    void dbusconnection();

private slots:
    void returntofirst(bool);

signals:
    void returnsignal();
};

#endif // ATTENTIONPAGE_H
