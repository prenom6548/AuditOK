#ifndef UPDATENOTIFICATIONWIDGET_H
#define UPDATENOTIFICATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimerEvent>
#include <QDebug>
#include <QHostInfo>

#include <curl/curl.h>

#include "kaglobal.h"
#include <modeldownload.h>
#include "ktoolbutton.h"

using namespace  std;
using namespace kdk;

class UpdateNotificationWidget : public QWidget
{
    Q_OBJECT
public:
    enum UpdateState {
        NewVersionAvailable,
        LatestVersion,
        NetworkError
    };

    explicit UpdateNotificationWidget(PublicLocalModel &model, QWidget *parent = nullptr);

    void initLayout();
    void setUpdateState(UpdateState state);
    void resetProgressBar();
    bool getNetStatus();
    bool checkNetWorkOnline();
    void setCancelDownloadButtonVisible(bool isVisible);
    void setProgressBarVisible(bool isVisible);
    void setCancelDownloadButtenEnabled(bool isEnabled);
    QString getDownloadUrl();
public slots:
    void slUpdateProgressValue(int value);
    void slClickExitButton();
signals:
    void updateDownloadProgress(int value );
    void sigCancelDownload();
    void sigShowIsInstalling();
private:
private:
    QHBoxLayout *m_mainHLayout;
    QLabel *updateMessageLabel = nullptr;
    QLabel *noNewUpdateLabel = nullptr;
    QLabel *errorMessageLabel = nullptr;
    QLabel *restartEffectLabel = nullptr;
    QPushButton *m_pauseButton;
    QPushButton *m_closeButton;
    QProgressBar *m_progressBar;
    QToolButton *m_downloadButton = nullptr; // 下载

    int m_downloadProgress;
    UpdateState m_currentState;
    QString m_modelDebName;
    bool m_downloading;
    bool m_netStaus;
};

#endif // UPDATENOTIFICATIONWIDGET_H
