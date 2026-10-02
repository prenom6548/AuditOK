/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef UKUI_SNI_STATUS_NOTIFIER_ITEM_H
#define UKUI_SNI_STATUS_NOTIFIER_ITEM_H
#include <QObject>
#include <QMenu>
namespace UkuiSni {
class StatusNotifierItemPrivate;
class StatusNotifierItem: public QObject
{
    Q_OBJECT
public:
    explicit StatusNotifierItem(QObject *parent = nullptr);
    explicit StatusNotifierItem(const QString &service, QObject *parent = nullptr);
    Q_DISABLE_COPY(StatusNotifierItem)

    //service name + path
    void setServiceName(const QString &service);
    void setPid(const uint &pid);

    void contextMenu(int x, int y);
    void activate(int x, int y);
    void secondaryActivate(int x, int y);
    void scroll(int delta, const QString &direction);
    void provideXdgActivationToken(const QString &token);

    QString service();
    QIcon attentionIcon() const;
    QString attentionIconName() const;
    QString attentionMovieName() const;
    QString category() const;
    QIcon icon() const;
    QString iconName() const;
    QString iconThemePath() const;
    QString id() const;
    bool itemIsMenu() const;
    QIcon overlayIcon() const;
    QString overlayIconName() const;
    QString status() const;
    QString title() const;
    QVariant toolTipIcon() const;
    QString toolTipSubTitle() const;
    QString toolTipTitle() const;
    QString windowId() const;
    QString display() const;
    uint pid() const;

Q_SIGNALS:
    void contextMenuReady(QMenu *menu);
    void activateResult(bool success);
    void dataUpdated();
    void menuStateChange(bool state);

private:
    StatusNotifierItemPrivate *d = nullptr;
};
}

#endif //UKUI_SNI_STATUS_NOTIFIER_ITEM_H
