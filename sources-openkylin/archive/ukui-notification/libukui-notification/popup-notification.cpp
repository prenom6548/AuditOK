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

#include "popup-notification.h"
#include <QDBusArgument>
#include <QDebug>
#include <QImageReader>
namespace UkuiNotification {
class PopupNotificationPrivate
{
public:
    PopupNotificationPrivate();
    ~PopupNotificationPrivate();
    QImage parseImageHint(const QDBusArgument &arg);
    void loadImageFromPath(const QString &path);
    QSize maximumImageSize();

    uint m_id = 0;
    QString m_applicationName;
    QString m_applicationIconName;
    QString m_summary;
    QString m_body;
    QList<QPair<QString, QString>> m_actions;
    bool m_hasDefaultAction = false;
    QString m_defaultActionLabel;
    QVariantMap m_hints = QVariantMap();
    QString m_category;
    QImage m_image;
    QString m_icon;
    QDateTime m_createdTime;
    QString m_desktopEntry;
    PopupNotification::Urgency m_urgency = PopupNotification::NormalUrgency;
    int m_timeout = -1;
    bool m_resident = false;
    bool m_transient = false;
    QString m_soundFile;
    QString m_soundName;
    bool m_suppressSound = false;
    QString m_display;
    QStringList m_actionState;
    bool m_noFold = false;
    int m_popupTimeout = 6000; //ms
};
}
using namespace UkuiNotification;

PopupNotificationPrivate::PopupNotificationPrivate()
{
}

PopupNotificationPrivate::~PopupNotificationPrivate() = default;

QImage PopupNotificationPrivate::parseImageHint(const QDBusArgument &arg)
{
    //copy from plasma-workspace
    int width, height, rowStride, hasAlpha, bitsPerSample, channels;
    QByteArray pixels;
    char *ptr;
    char *end;

    arg.beginStructure();
    arg >> width >> height >> rowStride >> hasAlpha >> bitsPerSample >> channels >> pixels;
    arg.endStructure();

    if (width <= 0 || height <= 0 || channels <= 0 || static_cast<qint64>(rowStride) < static_cast<qint64>(channels) * width) {
        qWarning() << "Invalid image parameters: width=" << width
                   << " height=" << height << " rowStride=" << rowStride
                   << " channels=" << channels;
        return QImage();
    }

    auto copyLineRGB32 = [](QRgb *dst, const char *src, int width) {
        const char *end = src + width * 3;
        for (; src != end; ++dst, src += 3) {
            *dst = qRgb(src[0], src[1], src[2]);
        }
    };

    auto copyLineARGB32 = [](QRgb *dst, const char *src, int width) {
        const char *end = src + width * 4;
        for (; src != end; ++dst, src += 4) {
            *dst = qRgba(src[0], src[1], src[2], src[3]);
        }
    };

    QImage::Format format = QImage::Format_Invalid;
    void (*fcn)(QRgb *, const char *, int) = nullptr;
    if (bitsPerSample == 8) {
        if (channels == 4) {
            format = QImage::Format_ARGB32;
            fcn = copyLineARGB32;
        } else if (channels == 3) {
            format = QImage::Format_RGB32;
            fcn = copyLineRGB32;
        }
    }
    if (format == QImage::Format_Invalid) {
        qWarning() << "Unsupported image format (hasAlpha:" << hasAlpha << "bitsPerSample:" << bitsPerSample
                    << "channels:" << channels << ")";
        return QImage();
    }

    QImage image(width, height, format);
    ptr = pixels.data();
    end = ptr + pixels.length();
    for (int y = 0; y < height; ++y, ptr += rowStride) {
        if (ptr + channels * width > end) {
            qWarning() << "Image data is incomplete. y:" << y << "height:" << height;
            break;
        }
        fcn((QRgb *)image.scanLine(y), ptr, width);
    }

    return image;
}

void PopupNotificationPrivate::loadImageFromPath(const QString &path)
{
    //copy from plasma-workspace
    QUrl imageUrl;
    if (path.startsWith(QLatin1Char('/'))) {
        imageUrl = QUrl::fromLocalFile(path);
    } else if (path.contains(QLatin1Char('/'))) {
        imageUrl = QUrl(path);

        if (!imageUrl.isLocalFile()) {
            qDebug() << "Refused to load image from" << path << "which isn't a valid local location.";
            return;
        }
    }

    if (!imageUrl.isValid()) {
        m_icon = path;
        return;
    }

    QImageReader reader(imageUrl.toLocalFile());
    reader.setAutoTransform(true);

    const QSize imageSize = reader.size();
    if (imageSize.isValid() && (imageSize.width() > maximumImageSize().width() || imageSize.height() > maximumImageSize().height())) {
        const QSize thumbnailSize = imageSize.scaled(maximumImageSize(), Qt::KeepAspectRatio);
        reader.setScaledSize(thumbnailSize);
    }

    m_image = reader.read();
}

QSize PopupNotificationPrivate::maximumImageSize()
{
    return QSize(256, 256);
}

/*!
    \class UkuiNotification::PopupNotification
    \inmodule UkuiNotification
    \inheaderfile popup-notification.h
    \brief 表示一条通知，包含一条通知消息的全部.

    PopupNotification 类封装了一条通知的所有属性和行为，包括通知ID、应用程序名称、图标、摘要、正文、动作、提示信息、类别、图片、声音等。
 */

/*!
    \enum PopupNotification::Urgency
    通知的紧急程度.
    \value LowUrgency 低：不弹窗提醒，直接进入通知中心；重复应用通知会折叠
    \value NormalUrgency 普通：弹窗默认6s后关闭，进入通知中心；重复应用通知会折叠
    \value CriticalUrgency 重要：通知弹窗默认常驻，可点击关闭；通知不会折叠
*/

/*!
    \typedef ActionList
    \relates UkuiNotification

    同 QList<QPair<QString, QString>>
*/

/*!
    Construct from \a id.
*/
PopupNotification::PopupNotification(uint id) : d(new PopupNotificationPrivate())
{
    d->m_id = id;
}

/*!
    Copy constructor from \a other.
*/
PopupNotification::PopupNotification(const PopupNotification &other) : d(new PopupNotificationPrivate(*other.d))
{
}

/*!
    Copy assignment operator from \a other.
*/
PopupNotification &PopupNotification::operator=( const PopupNotification &other)
{
    *d = *other.d;
    return *this;
}

/*!
    Move assignment operator from \a other.
*/
PopupNotification &PopupNotification::operator=(PopupNotification &&other) Q_DECL_NOEXCEPT
{
    d = other.d;
    other.d = nullptr;
    return *this;
}

/*!
    Destructor
*/
PopupNotification::~PopupNotification()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

/*!
    \readonly
    \property PopupNotification::id
    \brief 通知的ID
*/
uint PopupNotification::id() const
{
    return d->m_id;
}

/*!
    \property PopupNotification::applicationName
    \brief 应用程序名称.
*/
QString PopupNotification::applicationName() const
{
    return d->m_applicationName;
}

void PopupNotification::setApplicationName(const QString &applicationName)
{
    d->m_applicationName = applicationName;
}

/*!
    \property PopupNotification::applicationIconName
    \brief 应用程序图标名称.
*/
QString PopupNotification::applicationIconName() const
{
    return d->m_applicationIconName;
}

void PopupNotification::setApplicationIconName(const QString &applicationIconName)
{
    d->m_applicationIconName = applicationIconName;
}

/*!
    \property PopupNotification::summary
    \brief 通知消息的摘要.
*/
QString PopupNotification::summary() const
{
    return d->m_summary;
}

void PopupNotification::setSummary(const QString &summary)
{
    d->m_summary = summary;
}

/*!
    \property PopupNotification::body
    \brief 通知消息的正文.
*/
QString PopupNotification::body() const
{
    return d->m_body;
}

void PopupNotification::setBody(const QString &body)
{
    d->m_body = body;
}

/*!
    \readonly
    \property PopupNotification::hasDefaultAction
    \brief 通知消息是否有默认 action，通常用于点击通知弹窗时执行。
*/
bool PopupNotification::hasDefaultAction() const
{
    return d->m_hasDefaultAction;
}

QString PopupNotification::defaultActionLabel()
{
    return d->m_defaultActionLabel;
}

void PopupNotification::setActions(const QStringList &actions)
{
    if (actions.count() % 2 != 0) {
        qWarning() << "List of actions must contain an even number of items, tried to set actions to" << actions;
        return;
    }

    d->m_hasDefaultAction = false;

    for (int i = 0; i < actions.count(); i += 2) {
        const QString &key = actions.at(i);
        const QString &label = actions.at(i + 1);

        if (!d->m_hasDefaultAction && key == QLatin1String("default")) {
            d->m_hasDefaultAction = true;
            d->m_defaultActionLabel = label;
        }
        d->m_actions.append({key, label});
    }
}

/*!
    \readonly
    \property PopupNotification::actions
    \brief 通知消息的 action 列表.

    通知 action 列表，表示通知包含的 actions 及执行动作信息，列表中两个为一组，偶数位（从0开始）表示一条 action 的唯一标识，奇数位表示要显示给用户的的内容。
    一般点击消息体产生的  action 为 “default”，对应的内容无要求，一般不会显示给用户（但是需要在发送协议中和 action 拼成一组）。有额外需要用户操作的 action 则会在通知弹窗里以按钮形式展示。
*/
QList<QPair<QString, QString>> PopupNotification::actions() const
{
    return d->m_actions;
}

QVariantMap PopupNotification::hints() const
{
    return d->m_hints;
}

void PopupNotification::setHints(const QVariantMap &hints)
{
    d->m_hints = hints;
    d->m_desktopEntry = hints.value(QStringLiteral("desktop-entry")).toString();
    bool ok;
    int urgency = hints.value(QStringLiteral("urgency")).toInt(&ok);
    if(ok) {
        switch (urgency) {
            default:
            case 0: //低等级，不弹窗
                setUrgency(Urgency::LowUrgency);
                d->m_popupTimeout = 0;
                break;
            case 1: //中等级，弹窗时间默认6秒
                setUrgency(Urgency::NormalUrgency);
                d->m_popupTimeout = 6000;
                break;
            case 2: //高等级，默认弹窗常驻，不可折叠
                setUrgency(Urgency::CriticalUrgency);
                d->m_popupTimeout = -1;
                d->m_noFold = true;
                break;
        }
    }
    d->m_createdTime = QDateTime::fromString(hints.value(QStringLiteral("x-ukui-createdTime")).toString());
    d->m_resident = hints.value(QStringLiteral("resident")).toBool();
    d->m_transient = hints.value(QStringLiteral("transient")).toBool();
    d->m_category = hints.value(QStringLiteral("category")).toString();

    auto end = hints.end();
    auto it = hints.find(QStringLiteral("image-data"));
    if (it != end) {
        d->m_image = d->parseImageHint(it->value<QDBusArgument>());
        if (!d->m_image.isNull()) {
            const QSize max = d->maximumImageSize();
            if (d->m_image.size().width() > max.width() || d->m_image.size().height() > max.height()) {
                d->m_image = d->m_image.scaled(max, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
        }
    }
    if (d->m_image.isNull()) {
        it = hints.find(QStringLiteral("image-path"));

        if (it != end) {
            d->loadImageFromPath(it->toString());
        }
    }

    d->m_soundFile = hints.value(QStringLiteral("sound-file")).toString();
    d->m_soundName = hints.value(QStringLiteral("sound-name")).toString();
    d->m_suppressSound = hints.value(QStringLiteral("suppress-sound")).toBool();
    d->m_display = hints.value(QStringLiteral("x-ukui-display")).toString();
    d->m_actionState = hints.value(QStringLiteral("x-ukui-action-state")).toStringList();

    QVariant tmp = hints.value(QStringLiteral("x-ukui-no-fold"));
    if(!tmp.isNull()) {
        d->m_noFold = tmp.toBool();
    }
    tmp = hints.value(QStringLiteral("x-ukui-popup-timeout"));
    if(!tmp.isNull()) {
        d->m_popupTimeout = tmp.toInt();
    }
}

/*!
    \property PopupNotification::timeout
    \brief 通知消息的超时时间.
*/
int PopupNotification::timeout() const
{
    return d->m_timeout;
}

void PopupNotification::setTimeout(int timeout)
{
    d->m_timeout = timeout;
}

/*!
    \readonly
    \property PopupNotification::createdTime
    \brief 通知消息的创建时间.
*/
QDateTime PopupNotification::createdTime() const
{
    return d->m_createdTime;
}

/*!
    \readonly
    \property PopupNotification::enableActionIcons
    \brief 是否支持 hints 中的 action-icon 功能.
*/
bool PopupNotification::enableActionIcons() const
{
    //TODO 可以支持action-icon
    return false;
}

/*!
    \readonly
    \property PopupNotification::category
    \brief 通知消息的分类.
*/
QString PopupNotification::category() const
{
    return d->m_category;
}

QString PopupNotification::desktopEntry() const
{
    return d->m_desktopEntry;
}

/*!
    \readonly
    \property PopupNotification::image
    \brief 对应image-data字段.
*/
QImage PopupNotification::image() const
{
    return d->m_image;
}

/*!
    \readonly
    \property PopupNotification::icon
    \brief 当加载 image-data 失败，或通过 image-path 加载 image 失败时（image-path 不是 URL）,icon 会被设置为 image-path 数据。
*/
QString PopupNotification::icon() const
{
    return d->m_icon;
}

/*!
    \readonly
    \property PopupNotification::resident
    \brief 通知消息是否常驻.
    当设置为true时，通知中心将不会在用户点击 action 按钮后自动移除消息。仅当用户关闭或被通知发送者关闭才会移除通知。
*/
bool PopupNotification::resident() const
{
    return d->m_resident;
}

/*!
    \readonly
    \property PopupNotification::soundFile
    \brief 通知发送时播放的声音文件路径。
*/
QString PopupNotification::soundFile() const
{
    return d->m_soundFile;
}

/*!
    \readonly
    \property PopupNotification::soundName
    \brief 声音主题中的声音名称。
*/
QString PopupNotification::soundName() const
{
    return d->m_soundName;
}

/*!
    \readonly
    \property PopupNotification::suppressSound
    \brief 指定通知服务静音，通常用于通知发送者播放自己的声音。
*/
bool PopupNotification::suppressSound() const
{
    return d->m_suppressSound;
}

/*!
    \readonly
    \property PopupNotification::transient
    \brief 设置为 true 后，通知服务将通知视为临时的（不会一直存在）
*/
bool PopupNotification::transient() const
{
    return d->m_transient;
}

/*!
    \readonly
    \property PopupNotification::display
    \brief 表示当前通知要显示在哪个DISPLAY上.
*/
QString PopupNotification::display() const
{
    return d->m_display;
}

void PopupNotification::setUrgency(PopupNotification::Urgency urgency)
{
    d->m_urgency = urgency;
}

PopupNotification::Urgency PopupNotification::urgency() const
{
    return d->m_urgency;
}

/*!
    \readonly
    \property PopupNotification::actionState
    \brief 和 action 对应，依次表示每个 action 的状态(包括 default action).
*/
QStringList PopupNotification::actionState() const {
    return d->m_actionState;
}

/*!
    \readonly
    \property PopupNotification::noFold
    \brief 设置为true后，通知将不会被折叠.
*/
bool PopupNotification::noFold() const {
    return d->m_noFold;
}

/*!
    \readonly
    \property PopupNotification::popupTimeout
    \brief 通知弹窗持续时间.
*/
int PopupNotification::popupTimeout() const {
    return d->m_popupTimeout;
}
