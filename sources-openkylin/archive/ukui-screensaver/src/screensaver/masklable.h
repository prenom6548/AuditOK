#ifndef MASKLABLE_H
#define MASKLABLE_H

#include <QWidget>
#include <QLabel>

class MaskLable : public QLabel
{
    Q_OBJECT
public:
    explicit MaskLable(QWidget *parent = nullptr, bool isControlSaver = false);
protected:
    void paintEvent(QPaintEvent *event);

private:
    bool m_bControlSaver = false;
};

#endif // MASKLABLE_H
