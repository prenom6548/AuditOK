#ifndef WEATHERTHUMBNAILWIDGET_H
#define WEATHERTHUMBNAILWIDGET_H

#include "masklable.h"
#include "timethumbnailwidget.h"
#include "weatherwidget.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QVariantAnimation>

class WeatherThumbnailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WeatherThumbnailWidget(QWidget *parent = nullptr, bool isControlSaver = false);
    void setBackground(QPixmap background);
    void setSelectStatus(bool status);
    void setCurrentWeather(QString temperature, QString weather);
    void setErrorTip(QString tip);
    void setWeatherIcon(QString weather, QPixmap weatherStateIcon);

private:
    void initUI();
    void animateSize(const QSize &targetSize);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    void weatherModeClicked();
private:
    TimeThumbnailWidget* m_timeThumbnailWidget = nullptr;
    WeatherWidget *m_weatherWidget = nullptr;

    QHBoxLayout* m_mainLayout = nullptr;
    QPixmap m_background;
    QVariantAnimation* m_animation;
    MaskLable* m_maskLabel = nullptr;
    bool m_bHoverStatus = false;
    bool m_bSelectStatus = false;
    bool m_bControlSaver = false;

};

#endif // WEATHERTHUMBNAILWIDGET_H
