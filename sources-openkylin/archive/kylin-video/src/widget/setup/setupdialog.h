/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QColor>
#include <QListWidgetItem>
//#include <knavigationbar.h>

//using namespace kdk;

class SetupSystem;
class SetupPlay;
class SetupScreenshot;
class SetupSubtitle;
class SetupVolume;
class SetupCodec;
class SetupShortcut;

namespace Ui {
class SetUpDialog;
}

class SetUpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetUpDialog(QWidget *parent = nullptr);
    ~SetUpDialog();
    void setIndex(int index);

private:
    Ui::SetUpDialog *ui;

    double m_windowOpacity;
    QColor m_leftColor;
    QColor m_highlightColor;
    bool m_isBlackTheme;

    SetupSystem     *m_systemPage;
    SetupPlay       *m_playPage;
    SetupScreenshot *m_screenshotPage;
    SetupSubtitle   *m_subtitlePage;
    SetupVolume     *m_volumePage;
    SetupCodec      *m_codecPage;
    SetupShortcut   *m_shortcutPage;

//    KNavigationBar *leftBar;

    void initStyle();
    void initListWidget();
    void initConnect();
    void resetFont();

private slots:
    void slotChangeTheme(bool is_black_theme);

private:
    bool event(QEvent *e);
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);
    void done(int r) override;
    void paintEvent(QPaintEvent *e);
    void updateHightLightColor();
    void resetText(QListWidgetItem *item);
};

#endif // SETUPDIALOG_H
