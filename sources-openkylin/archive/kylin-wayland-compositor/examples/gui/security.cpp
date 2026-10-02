// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QPlainTextEdit>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>

#include <qpa/qplatformnativeinterface.h>

#include "kywc-security-v1-client-protocol.h"

class Security : public QCheckBox
{
    Q_OBJECT
  public:
    Security(QWidget *parent = nullptr, QScreen *screen = nullptr, wl_display *display = nullptr,
             kywc_security_manager_v1 *manager = nullptr)
        : QCheckBox(parent), wlDisplay(display)
    {
        setChecked(true);
        setText(screen->name());

        wl_output *output = reinterpret_cast<wl_output *>(
            QGuiApplication::platformNativeInterface()->nativeResourceForScreen(
                QByteArrayLiteral("output"), screen));
        if (!output) {
            qDebug() << "cannot get wl_output for" << screen->name();
            return;
        }

        security = kywc_security_manager_v1_get_security_for_output(manager, output);
        kywc_security_v1_add_listener(security, &security_listener, this);
        wl_display_flush(wlDisplay);

        connect(this, &QCheckBox::toggled, this, [&]() {
            isChecked() ? kywc_security_v1_enable(security) : kywc_security_v1_disable(security);
            wl_display_sync(wlDisplay);
        });

        qDebug() << "security created for" << text();
    }

    Security(QWidget *parent = nullptr, QWindow *window = nullptr, wl_display *display = nullptr,
             kywc_security_manager_v1 *manager = nullptr)
        : QCheckBox(parent), wlDisplay(display)
    {
        setChecked(true);
        setText("surface");

        wl_surface *surface = reinterpret_cast<wl_surface *>(
            QGuiApplication::platformNativeInterface()->nativeResourceForWindow(
                QByteArrayLiteral("surface"), window));
        if (!surface) {
            qDebug() << "cannot get wl_surface for window";
            return;
        }

        security = kywc_security_manager_v1_get_security_for_surface(manager, surface);
        kywc_security_v1_add_listener(security, &security_listener, this);
        wl_display_flush(wlDisplay);

        connect(this, &QCheckBox::toggled, this, [&]() {
            isChecked() ? kywc_security_v1_enable(security) : kywc_security_v1_disable(security);
            wl_display_sync(wlDisplay);
        });

        qDebug() << "security created for" << text();
    }

    ~Security()
    {
        if (security) {
            kywc_security_v1_destroy(security);
            wl_display_sync(wlDisplay);
        }
    }

  signals:
    void stateChanged(QString name, bool enabled);
    void touched(QString name, QString toucher);
    void aboutToDestroy(QString name);

  private:
    wl_display *wlDisplay = nullptr;
    kywc_security_v1 *security = nullptr;

    static void security_handle_failed(void *data, struct kywc_security_v1 *kywc_security_v1)
    {
        Security *security = static_cast<Security *>(data);
        kywc_security_v1_destroy(security->security);
        security->security = nullptr;
        emit security->aboutToDestroy(security->text());
        security->deleteLater();
    };

    static void security_handle_cancelled(void *data, struct kywc_security_v1 *kywc_security_v1)
    {
        Security *security = static_cast<Security *>(data);
        kywc_security_v1_destroy(security->security);
        security->security = nullptr;
        emit security->aboutToDestroy(security->text());
        security->deleteLater();
    };

    static void security_handle_enabled(void *data, struct kywc_security_v1 *kywc_security_v1,
                                        uint32_t enabled)
    {
        Security *security = static_cast<Security *>(data);
        emit security->stateChanged(security->text(), enabled);
    };

    static void security_handle_touched(void *data, struct kywc_security_v1 *kywc_security_v1,
                                        const char *toucher)
    {
        Security *security = static_cast<Security *>(data);
        emit security->touched(security->text(), toucher);
    };

    struct kywc_security_v1_listener security_listener = {
        security_handle_failed,
        security_handle_cancelled,
        security_handle_enabled,
        security_handle_touched,
    };
};

class Widget : public QWidget
{
  public:
    Widget(QWidget *parent = nullptr, wl_display *display = nullptr,
           kywc_security_manager_v1 *manager = nullptr)
        : QWidget(parent), wlDisplay(display), securityManager(manager)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        securityLayout = new QVBoxLayout;
        layout->addLayout(securityLayout);

        // log message
        logOutput = new QPlainTextEdit;
        logOutput->setReadOnly(true);
        layout->addWidget(logOutput);

        QList<QScreen *> screens = QGuiApplication::screens();
        foreach(QScreen * screen, screens)
        {
            outputSecurityCreate(screen);
        }

        connect(qApp, &QGuiApplication::screenAdded, this,
                [&](QScreen *screen) { onScreenChanged(screen, true); });
        connect(qApp, &QGuiApplication::screenRemoved, this,
                [&](QScreen *screen) { onScreenChanged(screen, false); });
    }

    ~Widget()
    {
        kywc_security_manager_v1_destroy(securityManager);
        wl_display_flush(wlDisplay);
    }

    void connectSignals(Security *security)
    {
        connect(security, &Security::stateChanged, this, [&](QString name, bool enabled) {
            logOutput->appendPlainText(QString("security %1 state %2").arg(name).arg(enabled));
        });

        connect(security, &Security::touched, this, [&](QString name, QString toucher) {
            logOutput->appendPlainText(QString("security %1 touched by %2").arg(name).arg(toucher));
        });

        connect(security, &Security::aboutToDestroy, this, [&](QString name) {
            logOutput->appendPlainText(QString("security %1 destroying").arg(name));
        });
    };

  protected:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::PlatformSurface) {
            QPlatformSurfaceEvent *pe = static_cast<QPlatformSurfaceEvent *>(event);
            if (pe->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
                Security *security = new Security(this, windowHandle(), wlDisplay, securityManager);
                securityLayout->addWidget(security);
                connectSignals(security);
            }
        }

        return QWidget::event(event);
    }

  private:
    wl_display *wlDisplay = nullptr;
    kywc_security_manager_v1 *securityManager = nullptr;

    QPlainTextEdit *logOutput = nullptr;
    QVBoxLayout *securityLayout = nullptr;

    void outputSecurityCreate(QScreen *screen)
    {
        Security *security = new Security(this, screen, wlDisplay, securityManager);
        securityLayout->addWidget(security);
        connectSignals(security);
    };

    void onScreenChanged(QScreen *screen, bool added)
    {
        qDebug() << screen << (added ? "added" : "removed");
        if (added) {
            outputSecurityCreate(screen);
        }
    };
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                                   const char *interface, uint32_t version)
{
    kywc_security_manager_v1 **manager = static_cast<kywc_security_manager_v1 **>(data);

    if (strcmp(interface, kywc_security_manager_v1_interface.name) == 0) {
        *manager = static_cast<kywc_security_manager_v1 *>(
            wl_registry_bind(registry, name, &kywc_security_manager_v1_interface, 1));
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t id)
{
    // do nothing
}

static struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "wayland");
    QApplication app(argc, argv);

    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    if (!native) {
        return -1;
    }
    wl_display *display = reinterpret_cast<wl_display *>(
        native->nativeResourceForIntegration(QByteArrayLiteral("wl_display")));
    if (!display) {
        qWarning("Failed to get Wayland display.");
        return -1;
    }
    qDebug() << "wayland display" << display;

    kywc_security_manager_v1 *manager = nullptr;
    wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, &manager);
    wl_display_dispatch(display);
    wl_display_roundtrip(display);
    wl_registry_destroy(registry);

    if (!manager) {
        qWarning("Failed to found kywc security protocol.");
        return -1;
    }
    qDebug() << "kywc security manager" << manager;

    Widget widget(nullptr, display, manager);
    widget.show();

    return app.exec();
}

#include "security.moc"
