#ifndef MULTIMEDIACONTROLWIDGET_H
#define MULTIMEDIACONTROLWIDGET_H

#include <QWidget>
#include <QDBusConnectionInterface>
#include <QDBusPendingCallWatcher>
#include "mprissaverwidget.h"

#ifdef signals
#undef signals
#endif
#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

class MediaSaverWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MediaSaverWidget(QWidget *parent = nullptr, bool saverMedia = false);
    virtual ~MediaSaverWidget();

    void initMultimediaControl();
    void setSaverMediaType(bool saverMedia);
    void updateFont(QString strFont, double fontSizeF);
    void updateMediaSize(float scale);

public Q_SLOTS:
    void onStatusChanged(bool isActive);

protected:
//    void paintEvent(QPaintEvent *event);

private:
    void init(const QString &strMediaPath);
    void initConnections();

    /**
     * @brief getServiceCmd 获取dbus服务的进程信息
     * @param strMediaPath dbus服务名称
     * @return 进程全路径
     */
    QString getServiceCmd(const QString &strMediaPath);

    /**
     * @brief getDefaultAppId 获取默认mime类型的desktop
     * @param contentType mime类型
     * @return desktop文件名
     */
    QString getDefaultAppId(const char *contentType);

    /**
     * @brief getDefaultAudioAppExecInfo 获取音频默认打开二进制
     * @return 二进制路径信息
     */
    QString getDefaultAudioAppExecInfo();

    void initSaverMediaWidget();

Q_SIGNALS:
    void loadDbusListNameFinished(bool success);

private:
    MPRISSaverWidget *m_pMPRISWidget = nullptr;
    QString m_strMediaPath = "";
    QDBusConnectionInterface *m_dbusDaemonInterface = nullptr;
    QDBusPendingCallWatcher *m_dbusListNameWatcher = nullptr;

    QString m_strCurFont;
    double m_lfFontSizeF = 1.0;

    float m_scale = 0;
    bool m_bSaverMedia = false;
};

#endif // MULTIMEDIACONTROLWIDGET_H
