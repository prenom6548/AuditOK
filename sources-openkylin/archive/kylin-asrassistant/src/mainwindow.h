#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QTimer>
#include <QPaintEvent>
#include <QSystemTrayIcon>
#include <QDBusInterface>
#include <QNetworkConfigurationManager>
#include <QMenu>
#include <QAction>
#include <QGSettings>
#include <QThread>
#include <kaboutdialog.h>
#include "publicattributes.hpp"
#include "loadanimation.h"
#include "shortcutwin.h"
#include "speechrecognitionclient.h"
#include "speechinput.h"
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    static MainWindow *getInstance();

    void showView();

private:
    MainWindow(QWidget *parent = nullptr);

    void initService();
    void initUI();
    void initTrayIcon();
    void initGsettings();
    void resetWindowPosition();
    void hideView();
    void starServer();
    void stopServer();
    void deleteServer();

protected:
    void paintEvent(QPaintEvent *event);
    void focusOutEvent(QFocusEvent *event);

public Q_SLOTS:
    void onServerStatus(SpeechRecognitionClient::ServerState state);
    void onRecvMsg(const QString &msg);
    void onAudioInfo(const QByteArray &data);
    void onOnlineStateChanged(bool isOnline);

    void timeout();
    void onModeChanged(bool isModel);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    // void onShotCutActionTriggled();
    // void onAboutActionTriggled();
    // void onExitActionTriggled();
    void changeTheme();
    void changeFontSize();
    void changeTransparency();
    void onAnimationClicked();

private:
    SpeechRecognitionClient *m_client = nullptr;
    SpeechInput *m_audioInput = nullptr;
    QThread *m_audioThread = nullptr;
    QTimer *m_timer = nullptr;                      // 语音识别计时
    QGSettings *m_fontData = nullptr;               // 字体变化
    QDBusInterface *m_modeDbusInterface = nullptr;  // 平板模式切换
    QDBusInterface *m_positionInterface = nullptr;  // 获取任务栏位置
    QNetworkConfigurationManager *m_networkManager = nullptr;

    // QAction *m_shotcutAction = nullptr;
    // QAction *m_aboutAction = nullptr;
    // QAction *m_exitAction = nullptr;
    // QPushButton *m_closeBtn = nullptr;
    QLabel *m_title = nullptr;
    QListWidget *m_labList = nullptr;
    QTextEdit *m_resultDisplay = nullptr;
    LoadAnimation *m_animation = nullptr;
    QLabel *m_pintOutLab = nullptr;
    QSystemTrayIcon *m_trayIcon = nullptr;
    // QMenu *m_trayIconMenu = nullptr;
    // kdk::KAboutDialog *m_aboutWindow = nullptr;
    // ShortCutWin *m_shortCutWin = nullptr;

    QString m_lab1 = tr("\"Open the App Store\"");
    QString m_lab2 = tr("\"Open system settings\"");
    QString m_lab3 = tr("\"Open the browser\"");
    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light;
    double m_opacity = 0.75;
    PublicAttributes::Model m_model = PublicAttributes::Model::PcModel;
    bool m_isStartServer = false;
    bool m_isInit = true;
    QString m_oldResults = "";
    bool m_isStartTimer = false;
};
#endif  // MAINWINDOW_H
