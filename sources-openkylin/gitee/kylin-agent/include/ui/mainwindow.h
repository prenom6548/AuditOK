#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QWidget>
#include <QMouseEvent>
#include <QString>
#include <QPushButton>
#include <QSettings>
#include <QScreen>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QTimer>
#include <QRect>
#include <QSize>
#include <QFlags>
#include <QSplitter>
#include <QSystemTrayIcon>

class Sidebar;
class ChatWidget;
class SettingsDialog;
class QSizeGrip;
class QCloseEvent;
class QMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showFromTray();
    void toggleFromTaskbar();

protected:

private slots:
    void toggleSidebar();
    void openSettings();
    void showBackendConnectionState(bool connected);
    void createNewSession();
    void loadSession(const QString &sessionId);
    void renameSession(const QString &sessionId);
    void deleteSession(const QString &sessionId);
    void searchSessions(const QString &text);
    void refreshSessionSummary();
    
private:
    void setupUi();
    QPushButton *createTitleButton(const QString &text, const QString &toolTip);
    void setupTrayIcon();
    void showTrayMessage(const QString &title, const QString &message);
    void selectFallbackSession();
    bool confirmQuitWithActiveDownloads();
    void beginWindowDrag(const QPoint &globalPos, const QPoint &localPos);
    void dragWindowTo(const QPoint &globalPos);

    QWidget *m_centralWidget;
    QWidget *m_titleBar;
    QWidget *m_dragLabel;
    QPushButton *m_pinBtn;
    QSizeGrip *m_sizeGrip;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    Sidebar *m_sidebar;
    ChatWidget *m_chatWidget;
    SettingsDialog *m_settingsDialog;
    QPoint m_dragPosition;
    QSize m_dragWindowSize;
    QSize m_dragOriginalMinimumSize;
    QSize m_dragOriginalMaximumSize;
    bool m_isPinned;
    bool m_dragging;
    bool m_quitting;
    bool m_hasShownTrayCloseHint;
};

#endif // MAINWINDOW_H
