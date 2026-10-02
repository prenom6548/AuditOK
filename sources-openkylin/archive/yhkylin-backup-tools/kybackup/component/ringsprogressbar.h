#ifndef RINGSPROGRESSBAR_H
#define RINGSPROGRESSBAR_H

#include <QWidget>

class RingsProgressbar : public QWidget
{
    Q_OBJECT
public:
    RingsProgressbar(QWidget *parent = nullptr);
    virtual ~RingsProgressbar();

protected:
    void paintEvent(QPaintEvent *);

public slots:
    void setPersent(int persent);

private:
    int m_persent = 0;
    int m_rotateAngle = 0;
};

#endif // RINGSPROGRESSBAR_H
