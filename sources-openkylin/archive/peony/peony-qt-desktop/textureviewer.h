#ifndef TEXTUREVIEWER_H
#define TEXTUREVIEWER_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QSGTexture>
#include <QTimer>
#include <QOpenGLContext>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
#ifdef KY_SDK_WAYLANDHELPER
class UkuiWindowHelper;
#endif

class TextureViewer : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT
    Q_PROPERTY(bool textureAvailable READ textureAvailable NOTIFY textureAvailableChanged)

public:
    explicit TextureViewer(QOpenGLContext *shareContext = nullptr, QWindow *parent = nullptr);
    ~TextureViewer();

    void enableBlurEffect();

    Q_INVOKABLE void setSourceTexture(quint32 textureId, int width, int height);
    Q_INVOKABLE void setCornerRadius(float radius);
    bool textureAvailable() const { return m_textureId != 0; }

Q_SIGNALS:
    void textureAvailableChanged();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    void setupVertexAttribs();
    void updateBlurRegion();

    quint32 m_textureId = 0;
    QSize m_textureSize;
    bool m_textureDirty = false;
    float m_cornerRadius = 12.0f;

    QOpenGLShaderProgram *m_program = nullptr;
    GLuint m_posAttr = 0;
    GLuint m_texCoordAttr = 0;
    GLuint m_matrixUniform = 0;
    GLuint m_textureUniform = 0;
    GLuint m_cornerRadiusUniform = 0;
    GLuint m_itemWidthUniform = 0;
    GLuint m_itemHeightUniform = 0;
    GLuint m_opacityUniform = 0;

    QTimer *m_updateTimer = nullptr;
#ifdef KY_SDK_WAYLANDHELPER
    UkuiWindowHelper *m_blurHelper = nullptr;
#endif
};

#endif // TEXTUREVIEWER_H