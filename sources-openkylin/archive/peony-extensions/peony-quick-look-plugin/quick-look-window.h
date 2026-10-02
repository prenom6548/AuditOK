/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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

#ifndef QUICK_LOOK_WINDOW_H
#define QUICK_LOOK_WINDOW_H

#include <QDialog>
#include <quick-look-plugin-iface.h>

class QLabel;
class QStackedWidget;
class QuickLookInfoPanel;
class QuickLookOpenButton;

class QuickLookWindow : public QDialog, public Peony::QuickLookWindowIface
{
    Q_OBJECT

public:
    explicit QuickLookWindow(QWidget *parent = nullptr);
    ~QuickLookWindow() override;

    void prepare(const QString &uri) override;
    void show(QWidget *parent = nullptr) override;
    void closeQuickLook() override;
    bool isVisible() const override;
    QObject *asObject() override;

private:
    void updateModeAndContent(const QString &uri);
    void scheduleHubPreview();
    void ensureHubWidget();
    static bool isHubMime(const QString &mime);
    static QString resolveMime(const QString &uri);

    QStackedWidget *m_contentStack = nullptr;
    QWidget *m_hubPage = nullptr;
    QWidget *m_hubWidget = nullptr;
    QLabel *m_hubLoadingLabel = nullptr;
    QuickLookInfoPanel *m_infoPanel = nullptr;
    QuickLookOpenButton *m_openButton = nullptr;
    QWidget *m_footer = nullptr;

    QString m_uri;
    bool m_isDir = false;
    bool m_useHub = false;
    int m_hubPreviewGeneration = 0;
};

#endif
