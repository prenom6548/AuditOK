#ifndef INPUTCONVERT_H
#define INPUTCONVERT_H

#include <QTimer>
#include "inputconvertbase.h"

class InputConvertNormal : public InputConvertBase
{
    Q_OBJECT
public:
    InputConvertNormal(Controller *controller);

    virtual ~InputConvertNormal();

    virtual void mouseEvent(const QMouseEvent *from, const QSize &frameSize, const QSize &showSize) override;

    virtual void wheelEvent(const QWheelEvent *from, const QSize &frameSize, const QSize &showSize) override;

    virtual void keyEvent(const QKeyEvent *from, const QSize &frameSize, const QSize &showSize) override;

public Q_SLOTS:
    void runAction();

private:
    AndroidMotioneventButtons convertMouseButtons(Qt::MouseButtons buttonState);

    AndroidKeycode convertKeyCode(int key, Qt::KeyboardModifiers modifiers);

    AndroidMetastate convertMetastate(Qt::KeyboardModifiers modifiers);

    void onScroll();

private:
    QTimer *m_scrollTimer = nullptr;
    int m_scrollCount = 0;
    long long m_scrollTime = 0;
    qint32 m_hScroll = 0;
    qint32 m_vScroll = 0;
    QPointF m_scrollPoint = QPoint(0,0);
    QSize m_frameSize = QSize(0,0);
};

#endif // INPUTCONVERT_H
