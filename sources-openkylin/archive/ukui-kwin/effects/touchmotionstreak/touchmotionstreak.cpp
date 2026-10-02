#include "touchmotionstreak.h"

#include <kwinglplatform.h>

#include <QTime>
#include <QByteArray>

#include <cmath>

namespace KWin{

TouchMotionStreakEffect::TouchMotionStreakEffect()
    : Effect()
{
    loadTexture();
    loadShader();
}

TouchMotionStreakEffect::~TouchMotionStreakEffect()
{
    delete m_texture;
    m_texture = nullptr;
}

void TouchMotionStreakEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    for(int i = 0;i < m_touchPoints.size(); ++i){
        QVector<TouchPoint>::iterator it = m_touchPoints[i].begin();
        while(it != m_touchPoints[i].end()){
            it->life += time;
            ++it;
        }
        while(m_touchPoints[i].size() && m_touchPoints[i].first().life > MAX_LIFE){
            if(m_touchPoints[i].size() == 1){
                int detail = m_touchPoints[i].first().detail;
                if(!m_idIsUse[i]){
                    m_detail2ID.erase(detail);
                }
            }
            m_touchPoints[i].removeFirst();
        }

    }
    calTriPoints();
    effects->prePaintScreen(data, time);
}

void TouchMotionStreakEffect::paintScreen(int mask, const QRegion &region, ScreenPaintData &data)
{
    // printf("adasdasds\n");
    effects->paintScreen(mask, region, data);
    if(effects->isOpenGLCompositing()){
        paintScreenGL(data);
    }
}

void TouchMotionStreakEffect::paintScreenGL(ScreenPaintData &data)
{
    GLShader *shader = ShaderManager::instance()->pushShader(ShaderTrait::MapTexture);
    shader->setUniform(GLShader::ModelViewProjectionMatrix, data.projectionMatrix());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();

    for(int i = 0;i < m_triPoints.size(); ++i)
    {
        QVector<float> vert, texCoord;
        const int &n = m_triPoints[i].size();
        for(int j = 0;j < n; ++j){
            vert << m_triPoints[i][j].x() << m_triPoints[i][j].y();
            texCoord << ((j & 1) ? 1.0f : 0.0f) << (j >> 1) * 1.0 / (n >> 1);
        }
        vbo->reset();
        m_texture->bind();
        vbo->setData(vert.size() / 2, 2, vert.data(), texCoord.data());
        vbo->render(GL_TRIANGLE_STRIP);
        m_texture->unbind();
    }


    glDisable(GL_BLEND);
    ShaderManager::instance()->popShader();
}

void TouchMotionStreakEffect::postPaintScreen()
{
    effects->postPaintScreen();
    repaint();
}

bool TouchMotionStreakEffect::isActive() const
{
    return m_detail2ID.size();
}

bool TouchMotionStreakEffect::touchDown(qint32 id, const QPointF &pos, quint32 time)
{
    // printf("当前手指ID=%d\n", id);
    insertTouchPoint(id, pos);
    repaint();
    return false;
}

bool TouchMotionStreakEffect::touchMotion(qint32 id, const QPointF &pos, quint32 time)
{
    updateTouchPoint(id, pos);
    repaint();
    return false;
}

bool TouchMotionStreakEffect::touchUp(qint32 id, quint32 time)
{
    // printf("离开的手指ID=%d\n", id);
    removeTouchPoint(id);
    // 各触摸点开始快速衰减
    // TODO 加上滑动距离作为衰减因子
    for(int i = 0;i < m_touchPoints.size(); ++i){
        int sub;
        for(int j = 0;j < m_touchPoints[i].size(); j++){
            if(!j){
                sub = MAX_LIFE - m_touchPoints[i][j].life;
                m_touchPoints[i][j].life = MAX_LIFE;

            }else{
                m_touchPoints[i][j].life += sub;
            }
        }
    }
    return false;
}

void TouchMotionStreakEffect::insertTouchPoint(qint32 detail, QPointF pos)
{
    int id = -1;
    do{
        ++id;
        if(id == m_touchPoints.size()){
            m_touchPoints.append(QVector<TouchPoint>());
            m_triPoints.append(QVector<QPointF>());
            m_idIsUse.append(false);
        }
    }while(m_idIsUse[id] || m_touchPoints[id].size());

    m_detail2ID[detail] = id;
    m_idIsUse[id] = true;
    m_touchPoints[id].append({detail, pos, 0});

    // printf("当前分配的ID为 %d\n", id);
}

void TouchMotionStreakEffect::updateTouchPoint(qint32 detail, QPointF pos)
{
    // 开关混成有可能导致事件不成对
    if(m_detail2ID.find(detail) == m_detail2ID.end()){
        return;
    }
    int id = m_detail2ID[detail];
    m_touchPoints[id].append({detail, pos, 0});
}

void TouchMotionStreakEffect::removeTouchPoint(qint32 detail)
{
    // 开关混成有可能导致事件不成对
    if(m_detail2ID.find(detail) == m_detail2ID.end()){
        return;
    }
    int id = m_detail2ID[detail];
    m_idIsUse[id] = false;
    if(!m_touchPoints[id].size()){
        m_detail2ID.erase(detail);
    }
}

void TouchMotionStreakEffect::loadTexture()
{
    QString file_path = {QStandardPaths::locate(QStandardPaths::DataLocation, QStringLiteral(DEFAULT_TEXTURE_IMAGE))};
    QImage img(file_path);

    //! TODO 使用KWin中的纹理融合算法
    //! 没有找到kwin里，opengl的纹理融合
    //! 先手写个简单的纹理和颜色的融合，简单支持
    qreal Rd = 139.0 / 255.0;
    qreal Gd = 134.0 / 255.0;
    qreal Bd = 130.0 / 255.0;
    qreal Ad = 30.0 / 255.0;


    for(int i = 0;i < img.width(); ++i){
        for(int j = 0;j < img.height(); ++j){
            QRgb p = img.pixel(i, j);
            qreal Rs = qRed(p) / 255.0;
            qreal Gs = qGreen(p) / 255.0;
            qreal Bs = qBlue(p) / 255.0;
            qreal As = qAlpha(p) / 255.0;
            // printf("%d\n", As);
            //! 纹理和颜色融合
//            img.setPixel(i, j, qRgba((Rs * As + Rd * (1 - As)) * 255,
//                                     (Gs * As + Gd * (1 - As)) * 255,
//                                     (Bs * As + Bd * (1 - As)) * 255,
//                                     (fmin(As, Ad)) * 255));
            //! 纹理仅控制形状
            img.setPixel(i, j, qRgba(Rd * 255,
                                     Gd * 255,
                                     Bd * 255,
                                     (fmin(As, Ad)) * 255));

        }
    }
    m_texture = new GLTexture(img);
}

void TouchMotionStreakEffect::loadShader()
{
    // m_shader.reset(ShaderManager::instance()->generateShaderFromResources(ShaderTrait::MapTexture, QString(), QStringLiteral("coverswitch-reflection.glsl")));
}

void TouchMotionStreakEffect::calTriPoints()
{
    for(int i = 0;i < m_touchPoints.size(); ++i)
    {
        m_triPoints[i].clear();
        if(m_touchPoints[i].size() < 3) continue;
        QPointF last(m_touchPoints[i].first().pos);
        QVector<TouchPoint>::iterator it = m_touchPoints[i].begin();
        for(++it; it != m_touchPoints[i].end(); ++it){
            QPointF &cur = it->pos;
            qreal d = dist(cur.x(), cur.y(), last.x(), last.y());
            if(d < MIN_POINT_SEG) continue;
            if(cur.x() == last.x() || cur.y() == last.y()){
                continue;
            }
            // 方向为 last->cur 的向量
            QPointF vec_l2c(cur.x() - last.x(), cur.y() - last.y());
            // cur last线段上的中点
            QPointF mid((cur.x() + last.x()) / 2, (cur.y() + last.y()) / 2);
            d /= 2;
            // 向量的斜率k
            qreal k = (cur.y() - last.y()) / (cur.x() - last.x());
            qreal vlk = 1.0 / k;
            qreal alpha = atan(fabs(vlk));
            qreal dx = DEFAULT_STROKE_WIDTH * cos(alpha);
            qreal dy = DEFAULT_STROKE_WIDTH * sin(alpha);
            QPointF p1, p2;
            if(k > 0 && vec_l2c.x() > 0){
                p1.setX(mid.x() - dx); p1.setY(mid.y() + dy);
                p2.setX(mid.x() + dx); p2.setY(mid.y() - dy);
            }else if(k > 0 && vec_l2c.x() < 0){
                p1.setX(mid.x() + dx); p1.setY(mid.y() - dy);
                p2.setX(mid.x() - dx); p2.setY(mid.y() + dy);;
            }else if(k < 0 && vec_l2c.x() > 0){
                p1.setX(mid.x() + dx); p1.setY(mid.y() + dy);
                p2.setX(mid.x() - dx); p2.setY(mid.y() - dy);
            }else{
                p1.setX(mid.x() - dx); p1.setY(mid.y() - dy);
                p2.setX(mid.x() + dx); p2.setY(mid.y() + dy);
            }
            m_triPoints[i].append(p1);
            m_triPoints[i].append(p2);
            last = cur;

            if(it + 1 == m_touchPoints[i].end()){
                mid = it->pos;
                if(k > 0 && vec_l2c.x() > 0){
                    p1.setX(mid.x() - dx); p1.setY(mid.y() + dy);
                    p2.setX(mid.x() + dx); p2.setY(mid.y() - dy);
                }else if(k > 0 && vec_l2c.x() < 0){
                    p1.setX(mid.x() + dx); p1.setY(mid.y() - dy);
                    p2.setX(mid.x() - dx); p2.setY(mid.y() + dy);;
                }else if(k < 0 && vec_l2c.x() > 0){
                    p1.setX(mid.x() + dx); p1.setY(mid.y() + dy);
                    p2.setX(mid.x() - dx); p2.setY(mid.y() - dy);
                }else{
                    p1.setX(mid.x() - dx); p1.setY(mid.y() - dy);
                    p2.setX(mid.x() + dx); p2.setY(mid.y() + dy);
                }
                m_triPoints[i].append(p1);
                m_triPoints[i].append(p2);
            }
        }
    }

}

void TouchMotionStreakEffect::repaint()
{
    // effects->addRepaintFull();

    if(m_triPoints.size()){
        QRect dirtyRect;
        QPoint topLeft(INT32_MAX, INT32_MAX), bottomRight(INT32_MIN, INT32_MIN);
        for(int i = 0;i < m_triPoints.size(); ++i){
            for(int j = 0;j < m_triPoints[i].size(); ++j){
                QPointF &p = m_triPoints[i][j];
                topLeft.setX(fmin(topLeft.x(), p.x()));
                topLeft.setY(fmin(topLeft.y(), p.y()));
                bottomRight.setX(fmax(bottomRight.x(), p.x()));
                bottomRight.setY(fmax(bottomRight.y(), p.y()));
            }
        }
        dirtyRect.setTopLeft(topLeft);
        dirtyRect.setBottomRight(bottomRight);

        m_dirtyRect = dirtyRect;
        effects->addRepaint(m_dirtyRect);
    }else{
        effects->addRepaintFull();
    }
    return;
}

}
