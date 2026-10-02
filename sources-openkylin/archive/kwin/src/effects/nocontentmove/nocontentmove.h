#ifndef KWIN_NOCONTENTMOVE_H
#define KWIN_NOCONTENTMOVE_H

#include <QObject>
#include <kwinanimationeffect.h>

namespace KWin
{
class NoContentMoveEffect : public AnimationEffect
{
    Q_OBJECT
public:
    NoContentMoveEffect();
    ~NoContentMoveEffect();

    static bool supported();
    static bool enabledByDefault();

    inline bool isActive() const override
    {
        return m_active || AnimationEffect::isActive();
    }
    void prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime) override;
    void prePaintWindow(EffectWindow *w, WindowPrePaintData &data, std::chrono::milliseconds presentTime) override;
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;

    void slotWindowStartUserMovedResized(KWin::EffectWindow *w);
    void slotWindowStepUserMovedResized(KWin::EffectWindow *w, const QRect &geometry);
    void slotWindowFinishUserMovedResized(KWin::EffectWindow *w);

public Q_SLOTS:
    void setColor(int red, int green, int blue);
    void setAlpha(float alpha);
    void setMode(int mode);
    void setBorderWith(int border);

private:
    int border_width = 1;
    QColor color = Qt::gray;
    float alpha = 1.0f; // 透明度0~1
    int showMode = 0x01; // 1位代表窗口拖动时显示的形状，1显示为矩形，0显示为虚框。0位代表是否显示原始窗口，1为显示原窗口，0为不显示。
    bool m_active;
    EffectWindow *m_moveWindow;
    // if x, y is enough, instead of over-used QRect
    QRect m_currentGeometry, m_originalGeometry;

    void paintOpenGL(QRect rect, WindowPaintData &data);

    void LoadConfig(); // 读取Mode，Color, Border, Alpha参数。
};
}

#endif
