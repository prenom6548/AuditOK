// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef GSETTINGSMANAGER_H
#define GSETTINGSMANAGER_H

#include <QColor>
#include <QObject>

class QGSettings;
class GSettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                   backgroundColorChanged)
    Q_PROPERTY(QColor themeColor READ themeColor WRITE setThemeColor NOTIFY themeColorChanged)
    Q_PROPERTY(QColor highLight READ highLight WRITE setHighLight NOTIFY highLightChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
  public:
    explicit GSettingsManager(QObject *parent = nullptr);
    virtual ~GSettingsManager();

    QColor borderColor() const;
    void setBorderColor(const QColor &color);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

    QColor themeColor() const;
    void setThemeColor(const QColor &color);

    QColor highLight() const;
    void setHighLight(const QColor &color);

    QColor textColor() const;
    void setTextColor(const QColor &color);

  signals:
    void borderColorChanged();
    void backgroundColorChanged();
    void themeColorChanged();
    void highLightChanged();
    void textColorChanged();

  private slots:
    void themeChanged();

  private:
    QGSettings *m_settingsTheme = nullptr;

    QColor m_borderColor;
    QColor m_backgroundColor;
    QColor m_themeColor;
    QColor m_highLight;
    QColor m_textColor;

    bool darkThemeStyle = false;
};

#endif // GSETTINGSMANAGER_H
