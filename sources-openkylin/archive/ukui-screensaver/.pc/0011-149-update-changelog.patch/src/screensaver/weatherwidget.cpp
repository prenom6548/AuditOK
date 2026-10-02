#include "weatherwidget.h"
#include <QDebug>

WeatherWidget::WeatherWidget(QWidget *parent)
    : QWidget{parent}
{
    initUI();
}

void WeatherWidget::initUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(8);

    m_textLayout = new QHBoxLayout();
    m_textLayout->setSpacing(16);

    m_netWorkTipLabel = new QLabel(this);
    m_netWorkTipLabel->setStyleSheet("color: white");
    QFont networkFont = m_netWorkTipLabel->font();
    networkFont.setPointSizeF(24);
    m_netWorkTipLabel->setFont(networkFont);
    m_netWorkTipLabel->setText(tr("Not connected to a network"));
    m_netWorkTipLabel->setFixedHeight(55);
    m_netWorkTipLabel->hide();

    m_temperatureTextLabel = new QLabel(this);
    m_temperatureTextLabel->setStyleSheet("color: white");
    m_temperatureTextLabel->setMinimumWidth(120);
    m_temperatureTextLabel->setFixedHeight(55);
    QFont temperatureFont = m_temperatureTextLabel->font();
    temperatureFont.setPointSizeF(42);
    m_temperatureTextLabel->setFont(temperatureFont);

    m_weatherTextLabel = new QLabel(this);
    m_weatherTextLabel->setStyleSheet("color: white");
    m_weatherTextLabel->setMinimumWidth(140);
    m_weatherTextLabel->setFixedHeight(55);
    m_weatherTextLabel->setContentsMargins(0, 5, 0, 0);

    QFont weatherFont = m_weatherTextLabel->font();
    weatherFont.setPointSizeF(24);
    m_weatherTextLabel->setFont(weatherFont);

    m_textLayout->addStretch();
    m_textLayout->addWidget(m_netWorkTipLabel);
    m_textLayout->addWidget(m_temperatureTextLabel);
    m_textLayout->addWidget(m_weatherTextLabel);
    m_textLayout->addStretch();
    m_mainLayout->addLayout(m_textLayout);

    m_iconLayout = new QHBoxLayout();
    m_iconLayout->setContentsMargins(0, 0, 100, 0);
    m_iconLayout->setSpacing(0);
    m_weatherIconLabel = new QLabel(this);
    m_weatherIconLabel->setFixedSize(200, 160);

    m_outerringIconLabel = new QLabel(this);
    m_outerringIconLabel->setFixedSize(534, 706);
    QPixmap m_pixmapOuterring
            = QPixmap(":/assets/outerring.png").scaled(534, 706, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_outerringIconLabel->setPixmap(m_pixmapOuterring);

    m_weatherIconLayout = new QHBoxLayout(m_outerringIconLabel);
    m_weatherIconLayout->setContentsMargins(0, 114, 297, 192);

    m_innerringIconLabel = new QLabel(m_outerringIconLabel);
    m_innerringIconLabel->setFixedSize(286, 420);
    QPixmap m_innerringDefault
            = QPixmap(":/assets/underlying.png").scaled(286, 420, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_innerringIconLabel->setPixmap(m_innerringDefault);
    m_weatherIconLayout->addWidget(m_weatherIconLabel, Qt::AlignLeft);
    m_weatherIconLayout->addWidget(m_innerringIconLabel, Qt::AlignCenter);

    m_netWorkTipLabel->setFixedWidth(this->width());

    m_iconLayout->addWidget(m_outerringIconLabel, Qt::AlignCenter);
    m_mainLayout->addLayout(m_iconLayout);
}

void WeatherWidget::updateWeatherSize(float scale)
{
    m_mainLayout->setSpacing(8 * scale);
    m_textLayout->setSpacing(16 * scale);
    m_iconLayout->setContentsMargins(0, 0, 100 * scale, 0);
    m_weatherTextLabel->setContentsMargins(0, 10 * scale, 0, 0);
    if (scale < 0.11) {
        m_weatherIconLayout->setContentsMargins(0, 120 * scale, 350 * scale, 198 * scale);
    } else if (scale < 0.2) {
        m_weatherIconLayout->setContentsMargins(0, 114 * scale, 340 * scale, 192 * scale);
    } else {
        m_weatherIconLayout->setContentsMargins(0, 114 * scale, 297 * scale, 192 * scale);
    }

    if (m_allLabels.isEmpty()) {
        QList<QLabel*> labels = this->findChildren<QLabel*>();
        foreach (QLabel* label, labels) {
            m_allLabels[label] = label->rect();
            const QPixmap* pixmap = label->pixmap();
            if (pixmap && !pixmap->isNull()) {
                if (!m_originalPixmaps.contains(label)) {
                    m_originalPixmaps[label] = *pixmap;
                }
            }
            const QFont font = label->font();
            if (!m_originalFonts.contains(label)) {
                m_originalFonts[label] = font;
            }
        }
    }

    QList<QLabel*> labels = this->findChildren<QLabel*>();
    foreach (QLabel* label, labels) {
        const QPixmap* pixmap = label->pixmap();
        QSize newSize = m_allLabels[label].size() * scale;
        if (pixmap && !pixmap->isNull()) {
            QPixmap scaledPixmap = m_originalPixmaps[label].scaled(
                newSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
            label->setPixmap(scaledPixmap);
        }
        label->setFixedSize(newSize);
        QFont font = label->font();
        font.setPointSizeF(m_originalFonts[label].pointSize() * scale);
        label->setFont(font);
    }
    m_scale = scale;
}

void WeatherWidget::setCurrentWeather(QString temperature, QString weather)
{
    m_temperatureTextLabel->setText(temperature);
    m_weatherTextLabel->setText(weather);
    m_temperatureTextLabel->adjustSize();
    m_weatherTextLabel->adjustSize();
    m_temperatureTextLabel->show();
    m_weatherTextLabel->show();
    m_netWorkTipLabel->hide();
    m_innerringIconLabel->lower();
}

void WeatherWidget::setWeatherIcon(QPixmap weather, QPixmap weatherStateIcon)
{
    m_weatherIconLabel->setPixmap(weather.scaled(150 * m_scale, 160 * m_scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    m_innerringIconLabel->setPixmap(weatherStateIcon.scaled(286 * m_scale, 420 * m_scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void WeatherWidget::setNetworkErrorTip()
{
    m_temperatureTextLabel->hide();
    m_weatherTextLabel->hide();
    m_netWorkTipLabel->show();
    m_netWorkTipLabel->setFixedWidth(this->width());
    QPixmap m_innerringDefault
            = QPixmap(":/assets/underlying.png").scaled(286 * m_scale, 420 * m_scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_innerringIconLabel->setPixmap(m_innerringDefault);
    m_weatherIconLabel->setPixmap(QPixmap());
    m_innerringIconLabel->lower();
}



