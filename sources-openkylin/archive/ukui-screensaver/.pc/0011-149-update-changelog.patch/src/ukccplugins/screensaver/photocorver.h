#ifndef PHOTOCORVER_H
#define PHOTOCORVER_H

#include <QLabel>
#include <QWidget>
#include "../../screensaver/masklable.h"

class PhotoCorver : public QLabel
{
    Q_OBJECT
public:
    explicit PhotoCorver(QWidget *parent = nullptr);
    void setBackground(QPixmap background);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
Q_SIGNALS:
    void photoCorverClicked();

private:
    QPixmap m_background;
    bool m_bHoverStatus = false;
    bool m_bSelectStatus = false;
    MaskLable *m_maskLabel = nullptr;
};

#endif // PHOTOCORVER_H
