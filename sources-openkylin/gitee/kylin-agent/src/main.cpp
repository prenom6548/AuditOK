#include <QApplication>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QIcon>

#include <cstdio>
#include <cstdlib>

#include "app.h"
#include "services/hermesgatewayservice.h"
#include "ui/mainwindow.h"
#include "utils/thememanager.h"

namespace {

bool hasCommandLineOption(const QStringList &arguments, const QString &option)
{
    return arguments.contains(option);
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    const QString category = QString::fromLatin1(context.category ? context.category : "");
    if (category == QStringLiteral("qt.qpa.xcb") &&
        message.startsWith(QStringLiteral("QXcbConnection: XCB error:"))) {
        return;
    }

    const QByteArray text = message.toLocal8Bit();
    const char *level = "Debug";
    switch (type) {
    case QtDebugMsg:
        level = "Debug";
        break;
    case QtInfoMsg:
        level = "Info";
        break;
    case QtWarningMsg:
        level = "Warning";
        break;
    case QtCriticalMsg:
        level = "Critical";
        break;
    case QtFatalMsg:
        level = "Fatal";
        break;
    }

    std::fprintf(stderr, "%s: %s\n", level, text.constData());
    if (type == QtFatalMsg) {
        std::abort();
    }
}

} // namespace

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);

    // Enable high DPI scaling
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    App app(argc, argv);
    if (!app.isPrimaryInstance()) {
        return 0;
    }
    if (hasCommandLineOption(app.arguments(), QStringLiteral("--quit")) ||
        hasCommandLineOption(app.arguments(), QStringLiteral("--hide"))) {
        return 0;
    }
    
    app.setApplicationName("KylinAgent");
    app.setApplicationDisplayName("KylinAgent");
    app.setDesktopFileName("kylin-agent");
    app.setApplicationVersion(QStringLiteral(KYLIN_AGENT_VERSION));
    app.setOrganizationName("KylinAgent");
    app.setOrganizationDomain("kylin-agent.local");
    
    // Set high DPI icon
    app.setWindowIcon(QIcon(":/ai-agent.png"));
    app.setQuitOnLastWindowClosed(false);
    
    ThemeManager::instance().applySavedTheme(&app);

    HermesGatewayService hermesGatewayService(&app);
    hermesGatewayService.ensureReady();
    
    MainWindow window;
    window.setWindowTitle("KylinAgent");
    window.resize(1200, 800);
    window.show();

    QObject::connect(&app, &App::messageReceived, &window, [&window](const QString &message) {
        qDebug() << "KylinAgent handling command:" << message;
        if (message == QStringLiteral("activate")) {
            window.showFromTray();
        } else if (message == QStringLiteral("toggle")) {
            window.toggleFromTaskbar();
        } else if (message == QStringLiteral("hide")) {
            window.showMinimized();
        } else if (message == QStringLiteral("quit")) {
            window.close();
        }
    });
    
    return app.exec();
}
