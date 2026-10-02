#include "textureviewer.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QTimer>
#include <QSGTexture>
#include <QDebug>
#include <QPainterPath>
#include <QRegion>
#include <QTransform>
#ifdef KY_SDK_WAYLANDHELPER
#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>
#endif

namespace {
QRegion roundedMaskRegion(int w, int h, float radius)
{
    if (w <= 0 || h <= 0) {
        return QRegion();
    }
    if (radius <= 0.0f) {
        return QRegion(0, 0, w, h);
    }

    const qreal r = qMin<qreal>(radius, qMin(w, h) * 0.5);
    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, w, h), r, r);

    constexpr qreal kFlattenScale = 8.0;
    const QTransform scaleUp = QTransform::fromScale(kFlattenScale, kFlattenScale);
    const QList<QPolygonF> scaledPolys = path.toSubpathPolygons(scaleUp);
    QRegion region;
    for (const QPolygonF &scaledPoly : scaledPolys) {
        QPolygon logicalPoly;
        logicalPoly.reserve(scaledPoly.size());
        for (const QPointF &pt : scaledPoly) {
            logicalPoly << QPoint(qRound(pt.x() / kFlattenScale), qRound(pt.y() / kFlattenScale));
        }
        region += QRegion(logicalPoly);
    }
    return region;
}
} // namespace

TextureViewer::TextureViewer(QOpenGLContext *shareContext, QWindow *parent)
    : QOpenGLWindow(shareContext, QOpenGLWindow::NoPartialUpdate, parent)
{
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    // 防止 Wayland 合成器标记 opaque region，让透明区域和毛玻璃生效
    setOpacity(0.99);
    // 设置透明背景
    QSurfaceFormat fmt = format();
    fmt.setAlphaBufferSize(8);  // 启用 alpha 通道
    setFormat(fmt);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(16);
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        if (m_textureDirty) {
            update();
            m_textureDirty = false;
        }
    });
    m_updateTimer->start();
}

TextureViewer::~TextureViewer()
{
#ifdef KY_SDK_WAYLANDHELPER
    delete m_blurHelper;
    m_blurHelper = nullptr;
#endif
    if (m_program) {
        delete m_program;
        m_program = nullptr;
    }
}

void TextureViewer::enableBlurEffect()
{
#ifdef KY_SDK_WAYLANDHELPER
    if (!m_blurHelper) {
        m_blurHelper = new UkuiWindowHelper(this);
    }
    updateBlurRegion();
#endif
}

void TextureViewer::updateBlurRegion()
{
#ifdef KY_SDK_WAYLANDHELPER
    if (!m_blurHelper) {
        return;
    }
    m_blurHelper->setBlurEffect(roundedMaskRegion(width(), height(), m_cornerRadius), 0, true);
#endif
}

void TextureViewer::setSourceTexture(quint32 textureId, int width, int height)
{
    qDebug() << "[TextureViewer] setSourceTexture called:"
             << "ID =" << textureId
             << "Size =" << width << "x" << height;

    m_textureId = textureId;
    m_textureSize = QSize(width, height);
    m_textureDirty = true;

    Q_EMIT textureAvailableChanged();
    update();
}

void TextureViewer::setCornerRadius(float radius)
{
    m_cornerRadius = radius;
    updateBlurRegion();
    update();
}

void TextureViewer::initializeGL()
{
    qDebug() << "[TextureViewer] initializeGL called";
    initializeOpenGLFunctions();

    m_program = new QOpenGLShaderProgram(this);

    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute highp vec4 posAttr;\n"
        "attribute highp vec2 texCoordAttr;\n"
        "varying highp vec2 texCoord;\n"
        "uniform highp mat4 matrix;\n"
        "void main() {\n"
        "   texCoord = texCoordAttr;\n"
        "   gl_Position = matrix * posAttr;\n"
        "}\n");

    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                       "varying highp vec2 texCoord;\n"
                                       "uniform sampler2D texture;\n"
                                       "uniform float cornerRadius;\n"
                                       "uniform float itemWidth;\n"
                                       "uniform float itemHeight;\n"
                                       "uniform float uOpacity;\n"
                                       "void main() {\n"
                                       "   vec4 texColor = texture2D(texture, texCoord);\n"
                                       "   if (cornerRadius <= 0.0) {\n"
                                       "       gl_FragColor = texColor;\n"
                                       "       return;\n"
                                       "   }\n"
                                       "   float radius = min(cornerRadius, min(itemWidth, itemHeight) * 0.5);\n"
                                       "   vec2 halfSize = vec2(itemWidth, itemHeight) * 0.5;\n"
                                       "   vec2 pixelPos = texCoord * vec2(itemWidth, itemHeight);\n"
                                       "   vec2 q = abs(pixelPos - halfSize) - halfSize + vec2(radius);\n"
                                       "   float d = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;\n"
                                       "   float cornerAlpha = 1.0 - smoothstep(-1.0, 0.5, d);\n"
                                       "   if (cornerAlpha < 0.01) discard;\n"
                                       "   gl_FragColor = texColor * cornerAlpha;\n"
                                       "}\n");
    m_program->link();

    m_posAttr = m_program->attributeLocation("posAttr");
    m_texCoordAttr = m_program->attributeLocation("texCoordAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");
    m_textureUniform = m_program->uniformLocation("texture");
    m_cornerRadiusUniform = m_program->uniformLocation("cornerRadius");
    m_itemWidthUniform = m_program->uniformLocation("itemWidth");
    m_itemHeightUniform = m_program->uniformLocation("itemHeight");
    m_opacityUniform = m_program->uniformLocation("uOpacity");
}

void TextureViewer::paintGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (m_textureId == 0) {
        return;
    }

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.ortho(0, width(), height(), 0, -1, 1);
    m_program->setUniformValue(m_matrixUniform, matrix);

    GLfloat vertices[] = {
        0.0f, 0.0f,
        static_cast<float>(width()), 0.0f,
        static_cast<float>(width()), static_cast<float>(height()),
        0.0f, static_cast<float>(height())
    };

    GLfloat texCoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    m_program->enableAttributeArray(m_posAttr);
    m_program->enableAttributeArray(m_texCoordAttr);
    m_program->setAttributeArray(m_posAttr, vertices, 2);
    m_program->setAttributeArray(m_texCoordAttr, texCoords, 2);

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    m_program->setUniformValue(m_textureUniform, 0);

    m_program->setUniformValue(m_cornerRadiusUniform, m_cornerRadius);
    m_program->setUniformValue(m_itemWidthUniform, (float)width());
    m_program->setUniformValue(m_itemHeightUniform, (float)height());
    m_program->setUniformValue(m_opacityUniform, 0.8f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisable(GL_BLEND);  // 绘制完关闭

    m_program->disableAttributeArray(m_posAttr);
    m_program->disableAttributeArray(m_texCoordAttr);
    m_program->release();
}

void TextureViewer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    updateBlurRegion();
}

void TextureViewer::setupVertexAttribs()
{
}
