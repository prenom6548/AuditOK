/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KQUICKMENUBUTTON_H
#define KQUICKMENUBUTTON_H

#include <QAction>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QQuickItem>
#include <QQuickWindow>
#include "ukuiwindowhelper.h"

namespace UKUISDKStyle {
#define MAILTYPE "x-scheme-handler/mailto"
#define LOCAL_CONFIG_DIR "/.config/"
#define SYSTEM_CONFIG_DIR "/usr/share/applications/"

/**
 * @brief
 * @since 1.0.0
 */
class KyQuickDialog : public QQuickItem
{
    Q_OBJECT
public:
    explicit KyQuickDialog(QQuickItem *parent = nullptr);
    ~KyQuickDialog();
    static KyQuickDialog* qmlAttachedProperties(QObject* parent);
    Q_INVOKABLE void feedbackShow();
    Q_INVOKABLE bool feedbackItemVisible();
    Q_INVOKABLE void showTileFlyout(int x, int y, int w, int h);
    /// 打开 KAboutDialog 等 QML 里 mailto 链接；逻辑对齐 kdk::KAboutDialog::linkActivated
    Q_INVOKABLE void openSupportLink(const QString &href);
    /// 打开默认支持邮箱（无 href 参数的旧入口）
    Q_INVOKABLE void appSupport();
    Q_INVOKABLE void privacyClicked();


    QString getDefaultAppId(const char *contentType);
    QStringList getSystemConfigDir();

protected:

private:
    UkuiWindowHelper *m_windowHelper = nullptr;
    QQuickWindow *m_quickWindow = nullptr;

};
}
QML_DECLARE_TYPEINFO(UKUISDKStyle::KyQuickDialog, QML_HAS_ATTACHED_PROPERTIES)

#endif // KQUICKMENUBUTTON_H
