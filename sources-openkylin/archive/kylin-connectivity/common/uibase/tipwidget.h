//
// Created by sqp on 2021/11/17.
//

#ifndef KYLIN_CONNECTIVITY_TIPWIDGET_H
#define KYLIN_CONNECTIVITY_TIPWIDGET_H


#include <QLabel>

class TipWidget : public QLabel
{
    Q_OBJECT

public:
    TipWidget();
    ~TipWidget();
    void SetMesseage(const QString &strMessage, const QPoint *pPoint = nullptr, const int time = 10);

private:
    void OnTimer();

private:
    QWidget *mpParent;
    QTimer *mpTimer;
    int mnTransparent;
};


#endif // KYLIN_CONNECTIVITY_TIPWIDGET_H
