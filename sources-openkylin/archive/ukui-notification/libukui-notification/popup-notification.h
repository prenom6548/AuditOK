/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 */

#ifndef UKUI_NOTIFICATION_POPUP_NOTIFICATION_H
#define UKUI_NOTIFICATION_POPUP_NOTIFICATION_H
#include "ukui-notification_global.h"
#include <QObject>
#include <QDateTime>
#include <QList>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QImage>
#include <QList>
#include <QPair>
namespace UkuiNotification {
class PopupNotificationPrivate;
typedef QList<QPair<QString, QString>> ActionList;
/**
 * @short 表示一条通知。
 * @author iaom
 */
class UKUINOTIFICATION_EXPORT PopupNotification
{
    friend class NotificationSettingsTest;
    Q_GADGET
    /**
     * 通知ID。
     */
    Q_PROPERTY(uint id READ id)
    /**
     * 应用名称。
     */
    Q_PROPERTY(QString applicationName READ applicationName WRITE setApplicationName)
    /**
     * 应用图标名。
     */
    Q_PROPERTY(QString applicationIconName READ applicationIconName WRITE setApplicationIconName)
    /**
     * 通知摘要。
     */
    Q_PROPERTY(QString summary READ summary WRITE setSummary)
    /**
     * 通知消息体。
     */
    Q_PROPERTY(QString body READ body WRITE setBody)
    /**
     * 通知创建时间。
     */
    Q_PROPERTY(QDateTime createdTime READ createdTime)
    /**
     * 是否有默认action,通常用于点击通知弹窗时执行。
     */
    Q_PROPERTY(bool hasDefaultAction READ hasDefaultAction)
    /**
     * 通知action列表，表示通知包含的actions及执行动作信息，列表中两个为一组，偶数位（从0开始）表示一条action的唯一标识，奇数位表示要显示给用户的的内容。
     * 一般点击消息体产生的action为“default”，对应的内容无要求，一般不会显示给用户（但是需要在发送协议中和action拼成一组）。有额外需要用户操作的action则会在通知弹窗里以按钮形式展示。
     */
    Q_PROPERTY(ActionList actions READ actions)
    /**
     * 通知超时失效时间（通知失效后会发送NotificationClosed信号，并且通知弹窗无法再继续操作）-1：系统默认；0 不会超时； >0 表示超时时间。
     */
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout)
    /**
     * 是否支持hints中的action-icon功能。
     */
    Q_PROPERTY(bool enableActionIcons READ enableActionIcons)
    /**
     * 对应hints中的image-data字段。
     */
    Q_PROPERTY(QImage image READ image)
    /**
     * 当加载image-data失败，或通过image-path加载image失败时（image-path不是URL）,icon会被设置为image-path数据。
     */
    Q_PROPERTY(QString icon READ icon)
    /**
     * 当设置为true时，通知中心将不会在用户点击action按钮后自动移除消息。仅当用户关闭或被通知发送者关闭才会移除通知。
     */
    Q_PROPERTY(bool resident READ resident)
    /**
     * 设置为true后，通知服务将通知视为临时的（不会一直存在）。
     */
    Q_PROPERTY(bool transient READ transient)
    /**
     * 通知发送时播放的声音文件路径。
     */
    Q_PROPERTY(QString soundFile READ soundFile)
    /**
     * 声音主题中的声音名称。
     */
    Q_PROPERTY(QString soundName READ soundName)
    /**
     * 指定通知服务静音，通常用于通知发送者播放自己的声音。
     */
    Q_PROPERTY(bool suppressSound READ suppressSound)
    /**
     * 当前通知的种类。
     */
    Q_PROPERTY(QString category READ category)
    /**
     * 紧急程度。
     */
    Q_PROPERTY(Urgency urgency READ urgency WRITE setUrgency)
    /**
     * 表示当前通知要显示在哪个DISPLAY上。
     */
    Q_PROPERTY(QString display READ display)
    /**
     * 和action对应，依次表示每个action的状态(包括default action)。
     */
    Q_PROPERTY(QStringList actionState READ actionState)
    /**
     * 设置为true后，通知将不会被折叠。
     */
    Q_PROPERTY(bool noFold READ noFold)
    /**
     * 通知弹窗持续时间，超过此时间后被收纳进通知中心（单位秒）。-1：表示消息通知常驻,不会收纳进通知中心；0：表示通知将直接进入侧边栏通知中心，不会弹窗；>0: 表示通知弹窗持续时间。
     */
    Q_PROPERTY(int popupTimeout READ popupTimeout)

public:
/**
 * The notification urgency.
 */
    enum Urgency {
        /**
         * 低：不弹窗提醒，直接进入通知中心；重复应用通知会折叠。
         */
        LowUrgency =  0,
        /**
         * 普通：弹窗默认6s后关闭，进入通知中心；重复应用通知会折叠。
         */
        NormalUrgency = 1,
        /**
         * 重要：通知弹窗默认常驻，可点击关闭；通知不会折叠。
         */
        CriticalUrgency = 2,
    };
    Q_ENUM(Urgency)

    explicit PopupNotification(uint id = 0);
    PopupNotification(const PopupNotification &other);
    PopupNotification &operator=(const PopupNotification &other);
    PopupNotification &operator=(PopupNotification &&other) Q_DECL_NOEXCEPT;
    ~PopupNotification();

    /**
     * 通知ID。
     */
    uint id() const;
    /**
     * 应用名称。
     */
    QString applicationName() const;
    void setApplicationName(const QString &applicationName);

    /**
     * 应用图标名。
     */
    QString applicationIconName() const;
    void setApplicationIconName(const QString &applicationIconName);

    /**
     * 通知摘要。
     */
    QString summary() const;
    void setSummary(const QString &summary);

    /**
     * 通知消息体。
     */
    QString body() const;
    void setBody(const QString &body);

    /**
     * 是否有默认action,通常用于点击通知弹窗时执行。
     */
    bool hasDefaultAction() const;
    /**
     * 默认action的label名称。
     */
    QString defaultActionLabel();
    void setActions(const QStringList &actions);
    ActionList actions() const;
    QStringList actionState() const;

    QVariantMap hints() const;
    void setHints(const QVariantMap &hints);

    int timeout() const;
    void setTimeout(int timeout);

    QDateTime createdTime() const;
    //m_hints
    /**
     * 使用图标显示action
     * @return
     */
    bool enableActionIcons() const;
    /**
     * 分类
     * @return
     */
    QString category() const;
    /**
     * desktop名称（不带.desktop后缀）
     * @return
     */
    QString desktopEntry() const;
    /**
     * 对应image-data字段
     * @return
     */
    QImage image() const;
    /**
     * 当加载image-data失败，或通过image-path加载image失败时（image-path不是URL）,icon会被设置为image-path数据
     * @return
     */
    QString icon() const;
    /**
     * 是否持续显示，不会自动移除，直到被用户手动或发送者删除
     * @return
     */
    bool resident() const;
    /**
     * 通知发送时播放的声音文件路径
     * @return
     */
    QString soundFile() const;
    /**
     * 声音主题中的声音名称
     * @return
     */
    QString soundName() const;
    /**
     * 是否静音
     * @return
     */
    bool suppressSound() const;
    /**
     * 暂时通知（会自动移除）
     * @return
     */
    bool transient() const;

    Urgency urgency() const;
    void setUrgency(Urgency urgency);
    /**
     * 通知发送者所在的display
     * @return
     */
    QString display() const;
    /**
     * 是否不折叠
     * @return
     */
    bool noFold() const;
    /**
     * 弹窗持续时间
     * @return
     */
    int popupTimeout() const;

private:
    PopupNotificationPrivate *d = nullptr;
};
}

#endif //UKUI_NOTIFICATION_POPUP_NOTIFICATION_H
