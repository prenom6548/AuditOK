/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2026, KylinSoft Co, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef PREVIEWCONTROLBAR_H
#define PREVIEWCONTROLBAR_H

#include <QGraphicsOpacityEffect>
#include <QPointer>
#include <QPropertyAnimation>
#include <QWidget>

class QTimer;

class PreviewControlBar : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewControlBar(QWidget *parent = nullptr);
    ~PreviewControlBar() override = default;

    void setPreviewWidget(QWidget *previewWidget);
    void setFloating(bool floating);
    virtual void showBar(bool visible);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void changeEvent(QEvent *event) override;

    bool isDarkTheme() const;
    virtual bool isInteractionActive() const;
    virtual QString themeStyleSheet(bool dark) const;
    virtual void themeChanged(bool dark);
    void refreshThemeStyle();
    bool isPreviewObject(QObject *object) const;

private:
    bool isCursorInPreview() const;
    void installPreviewEventFilters();
    void scheduleHide();
    void updateFloatingGeometry();
    void scheduleThemeUpdate();

    QPointer<QWidget> m_preview_widget;
    QGraphicsOpacityEffect *m_opacity_effect;
    QPropertyAnimation *m_opacity_animation;
    QTimer *m_hide_timer;
    bool m_bar_visible = true;
    bool m_floating = false;
    bool m_updating_theme = false;
    bool m_theme_update_pending = false;
};

#endif // PREVIEWCONTROLBAR_H
