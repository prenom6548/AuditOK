#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include <QObject>
#include <QScreen>
#include <QHash>
#include <QList>
#include <QPoint>
#include <QSize>
#include <QRect>
#include <QTimeLine>
#include <QQuickView>
#include <QVariantMap>
#include <QTimeLine>

class ScreenManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QScreen* primaryScreen READ primaryScreen NOTIFY primaryScreenChanged)
    Q_PROPERTY(int screenCount READ screenCount NOTIFY screenCountChanged)
    Q_PROPERTY(int displayMode READ displayMode NOTIFY displayModeChanged)
    Q_PROPERTY(QRect virtualDesktopGeometry READ virtualDesktopGeometry NOTIFY virtualDesktopGeometryChanged)

public:
    // 显示模式
    enum DisplayMode {
        Unknown = 0,    // 未知模式
        Single = 1,     // 单屏模式
        Mirror = 2,     // 镜像模式
        Extend = 3      // 扩展模式
    };
    Q_ENUM(DisplayMode)

    static ScreenManager* getInstance();

    // 屏幕信息方法
    QScreen* primaryScreen() const;
    bool isPrimaryScreen(QScreen* screen) const;
    QList<QScreen*> allScreens() const;
    int screenCount() const;
    int displayMode() const;
    QRect virtualDesktopGeometry() const;

    // 根据ID获取屏幕信息
    Q_INVOKABLE QVariantMap getScreenInfo(int screenId) const;
    Q_INVOKABLE QScreen* getScreenById(int screenId) const;
    Q_INVOKABLE int getScreenId(QScreen* screen) const;
    Q_INVOKABLE int getPrimaryScreenId() const;

    // 屏幕位置和几何信息
    Q_INVOKABLE QRect getScreenGeometry(int screenId) const;
    Q_INVOKABLE QPoint getScreenPosition(int screenId) const;
    Q_INVOKABLE QSize getScreenSize(int screenId) const;
    Q_INVOKABLE qreal getScreenScaleFactor(int screenId) const;

    // 判断一个点在哪个屏幕上
    Q_INVOKABLE int getScreenIdAt(const QPoint& pos) const;
    Q_INVOKABLE QPoint mapToGlobal(int screenId, const QPoint& pos) const;
    Q_INVOKABLE QPoint mapFromGlobal(int screenId, const QPoint& pos) const;

    // 窗口管理
    Q_INVOKABLE void relocateWindows();

    // 屏幕模式管理
    Q_INVOKABLE void switchToSingleScreenMode();
    Q_INVOKABLE void switchToMultiScreenMode();
    Q_INVOKABLE int checkScreenMode(const QRect& geometry);

    void initScreens();
    bool updateDisplayMode();
    void updateVirtualDesktopGeometry();
    int generateScreenId();
    
    void addBackgroundWindow(QWindow *window, QScreen* screen, int id = -1);
    void removeBackgroundWindow(QWindow *window);
    QList<QWindow *> getBackgroundWindows() const;
    QWindow *getWindowAt(const QPoint& pos) const;
    QWindow *getWindowById(int screenId) const;

public Q_SLOTS:
    void handleScreenAdded(QScreen* screen);
    void handleScreenRemoved(QScreen* screen);
    void handlePrimaryScreenChanged(QScreen* screen);
    void handleScreenGeometryChanged(QScreen *screen, const QRect &geometry);

Q_SIGNALS:
    void primaryScreenChanged();
    void screenAdded(QScreen* screen, int id);
    void screenRemoved(QScreen* screen, int id);
    void screenRemovedFromExtend(QScreen* screen, int id);
    void screenCountChanged();
    void displayModeChanged();
    void virtualDesktopGeometryChanged();

    void windowsRelocateRequired();

    // 模式切换信号
    void switchDisplayModeActivated();
    void singleScreenModeActivated();
    void multiScreenModeActivated();

    // 窗口管理信号
    //void windowAdded(QQuickView* view, int screenId);
    //void windowRemoved(QQuickView* view, int screenId);

    void windowAdded(QObject* window, int screenId);
    void windowRemoved(QObject* window, int screenId);

    void resetWindowIds(QScreen* oldScreen, int oldId, QScreen* newScreen, int newId);

private:
    explicit ScreenManager(QObject *parent = nullptr);
    ~ScreenManager();

    static ScreenManager* m_instance;

    QHash<QScreen*, int> m_screenIds;
    QHash<int, QScreen*> m_screens;
    QHash<int, QWindow*> m_backgroundWindows;

    DisplayMode m_displayMode;
    QRect m_virtualDesktopGeometry;

    // 上次分配的ID
    int m_lastScreenId;
};

#define SCREEN_MANAGER ScreenManager::getInstance()

#endif // SCREENMANAGER_H
