#include "frame.h"

Frame::FrameCallback *Frame::m_frameCallback = nullptr;

void Frame::setFrameCallback(FrameCallback *frameCallback) {
    m_frameCallback = frameCallback;
}

BaseData *Frame::GetBaseData() {
    if (m_frameCallback == nullptr) {
        return nullptr;
    }
    BaseData *base_data = nullptr;
    m_frameCallback("GetBaseData", &base_data);
    return base_data;
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

void Frame::SwtichToToolBox() {
    if (m_frameCallback == nullptr) {
        return;
    }
    m_frameCallback("SwitchToToolBox");
}

void Frame::SwitchToFaqSpecificType(char *type) {
    if (m_frameCallback == nullptr) {
        return;
    }
    m_frameCallback("SwitchToFaqSpecificType", type);
}

void Frame::RunFaultDiagnosis() {
    if (m_frameCallback == nullptr) {
        return;
    }
    m_frameCallback("RunFaultDiagnosis");
}

void Frame::RunJunkClean() {
    if (m_frameCallback == nullptr) {
        return;
    }
    m_frameCallback("RunJunkClean");
}

void Frame::ChangeBackButtonState(int state) {
    if (m_frameCallback == nullptr) {
        return;
    }
    m_frameCallback("ChangeBackButtonState",&state);
}

void Frame::ApplyKydoctor(char *mark, bool *ok) {
    if (m_frameCallback == nullptr) {
        return;
    }
    m_frameCallback("ApplyKydoctor", mark, ok);
}

void Frame::ReleaseKydoctor(char *mark) {
    if (m_frameCallback == nullptr) {
        return;
    }
    m_frameCallback("ReleaseKydoctor", mark);
}
