/*
 * Minimal stub for MultitaskViewManagerByWlcom to enable unit testing
 * This file provides a minimal interface matching MultitaskViewManagerByWlcom
 * without requiring the full QML initialization
 */

#ifndef MINIMAL_MULTITASKVIEWMANAGER_H
#define MINIMAL_MULTITASKVIEWMANAGER_H

#include <QObject>
#include <QString>
#include <QRect>
#include <QScreen>

// Minimal stub class that mimics MultitaskViewManagerByWlcom interface
// This is used only for unit testing to avoid QML/display initialization
class MultitaskViewManagerByWlcom : public QObject
{
    Q_OBJECT

public:
    explicit MultitaskViewManagerByWlcom(QObject *parent = nullptr)
        : QObject(parent), m_openCalled(false), m_closeCalled(false) {}

    ~MultitaskViewManagerByWlcom() override = default;

    // Mirrored interface from real MultitaskViewManagerByWlcom
    Q_INVOKABLE void open() {
        m_openCalled = true;
    }

    Q_INVOKABLE void close() {
        m_closeCalled = true;
    }

    Q_INVOKABLE void openTab() {}
    Q_INVOKABLE void openBackTab() {}
    Q_INVOKABLE void setShowingDesktop(bool) {}
    Q_INVOKABLE bool isMultitaskViewVerticalLayout() const { return false; }
    Q_INVOKABLE bool shouldShowTabBorder() const { return false; }
    Q_INVOKABLE void setShouldShowTabBorder(bool) {}
    Q_INVOKABLE void activeAreaChangeDataCollect(const QString&) {}
    Q_INVOKABLE bool isWlcom() const { return true; }
    Q_INVOKABLE bool isLayoutLeftToRight() { return true; }
    Q_INVOKABLE bool isAltKeyPressed() { return false; }
    Q_INVOKABLE void keyNavigationDataCollect(const int&) {}

    QScreen* getCurrentScreen() { return nullptr; }
    QRect currentScreenRect() { return QRect(); }
    bool isBackTab() { return false; }
    bool altTabVisible() { return false; }
    bool winTabVisible() { return false; }
    int desktopTipPos() { return 0; }

    // Test helper methods
    bool wasOpenCalled() const { return m_openCalled; }
    bool wasCloseCalled() const { return m_closeCalled; }
    void reset() { m_openCalled = false; m_closeCalled = false; }

private:
    bool m_openCalled;
    bool m_closeCalled;
};

#endif // MINIMAL_MULTITASKVIEWMANAGER_H
