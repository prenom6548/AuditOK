// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef MAGNIFIER_H
#define MAGNIFIER_H

#include <QGSettings>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QWidget>

class ComKylinWlcomEffectInterface;
class OrgUkuiMagnifierInterface;

class Magnifier : public QWidget
{
    Q_OBJECT

  public:
    explicit Magnifier(int type = 1, QWidget *parent = nullptr);
    ~Magnifier();
    int checkZoomIsStarted();

  protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

  private slots:
    void on_zoomIn_clicked();
    void on_zoomOut_clicked();
    void on_scaleChanged(uint scale);
    void on_zoomStop();
    void on_themeChanged();

  private:
    int m_type;
    OrgUkuiMagnifierInterface *ukui_magnifier = nullptr;
    ComKylinWlcomEffectInterface *wlcom_effect = nullptr;

    QPushButton *button_zoomIn;
    QPushButton *button_zoomOut;
    QLabel *scale_label;
    QGSettings *m_settingsTheme = nullptr;
    QColor m_bgColor;
    bool flag_zoomIn = false;
    bool flag_zoomOut = false;

    void setupUI();
    void setupConnection();
    void enableZoom();
};

#endif // MAGNIFIER_H
