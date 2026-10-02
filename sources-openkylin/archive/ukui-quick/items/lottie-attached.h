/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

#ifndef UKUI_QUICK_ITEMS_LOTTIE_ATTACHED_H
#define UKUI_QUICK_ITEMS_LOTTIE_ATTACHED_H

#include <QImage>
#include <QObject>
#include <QSize>
#include <QString>
#include <QStringList>
#include <qqml.h>

#include "lottie-internal-visibility_p.h"

#include <memory>

namespace UkuiQuick {

class LottieAttachedPrivate;
class LottieItemHostAdapter;
class IconLottieHostAdapter;
class LottieAsset;
class LottieAssetRevision;
class LottieMonotonicClock;

enum class LottieErrorSummary {
    None,
    SourceValidation,
    ThemeApplication,
    FrameRasterization,
    RuntimeUnavailable
};

class LottieAttached : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source NOTIFY sourceChanged)
    Q_PROPERTY(bool playing READ playing WRITE setPlaying NOTIFY playingChanged)
    Q_PROPERTY(bool loop READ loop WRITE setLoop NOTIFY loopChanged)
    Q_PROPERTY(qreal speed READ speed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(int currentFrame READ currentFrame WRITE setCurrentFrame NOTIFY currentFrameChanged)
    Q_PROPERTY(int totalFrame READ totalFrame NOTIFY totalFrameChanged)
    Q_PROPERTY(qreal fps READ fps NOTIFY fpsChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QString preferredTheme READ preferredTheme NOTIFY preferredThemeChanged)
    Q_PROPERTY(bool followHostTheme READ followHostTheme WRITE setFollowHostTheme NOTIFY followHostThemeChanged)
    Q_PROPERTY(QStringList availableThemes READ availableThemes NOTIFY availableThemesChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(bool completed READ completed NOTIFY completedChanged)
    Q_PROPERTY(UkuiQuick::LottieAttached::HiddenPlaybackPolicy hiddenPlaybackPolicy
               READ hiddenPlaybackPolicy WRITE setHiddenPlaybackPolicy
               NOTIFY hiddenPlaybackPolicyChanged)

public:
    enum Status {
        Null,
        Loading,
        Ready,
        Error
    };
    Q_ENUM(Status)

    enum HiddenPlaybackPolicy {
        PauseWhenHidden,
        ContinueWhenHidden,
        RestartWhenShown
    };
    Q_ENUM(HiddenPlaybackPolicy)

    explicit LottieAttached(QObject *parent = nullptr);
    ~LottieAttached() override;
    static LottieAttached *qmlAttachedProperties(QObject *object);

    QString source() const;

    bool playing() const;
    void setPlaying(bool playing);

    bool loop() const;
    void setLoop(bool loop);

    qreal speed() const;
    void setSpeed(qreal speed);

    int currentFrame() const;
    void setCurrentFrame(int frame);

    int totalFrame() const;
    qreal fps() const;

    QString theme() const;
    void setTheme(const QString &theme);

    QString preferredTheme() const;
    bool followHostTheme() const;
    void setFollowHostTheme(bool followHostTheme);
    Q_INVOKABLE void resetThemePreference();

    QStringList availableThemes() const;

    Status status() const;
    QString errorString() const;
    bool active() const;
    bool completed() const;
    HiddenPlaybackPolicy hiddenPlaybackPolicy() const;
    void setHiddenPlaybackPolicy(HiddenPlaybackPolicy policy);
    QImage committedImage() const;
    void setHostRenderSize(const QSize &size);
    void setSceneActive(bool active);

Q_SIGNALS:
    void sourceChanged();
    void playingChanged();
    void loopChanged();
    void speedChanged();
    void currentFrameChanged();
    void totalFrameChanged();
    void fpsChanged();
    void themeChanged();
    void preferredThemeChanged();
    void followHostThemeChanged();
    void availableThemesChanged();
    void statusChanged();
    void errorStringChanged();
    void activeChanged();
    void completedChanged();
    void hiddenPlaybackPolicyChanged();
    void frameImageChanged();
    void loaded();

private Q_SLOTS:
    void onFrameTimerTimeout();

private:
    friend class LottieItemHostAdapter;
    friend class IconLottieHostAdapter;

    UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieAttached(
        QObject *parent,
        std::unique_ptr<LottieMonotonicClock> monotonicClock);

    void setSource(const QString &source);
    void setHostTheme(const QString &theme);
    QString requestedThemeForSource() const;
    QString effectiveThemeForAsset(const LottieAsset &asset, bool *manualPreferenceUnavailable) const;
    bool applyAsyncEffectiveTheme(quint64 themeGeneration, bool notifyTheme = true);
    void requestSourceAsync();
    void requestCurrentFrameAsync();
    void failAsync(LottieErrorSummary errorSummary);
    void updateFrameTimer();
    void stopFrameTimer();
    void startHiddenPlaybackTimer();
    void cancelHiddenPlaybackTimer();
    qint64 takeHiddenPlaybackElapsed();
    bool applyHiddenPlaybackElapsed(qint64 elapsedNsecs,
                                    bool checkpoint = true,
                                    bool requestFrame = true);
    void resetPlaybackForHiddenRestart();
    void restartSelectedFrameRequest();
    void onClockElapsed(qint64 elapsedNsecs);
    qint64 frameIntervalNsecs() const;
    int frameTimerIntervalMsecs() const;
    void advanceFrames(qint64 frameCount);
    bool selectPlaybackFrame(int frame, bool checkpoint,
                             bool requestFrame = true);
    void beginReplay();
    bool commitFrameImage(const QImage &image, const QSize &size);

    LottieAttachedPrivate *d {nullptr};
};

} // namespace UkuiQuick

QML_DECLARE_TYPEINFO(UkuiQuick::LottieAttached, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUI_QUICK_ITEMS_LOTTIE_ATTACHED_H
