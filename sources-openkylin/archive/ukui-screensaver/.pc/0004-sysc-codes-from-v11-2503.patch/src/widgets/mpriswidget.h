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
#include "klabel.h"

class MPRISWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MPRISWidget(QWidget *parent = nullptr);

    void init(QString strMediaPlayerPath);

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

Q_SIGNALS:
    void statusChanged(bool isValid);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void initUI();
    void initConnections();

private:
    QDBusConnectionInterface *m_mprisMonitor = nullptr;
    MPRISDBusClient *m_mprisDbusClient = nullptr;
    QString m_strMediaPlayerPath;
    int m_nPlayState = -1;
    bool m_isPlayBtnClicked = false;
    QPixmap m_pixmapAlbumDefault;

    // UI
    QHBoxLayout *m_mainLayout = nullptr;
    QLabel *m_labelAlbum = nullptr; // 专辑图片

    QVBoxLayout *m_rightLayout = nullptr;
    KLabel *m_labelName = nullptr;
    KLabel *m_labelArtist = nullptr;
    QLabel *m_labelLine = nullptr;

    QWidget *m_wdgControl = nullptr;
    QPushButton *m_btnPrev = nullptr;
    QPushButton *m_btnPlayPause = nullptr;
    QPushButton *m_btnNext = nullptr;
};

#endif // MPRISWIDGET_H
