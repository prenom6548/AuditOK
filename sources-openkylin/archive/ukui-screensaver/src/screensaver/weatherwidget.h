#ifndef WEATHERWIDGET_H
#define WEATHERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QFont>
#include <QMap>
#include "customlabel.h"

class WeatherWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WeatherWidget(QWidget *parent = nullptr);
    void setCurrentWeather(QString temperature, QString weather);
    void setErrorTip(QString tip);
    void setWeatherIcon(QString weather, QPixmap weatherStateIcon);
    void updateWeatherSize(float scale);
private:
    void initUI();

private:
    QVBoxLayout *m_mainLayout = nullptr;

    QHBoxLayout *m_textLayout = nullptr;
    QLabel *m_temperatureTextLabel = nullptr;
    QLabel *m_weatherTextLabel = nullptr;
    CustomLabel *m_netWorkTipLabel = nullptr;

    QHBoxLayout *m_iconLayout = nullptr;
    QLabel *m_outerringIconLabel = nullptr;
    QLabel *m_innerringIconLabel = nullptr;

    QHBoxLayout *m_weatherIconLayout = nullptr;
    QLabel *m_weatherIconLabel = nullptr;

    float m_scale = 0;
    QMap<QLabel*, QRect> m_allLabels;
    QMap<QLabel*, QPixmap> m_originalPixmaps;
    QMap<QLabel*, QFont> m_originalFonts;
};

#endif // WEATHERWIDGET_H
