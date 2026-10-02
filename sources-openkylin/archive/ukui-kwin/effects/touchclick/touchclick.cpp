#include "touchclick.h"

#include <QTime>

#include <cmath>

namespace KWin {

TouchClickEffect::TouchClickEffect()
    :Effect()
{
    m_isActiveLongPress = false;
    m_drawProgress = 0;

    m_drawAnimation = new QPropertyAnimation(this, "drawValue");
    m_drawAnimation->setDuration(LONG_PRESS_LAST_TIME);
    m_drawAnimation->setEasingCurve(QEasingCurve::Linear);

    m_activeTimer = new QTimer(this);
    m_activeTimer->setSingleShot(true);
    m_activeTimer->setInterval(LONG_PRESS_WAIT_TIME);
    connect(m_activeTimer, &QTimer::timeout, this, &TouchClickEffect::onActiveTimerTimeout);

    loadTexture();
    loadShader();
}

TouchClickEffect::~TouchClickEffect()
{
    delete m_ringsTexture;
    m_ringsTexture = nullptr;
    delete m_ringsBgTexture;
    m_ringsBgTexture = nullptr;
    delete m_circleTexture;
    m_circleTexture = nullptr;
}

void TouchClickEffect::loadTexture()
{
    QString ringTextureFilePath = {QStandardPaths::locate(QStandardPaths::DataLocation, QStringLiteral("ringstexture.png"))};
    QImage ringsTextureImage(ringTextureFilePath);
    // 纹理与颜色融合， 纹理仅控制形状
    // uint ringR = 229, ringG = 239, ringB = 247, ringA = 165;
    uint ringR = 35, ringG = 108, ringB = 218, ringA = 216;
    for(int h = 0;h < ringsTextureImage.height(); ++h){
        for(int w = 0;w < ringsTextureImage.width(); ++w){
            const QColor &c = ringsTextureImage.pixelColor(w, h);
            ringsTextureImage.setPixelColor(w, h, QColor(ringR, ringG, ringB, fmin(ringA, c.alpha())));
        }
    }
    m_ringsTexture = new GLTexture(ringsTextureImage);

    QImage ringsBgTextureImage(ringTextureFilePath);
    // 纹理与颜色融合， 纹理仅控制形状
    uint ringBgR = 229, ringBgG = 239, ringBgB = 247, ringBgA = 165;
    // uint ringR = 35, ringG = 108, ringB = 218, ringA = 216;
    for(int h = 0;h < ringsBgTextureImage.height(); ++h){
        for(int w = 0;w < ringsBgTextureImage.width(); ++w){
            const QColor &c = ringsBgTextureImage.pixelColor(w, h);
            ringsBgTextureImage.setPixelColor(w, h, QColor(ringBgR, ringBgG, ringBgB, fmin(ringBgA, c.alpha())));
        }
    }
    m_ringsBgTexture = new GLTexture(ringsBgTextureImage);

    QString circleTextureFilePath = {QStandardPaths::locate(QStandardPaths::DataLocation, QStringLiteral("circletexture.png"))};
    QImage circleTextureImage(circleTextureFilePath);
    // 纹理与颜色融合， 纹理仅控制形状
    uint circleR = 229, circleG = 239, circleB = 247, circleA = 165;
    for(int h = 0;h < circleTextureImage.height(); ++h){
        for(int w = 0;w < circleTextureImage.width(); ++w){
            const QColor &c = circleTextureImage.pixelColor(w, h);
            circleTextureImage.setPixelColor(w, h, QColor(circleR, circleG, circleB, fmin(circleA, c.alpha())));
        }
    }
    m_circleTexture = new GLTexture(circleTextureImage);
}

void TouchClickEffect::loadShader()
{

}

void TouchClickEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    effects->prePaintScreen(data, time);

    for(auto it = m_allPoints.begin();it != m_allPoints.end();){
        if(it->time != -1){
            it->time += time;
            if(it->time > TOUCH_POINT_LIFE){
                it = m_allPoints.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void TouchClickEffect::paintScreen(int mask, const QRegion &region, ScreenPaintData &data)
{
    effects->paintScreen(mask, region, data);
    if(effects->isOpenGLCompositing())
        paintScreenGL(mask, region, data);
}

double TouchClickEffect::getDrawValue() const
{
    return m_drawProgress;
}

void TouchClickEffect::setDrawValue(double value)
{
    m_drawProgress = value;
    repaint();
}

void TouchClickEffect::onActiveTimerTimeout()
{
    m_isActiveLongPress = true;

    m_drawProgress = 0;
    m_drawAnimation->setStartValue(0.0);
    m_drawAnimation->setEndValue(1.0);
    m_drawAnimation->start();

    repaint();
}

void TouchClickEffect::paintScreenGL(int mask, const QRegion &region, ScreenPaintData &data)
{
    GLShader *shader = ShaderManager::instance()->pushShader(ShaderTrait::MapTexture);
    shader->setUniform(GLShader::ModelViewProjectionMatrix, data.projectionMatrix());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //! 绘制点击的点
    for(int i = 0;i < m_allPoints.size(); ++i){
        TouchPoint &p = m_allPoints[i];
        float cr = (TOUCH_POINT_LIFE - p.time) * 1.0 / TOUCH_POINT_LIFE * CIRCLE_RADIUS;
        // drawCircleGl(QColor(229, 239, 247, 216), p.pos.x(), p.pos.y(), cr, 0, 1);
        drawRingsGl(m_circleTexture, p.pos.x(), p.pos.y(),
                                    0, cr, 0, 1);
    }

    //! 绘制长按动画
    if(m_allPoints.size() == 1 && m_allPoints[0].time == -1 && m_isActiveLongPress){
        QPointF &p = m_allPoints[0].pos;
        // drawRingsGl(QColor(229, 239, 247, 165), p.x(), p.y(), RINGS_MINRADIUS, RINGS_MAXRADIUS, 1);
        // drawRingsGl(QColor(35, 108, 218, 216), p.x(), p.y(), RINGS_MINRADIUS, RINGS_MAXRADIUS, m_drawProgress);
        drawRingsGl(m_ringsBgTexture, p.x(), p.y(), RINGS_MINRADIUS, RINGS_MAXRADIUS, 0 ,1);
        drawRingsGl(m_ringsTexture, p.x(), p.y(), RINGS_MINRADIUS, RINGS_MAXRADIUS, 0 ,m_drawProgress);
//        drawRingsGl(m_circleTexture, p.x(), p.y()  - RINGS_MINRADIUS - RINGS_WIDTH / 2,
//                                             0, RINGS_WIDTH / 2, M_PI, 0.5);
//        drawRingsGl(m_circleTexture, p.x() + (RINGS_MINRADIUS + RINGS_WIDTH / 2) * sin(2 * M_PI * m_drawProgress),
//                                             p.y() - (RINGS_MINRADIUS + RINGS_WIDTH / 2) * cos(2 * M_PI * m_drawProgress),
//                                             0, RINGS_WIDTH / 2, 2 * M_PI * m_drawProgress, 0.5);
    }
    glDisable(GL_BLEND);
    ShaderManager::instance()->popShader();
}

void TouchClickEffect::drawCircleGl(const QColor& color, float cx, float cy, float r, float offset, float alpha)
{
    const int num_segments = 80;
    const float theta = 2.0 * M_PI * alpha / float(num_segments);
    const float c = cosf(theta);
    const float s = sinf(theta);
    float t;
    float x = r * sinf(offset);
    float y = -r * cosf(offset);

    GLVertexBuffer* vbo = GLVertexBuffer::streamingBuffer();
    vbo->reset();
    vbo->setUseColor(true);
    vbo->setColor(color);
    QVector<float> verts;
    verts.reserve(num_segments * 2 + 4);

    verts << cx << cy;
    for (int ii = 0; ii < num_segments; ++ii) {
        verts << x + cx << y + cy;
        t = x;
        x = c * x - s * y;
        y = s * t + c * y;
    }
    verts << x + cx << y + cy;
    vbo->setData(verts.size() / 2, 2, verts.data(), nullptr);
    vbo->render(GL_TRIANGLE_FAN);
}


QPointF TouchClickEffect::calculateExternalRectAndRoundRaysIntersect(float cx, float cy, float r, float alpha)
{
    QPointF res;
    float dy, dx;
    if(alpha <= M_PI_4 || alpha >= 2 * M_PI - M_PI_4){
        dy = r;
        dx = dx * tan(alpha);
    }else if(alpha >= M_PI_4 && alpha <= M_PI_2 + M_PI_4){
        dx = r;
        dy = dx * (1.0 / tan(alpha));
    }else if(alpha >= M_PI_2 + M_PI_4 && alpha <= M_PI + M_PI_4){
        dy = -r;
        dx = dy * tan(alpha);
    }else{
        dx = -r;
        dy = dx * (1.0 / tan(alpha));
    }
    // 处理成Qt里的坐标系
    dy = -dy;

    res.setX(cx + dx);
    res.setY(cy + dy);
    return res;
}

void TouchClickEffect::drawCircleGl(GLTexture *texture, float cx, float cy, float r, float offset, float alpha)
{
    static const QPointF rectPoint[] = {
        {1.0f, 1.0f},
        {0.0f, 1.0f},
        {0.0f, 0.0f},
        {1.0f, 0.0f}
    };

    QVector<float> verts, texCoords;
    verts << cx << cy;
    texCoords << 0.5f << 0.5f;

    //! 计算坐标映射的关键点
    QPointF bPos = calculateExternalRectAndRoundRaysIntersect(cx, cy, r, offset);
    QPointF ePos = calculateExternalRectAndRoundRaysIntersect(cx, cy, r, offset + alpha * 2.0 * M_PI);

    verts << bPos.x() << bPos.y();
    texCoords << (bPos.x() - cx) / r + 0.5f << (cy - bPos.y()) / r + 0.5f;

    // 太难计算，暂时先使用绘制圆环的方式去绘制圆
    // TODO 计算矩形上的关键点

}

void TouchClickEffect::drawRingsGl(const QColor &color, float cx, float cy, float rmin, float rmax, float alpha)
{
    const int num_segments = 100;
    const float theta = 2.0 * M_PI * alpha / float(num_segments);
    const float c = cosf(theta);
    const float s = sinf(theta);

    float tmin, tmax;
    float xmin = 0, xmax = 0;
    float ymin = -rmin, ymax = -rmax;
    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
    vbo->reset();
    vbo->setUseColor(true);
    vbo->setColor(color);
    QVector<float> verts;
    verts.reserve(num_segments * 4 + 4);

    for(int i = 0;i < num_segments; ++i){
        verts << xmin + cx << ymin + cy;
        verts << xmax + cx << ymax + cy;
        tmin = xmin;
        tmax = xmax;
        xmin = c * xmin - s * ymin;
        xmax = c * xmax - s * ymax;
        ymin = s * tmin + c * ymin;
        ymax = s * tmax + c * ymax;
    }
    verts << xmin + cx << ymin + cy;
    verts << xmax + cx << ymax + cy;
    vbo->setData(verts.size() / 2, 2, verts.data(), nullptr);
    vbo->render(GL_TRIANGLE_STRIP);

}

void TouchClickEffect::drawRingsGl(GLTexture *texture, float cx, float cy, float rmin, float rmax, float offset, float alpha)
{
    const int num_segments = 100;
    const float theta = 2.0 * M_PI * alpha / float(num_segments);
    const float c = cosf(theta);
    const float s = sinf(theta);

    float tmin, tmax;
    float xmin = rmin * sin(offset), xmax = rmax * sin(offset);
    float ymin = -rmin * cos(offset), ymax = -rmax * cos(offset);
    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
    vbo->reset();

    QVector<float> verts;
    QVector<float> texCoords;
    verts.reserve(num_segments * 4 + 4);

    for(int i = 0;i < num_segments; ++i){
        verts << xmin + cx << ymin + cy;
        verts << xmax + cx << ymax + cy;
        tmin = xmin;
        tmax = xmax;
        xmin = c * xmin - s * ymin;
        xmax = c * xmax - s * ymax;
        ymin = s * tmin + c * ymin;
        ymax = s * tmax + c * ymax;
        texCoords << i * 1.0f / num_segments << 1.0f;
        texCoords << i * 1.0f / num_segments << 0.0f;
    }
    verts << xmin + cx << ymin + cy;
    verts << xmax + cx << ymax + cy;
    texCoords << 1.0f << 1.0f;
    texCoords << 1.0f << 0.0f;

    texture->bind();
    vbo->setData(verts.size() / 2, 2, verts.data(), texCoords.data());
    vbo->render(GL_TRIANGLE_STRIP);
    texture->unbind();
}


void TouchClickEffect::repaint()
{
//    QRect screenRect = effects->virtualScreenGeometry();
//    QRegion dirtyArea;
//    for(int i = 0;i < m_allPoints.size(); ++i){
//        QPointF &p = m_allPoints[i].pos;
//        dirtyArea |= QRegion(QRect(p.x() - RINGS_MAXRADIUS - 10,
//                                   p.y() - RINGS_MAXRADIUS - 10,
//                                   p.x() + RINGS_MAXRADIUS + 10,
//                                   p.y() + RINGS_MAXRADIUS + 10));
//    }
//    dirtyArea &= screenRect;
//    effects->addRepaint(dirtyArea);
    //! TODO 计算dirtyArea时，在边缘时kwin有bug，不更新边缘区域。先全屏重绘。
    effects->addRepaintFull();
}


void TouchClickEffect::postPaintScreen()
{
    effects->postPaintScreen();
    repaint();
}

bool TouchClickEffect::isActive() const
{
    return m_allPoints.size();
}

bool TouchClickEffect::touchDown(qint32 id, const QPointF &pos, quint32 time)
{
    if(!m_fingerDetails.size()){
        //! 倒计时去激活长按动画
        m_activeTimer->start();
    }else{
        if(m_activeTimer->isActive()){
            m_activeTimer->stop();
        }
        m_isActiveLongPress = false;
    }
    m_fingerDetails.insert(id);
    m_allPoints.append({-1, id, pos});
    repaint();
    return false;
}

bool TouchClickEffect::touchMotion(qint32 id, const QPointF &pos, quint32 time)
{
    for(int i = 0;i < m_allPoints.size(); ++i){
        TouchPoint &p = m_allPoints[i];
        if(p.detail != id) continue;
        if(p.time >= 0) break;
        qreal dis = sqrt(pow(pos.x() - p.pos.x(), 2) + pow(pos.y() - p.pos.y(), 2));
        if(dis > MIN_SHAKE_DISTANCE){
            p.time = 0;
        }
        break;
    }
    repaint();
    return false;
}

bool TouchClickEffect::touchUp(qint32 id, quint32 time)
{   
    Q_UNUSED(time)
    if(m_fingerDetails.find(id) == m_fingerDetails.end())
        return false;
    m_fingerDetails.erase(id);
    if(!m_fingerDetails.size()){
        if(m_activeTimer->isActive()){
            m_activeTimer->stop();
        }
        m_isActiveLongPress = false;
    }
    for(int i = 0;i < m_allPoints.size(); ++i){
        TouchPoint &p = m_allPoints[i];
        if(p.detail != id) continue;
        if(p.time >= 0) break;
        p.time = 0;
    }
    return false;
}

}
