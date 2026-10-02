#ifndef KWIN_TOUCHCLICK_H
#define KWIN_TOUCHCLICK_H

#include <kwineffects.h>
#include <kwinglutils.h>

#include <QPointF>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVector>
#include <unordered_set>

namespace KWin
{

#define MIN_SHAKE_DISTANCE 7 // 防抖处理
#define CIRCLE_RADIUS 24
#define RINGS_WIDTH 12
#define RINGS_MINRADIUS (CIRCLE_RADIUS)
#define RINGS_MAXRADIUS (RINGS_MINRADIUS + RINGS_WIDTH)
#define LONG_PRESS_WAIT_TIME 300 /*ms*/
#define LONG_PRESS_LAST_TIME 500 /*ms*/
#define TOUCH_POINT_LIFE 250

/*!
 * \brief touch click effect and long press effect
 * \author Yunpeng Zhu.
 */
class TouchClickEffect
    : public Effect
{
    Q_OBJECT
    Q_PROPERTY(double drawValue READ getDrawValue WRITE setDrawValue)

    typedef struct _touch_point
    {
        //! 当time==0时，开始消失
        //! 当time==-1时，触摸点不消失
        int64_t time;
        int detail;
        QPointF pos;
    } TouchPoint;

public:
    TouchClickEffect();
    ~TouchClickEffect();
    void prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime) override;
    void paintScreen(int mask, const QRegion &region, ScreenPaintData &data) override;
    void postPaintScreen() override;
    bool isActive() const override;
    bool touchDown(qint32 id, const QPointF &pos, quint32 time) override;
    bool touchMotion(qint32 id, const QPointF &pos, quint32 time) override;
    bool touchUp(qint32 id, quint32 time) override;

    double getDrawValue() const;
    void setDrawValue(double value);

    static bool supported();

private Q_SLOTS:
    void onActiveTimerTimeout();

private:
    void paintScreenGL(int mask, const QRegion &region, ScreenPaintData &data);
    /*!
     * \brief 计算圆心发出的射线和圆外接矩形相交的点的位置
     * \param (cx, cy)圆心  r半径
     * \param 射线与向量[(cx,cy)->(cx, cy - r)]成夹角alpha
     * \return 相交点的位置
     */
    QPointF calculateExternalRectAndRoundRaysIntersect(float cx, float cy, float r, float alpha);
    void drawCircleGl(const QColor &color, float cx, float cy, float r, float offset, float alpha);
    void drawCircleGl(GLTexture *texture, float cx, float cy, float r, float offset, float alpha);
    void drawRingsGl(const QColor &color, float cx, float cy, float rmin, float rmax, float alpha /*0-1*/);
    void drawRingsGl(GLTexture *texture, float cx, float cy, float rmin, float rmax, float offset, float alpha /*0-1*/);
    void repaint();
    void loadTexture();

    std::chrono::milliseconds m_lastPresentTime = std::chrono::milliseconds::zero();
    bool m_isActiveLongPress;
    std::unordered_set<int> m_fingerDetails;

    QTimer *m_activeTimer;
    QPropertyAnimation *m_drawAnimation;
    //! 当前绘制的进度
    double m_drawProgress;

    QVector<TouchPoint> m_allPoints;
    GLTexture *m_ringsTexture;
    GLTexture *m_ringsBgTexture;
    GLTexture *m_circleTexture;
};

}

#endif // KWIN_TOUCHCLICK_H
