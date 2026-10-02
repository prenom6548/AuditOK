#include "frame.h"

Frame::FrameCallback *Frame::m_frameCallback = nullptr;

void Frame::setFrameCallback(FrameCallback *frameCallback) {
    m_frameCallback = frameCallback;
}

QRect Frame::geometry() {
    if (m_frameCallback == nullptr)
        return QRect();

    int x = 0, y = 0, width = 0, height = 0;
    m_frameCallback("geometry", &x, &y, &width, &height);

    return QRect(x, y, width, height);
}

void Frame::SwitchToProblemFeedback() {
    if (m_frameCallback == nullptr) {
        return;
    }
    m_frameCallback("SwitchToProblemFeedback");
}
