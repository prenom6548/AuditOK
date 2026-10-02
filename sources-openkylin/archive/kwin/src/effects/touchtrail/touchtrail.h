#ifndef KWIN_TOUCHMOTIONSTREAK_H
#define KWIN_TOUCHMOTIONSTREAK_H

#include <kwineffects.h>
#include <kwinglutils.h>

#include <QList>

#include <unordered_map>

namespace KWin
{

#define MAX_FINGER_COUNT 10 // 触摸屏支持最大触摸点的个数
#define MIN_POINT_SEG 2 /*px*/ // 触摸点之间允许的最小欧氏距离
#define DEFAULT_STROKE_WIDTH 3 /*px*/ // 条带的宽度
#define MAX_POINT_NUMBER 8 // 保存的触摸点的最大数量
#define DEFAULT_TEXTURE_IMAGE "trailing.png" // 默认纹理
#define MAX_LIFE 200 // 每个触摸点存活的最长时间

#define dist(x1, y1, x2, y2) sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))

/*!
 * \brief UKUI Touch Motion Streak Effect
 * \author Yunpeng Zhu.
 */
class TouchTrailEffect
    : public Effect
{
    Q_OBJECT
    typedef struct _touch_point
    {
        int detail;
        QPointF pos;
        int life;
    } TouchPoint;

public:
    TouchTrailEffect();
    virtual ~TouchTrailEffect();

    void prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime) override;
    void paintScreen(int mask, const QRegion &region, ScreenPaintData &data) override;
    void postPaintScreen() override;

    bool isActive() const override;
    bool touchDown(qint32 id, const QPointF &pos, quint32 time) override;
    bool touchMotion(qint32 id, const QPointF &pos, quint32 time) override;
    bool touchUp(qint32 id, quint32 time) override;

    static bool supported();

private:
    void paintScreenGL(ScreenPaintData &data);
    void insertTouchPoint(qint32 detail, QPointF pos);
    void updateTouchPoint(qint32 detail, QPointF pos);
    void removeTouchPoint(qint32 detail);
    void loadTexture();
    void loadShader();
    /*!
     * \brief 计算三角形条带顶点的位置
     */
    void calTriPoints();
    void repaint();

    //! 保存触摸点坐标
    QVector<QVector<TouchPoint>> m_touchPoints;
    //! 保存所需要绘制的三角形条带的坐标
    QVector<QVector<QPointF>> m_triPoints;
    //! 标记id是否被使用
    QVector<bool> m_idIsUse;
    //! 将detail映射到0~MAX_FINGER_COUNT-1
    std::unordered_map<int, int> m_detail2ID;
    //! 绘制
    GLTexture *m_texture;
    //! 记录需要重绘的区域
    QRect m_dirtyRect;
    std::chrono::milliseconds m_lastPresentTime = std::chrono::milliseconds::zero();
};

}

#endif // KWIN_TOUCHMOTIONSTREAK_H
