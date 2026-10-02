#ifndef MPRISWIDGET_H
#define MPRISWIDGET_H

#include <QFrame>
#include <QDBusInterface>
#include "freedesktophelper.h"
#include "mprisdbusclient.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDBusConnectionInterface>
#include <QVariantMap>
#include <QTimer>
#include "customlabel.h"
#include "dbusadapter.h"
#include "rotatablelabel.h"
#include "cornerrotatinglabel.h"

class MPRISSaverWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MPRISSaverWidget(QWidget *parent = nullptr, bool saverMedia = false);
    ~MPRISSaverWidget();
    void init(QString strMediaPlayerPath);
    void setSaverMediaType(bool saverMedia);

public Q_SLOTS:
    void onServiceStatusChanged(QString strName, bool isActive);
    void onLoadMediaPath(const QString &strPath);
    void onLostMediaPath(const QString &strPath);

    void onBtnPreview();
    void onBtnNext();
    void onBtnPlayPause();

    void onMetadataChanged();
    void onPlaybackStatusChanged();
    void onCanControlChanged();
    void updateFont(QString strFont, double fontSizeF);

    void updateMediaSize(float scale);

    void onDelayMetadataUpdate();

    void onPlayerStateChanged(int status);
Q_SIGNALS:
    void statusChanged(bool isValid);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void initUI();
    void initConnections();

    void delayMetadataUpdate(int nTimeout = 1000);

private:
    QDBusConnectionInterface *m_mprisMonitor = nullptr;
    MPRISDBusClient *m_mprisDbusClient = nullptr;
    QString m_strMediaPlayerPath;
    int m_nPlayState = -1;
    bool m_isPlayBtnClicked = false;
    QPixmap m_pixmapAlbumDefault;

    // UI
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_pLabelDitu = nullptr; // 默认底图
    RotatingImageLabel *m_labelAlbum = nullptr; // 专辑图片
    CornerRotatingLabel *m_labelBopian = nullptr;    // 拨片图片
    QHBoxLayout *m_albumLayout = nullptr;

    QLabel *m_labelName = nullptr;
    QLabel *m_labelArtist = nullptr;
    QLabel *m_labelLine = nullptr;

    QHBoxLayout *m_buttonLayout = nullptr;
    QPushButton *m_btnPrev = nullptr;
    QPushButton *m_btnPlayPause = nullptr;
    QPushButton *m_btnNext = nullptr;

    QTimer *m_timerDelayUpdate = nullptr;

    bool m_bSaverMedia = false;
    DbusAdapter *m_adapter = nullptr;

    float m_scale = 0;
    QMap<QLabel*, QRect> m_allLabels;
    QMap<QLabel*, QPixmap> m_originalPixmaps;
    QMap<QLabel*, QFont> m_originalFonts;
    QMap<QPushButton*, QRect> m_allButtons;
};

#endif // MPRISWIDGET_H
