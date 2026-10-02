// customalbumviewer.h
#ifndef CUSTOMALBUMVIEWER_H
#define CUSTOMALBUMVIEWER_H

#include <QWidget>
#include <QTimer>
#include <QDir>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "customlabel.h"
#include "adaptivelabel.h"

class CustomAlbumViewer : public QWidget
{
    Q_OBJECT

public:
    CustomAlbumViewer(QWidget *parent = nullptr);
    ~CustomAlbumViewer();

    void loadImages(const QString &directoryPath);
    void setUpdateInterval(int interval = 60000); // 默认1分钟
    void setImagesPath(const QString &strPath);
    void updateViewerSize(float scale = 0);
    void setDefaultPixmap(QPixmap pixmap);
    void stopDisplayImages();

public slots:
    void nextSlide();
    void setSequentialMode(bool sequential);

private:
    void displayImages();
    QPixmap scalePixmap(const QPixmap &original, const QSize &size);

Q_SIGNALS:
    void pixmapChanged();

private:
    float m_scale;
    QTimer *m_timer;
    QStringList m_imagePaths;
    QString m_sSelectPhotoPath;
    QPixmap m_defaultImage;
    bool m_isSequential = true;
    int m_currentIndex = 0;

    QWidget *m_centralWidget = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    CustomLabel *m_tipLabel = nullptr;
    AdaptiveLabel *m_topLabel = nullptr;
    QHBoxLayout *m_bottomLayout = nullptr;
    AdaptiveLabel *m_leftBottomLabel = nullptr;
    AdaptiveLabel *m_rightBottomLabel = nullptr;
};
#endif // CUSTOMALBUMVIEWER_H
