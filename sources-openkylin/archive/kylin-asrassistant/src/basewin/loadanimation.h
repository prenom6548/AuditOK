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

    void start();
    void stop();

protected:
    bool eventFilter(QObject *watch, QEvent *event) override;

private:
    QLabel *m_loadLab = nullptr;
    QTimer *m_timer = nullptr;
    int m_count = 0;
    int m_size = 0;
    bool m_isPlay = true;

Q_SIGNALS:
    void cliecked();

public Q_SLOTS:
    void timeout();
    void onUpdateLevel(double level);
};

#endif // LOADANIMATION_H
