#ifndef MUSICTHUMBNAILWIDGET_H
#define MUSICTHUMBNAILWIDGET_H

#include "mprissaverwidget.h"
#include "timethumbnailwidget.h"
#include "masklable.h"
#include <QWidget>
#include <QVBoxLayout>

class MusicThumbnailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MusicThumbnailWidget(QWidget *parent = nullptr, bool isControlSaver = false);
    void setBackground(QPixmap background);
    void setSelectStatus(bool status);
private:
    void initUI();
    void animateSize(const QSize &targetSize);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    void musicModeClicked();

private:
    TimeThumbnailWidget* m_timeThumbnailWidget = nullptr;

    MPRISSaverWidget* m_mprisSaverWidget = nullptr;
    QHBoxLayout* m_mainLayout = nullptr;
    QPixmap m_background;
    MaskLable* m_maskLabel = nullptr;
    QVariantAnimation* m_animation;
    bool m_bHoverStatus = false;
    bool m_bSelectStatus = false;
    bool m_bControlSaver = false;
};

#endif // MUSICTHUMBNAILWIDGET_H
