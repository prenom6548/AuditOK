#ifndef ALBUMTHUMBNAILWIDGET_H
#define ALBUMTHUMBNAILWIDGET_H

#include "customalbumviewer.h"
#include "timethumbnailwidget.h"
#include "masklable.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QEvent>
#include <QVariantAnimation>
#include <QEasingCurve>

class AlbumThumbnailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AlbumThumbnailWidget(QWidget *parent = nullptr, bool isControlSaver = false);
    void setBackground(QPixmap background);
    void setDefaultPath(QString defaultPath);
    void setDefaultPhoto(QPixmap photo);
    void setSelectStatus(bool status);

private:
    void initUI();
    void animateSize(const QSize &targetSize);
protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    void albumModeClicked();

private:
    TimeThumbnailWidget *m_timeThumbnailWidget = nullptr;

    CustomAlbumViewer *m_customAlbumViewer = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;
    QPixmap m_background;
    MaskLable *m_maskLabel = nullptr;
    QVariantAnimation *m_animation;
    bool m_bHoverStatus = false;
    bool m_bSelectStatus = false;
    bool m_bControlSaver = false;
};

#endif // ALBUMTHUMBNAILWIDGET_H
