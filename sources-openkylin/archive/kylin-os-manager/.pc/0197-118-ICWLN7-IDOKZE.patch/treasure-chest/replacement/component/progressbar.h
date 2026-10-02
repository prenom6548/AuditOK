#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>
#include <QLayout>
#include "kprogressbar.h"
#include "text_label.h"

class ProgressBar : public QWidget
{
    Q_OBJECT
public:
    ProgressBar(QWidget *parent);
    ~ProgressBar();

    void setFileName(QString str);
    void setBarValue(qint64 num);
    void setSpeedValue(QString str);
    void SetBarRange(qint64 min, qint64 max);

private:
    void initUI();
    kdk::KProgressBar *m_bar = nullptr;
    K::TextLabel *m_labelFileName = nullptr;
    K::TextLabel *m_labelPerce = nullptr;
    K::TextLabel *m_labelSpeed = nullptr;

    QVBoxLayout *m_layoutV = nullptr;
    QHBoxLayout *m_layoutH = nullptr;

public slots:
};
#endif
