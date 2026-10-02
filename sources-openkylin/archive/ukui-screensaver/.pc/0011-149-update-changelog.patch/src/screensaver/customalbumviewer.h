// customalbumviewer.h
#ifndef CUSTOMALBUMVIEWER_H
#define CUSTOMALBUMVIEWER_H

#include <QWidget>
#include <QTimer>
#include <QDir>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class CustomAlbumViewer : public QWidget
{
    Q_OBJECT

public:
    CustomAlbumViewer(QWidget *parent = nullptr);
    ~CustomAlbumViewer();

    void loadImages(const QStringList &directoryPaths);
    void startSlideshow(int interval = 60000); // 默认1分钟
    void updateViewerSize(float scale);
    void setDefaultPixmap(QPixmap pixmap);
    void stopDisplayImages();
public slots:
    void nextSlide();
    void setSequentialMode(bool sequential);
    void setRandomMode(bool random);

private:
    void displayImages();
    QPixmap scalePixmap(const QPixmap &original, const QSize &size);

Q_SIGNALS:
    void pixmapChanged();

private:
    float m_scale;
    QTimer *m_timer;
    QStringList m_imagePaths;
    QPixmap m_defaultImage;
    bool m_isSequential = true;
    int m_currentIndex = 0;

    QWidget *m_centralWidget = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_tipLabel = nullptr;
    QLabel *m_topLabel = nullptr;
    QHBoxLayout *m_bottomLayout = nullptr;
    QLabel *m_leftBottomLabel = nullptr;
    QLabel *m_rightBottomLabel = nullptr;
};
#endif // CUSTOMALBUMVIEWER_H
