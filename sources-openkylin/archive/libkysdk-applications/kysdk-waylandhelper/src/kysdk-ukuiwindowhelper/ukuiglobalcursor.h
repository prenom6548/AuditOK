#pragma once

#include <QObject>

class UkuiGolbalCursor;
class UkuiGolbalCursorPrivate;

UkuiGolbalCursor *ukuiGolbalCursor();

class UkuiGolbalCursor
{
public:
    ~UkuiGolbalCursor();

    std::function<void()> &on_enter();
    std::function<void()> &on_leave();
    std::function<void(int, int)> &on_position();
    std::function<void(int, int)> &on_hotspot();

private:
    UkuiGolbalCursorPrivate *m_ukuiGolbalCursorPrivate = nullptr;

    explicit UkuiGolbalCursor();
    friend UkuiGolbalCursor *ukuiGolbalCursor();
};
