#ifndef LOADANIMATION_H
#define LOADANIMATION_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

class LoadAnimation : public QWidget
{
    Q_OBJECT
public:
    explicit LoadAnimation(QWidget *parent = nullptr);

    void setHighLight();

private:
    QLabel *m_loadLab = nullptr;
    QTimer *m_timer = nullptr;
    int m_count = 1;

Q_SIGNALS:

public Q_SLOTS:
    void timeout();
};

#endif // LOADANIMATION_H
