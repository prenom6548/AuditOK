#include "ui/mainwindow.h"

#include "services/apiservice.h"
#include "services/controlservice.h"
#include "services/databaseservice.h"
#include "services/sessionservice.h"
#include "ui/chatwidget.h"
#include "ui/settingsdialog.h"
#include "ui/sidebar.h"

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_titleBar(nullptr)
    , m_dragLabel(nullptr)
    , m_pinBtn(nullptr)
    , m_sizeGrip(nullptr)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_sidebar(nullptr)
    , m_chatWidget(nullptr)
    , m_settingsDialog(nullptr)
    , m_isPinned(false)
    , m_dragging(false)
    , m_quitting(false)
    , m_hasShownTrayCloseHint(false)
{
    setupUi();
    refreshSessionSummary();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("KylinAgent"));
    setMinimumSize(900, 640);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QWidget *titleBar = new QWidget(central);
    titleBar->setFixedHeight(40);
    titleBar->setObjectName(QStringLiteral("titleBar"));
    titleBar->setStyleSheet(QStringLiteral(
        "#titleBar {"
        "  background: #f7f8fa;"
        "  border-bottom: 1px solid #dfe3ea;"
        "}"
    ));

    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(10, 0, 8, 0);
    titleLayout->setSpacing(8);

    QPushButton *sidebarButton = createTitleButton(QStringLiteral("☰"), QStringLiteral("Toggle sidebar"));
    connect(sidebarButton, &QPushButton::clicked, this, &MainWindow::toggleSidebar);
    titleLayout->addWidget(sidebarButton);

    QLabel *title = new QLabel(QStringLiteral("KylinAgent"), titleBar);
    title->setStyleSheet(QStringLiteral("font-weight: 600; color: #20242a;"));
    titleLayout->addWidget(title);
    titleLayout->addStretch();

    QPushButton *settingsButton = createTitleButton(QStringLiteral("⚙"), QStringLiteral("Settings"));
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
    titleLayout->addWidget(settingsButton);

    QPushButton *minimizeButton = createTitleButton(QStringLiteral("_"), QStringLiteral("Minimize"));
    connect(minimizeButton, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    titleLayout->addWidget(minimizeButton);

    QPushButton *closeButton = createTitleButton(QStringLiteral("x"), QStringLiteral("Close"));
    connect(closeButton, &QPushButton::clicked, this, &QMainWindow::close);
    titleLayout->addWidget(closeButton);

    rootLayout->addWidget(titleBar);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, central);
    splitter->setChildrenCollapsible(false);

    m_sidebar = new Sidebar(splitter);
    m_sidebar->setMinimumWidth(220);
    m_sidebar->setMaximumWidth(360);
    connect(m_sidebar, &Sidebar::newSessionRequested, this, &MainWindow::createNewSession);
    connect(m_sidebar, &Sidebar::searchChanged, this, &MainWindow::searchSessions);
    connect(m_sidebar, &Sidebar::sessionSelected, this, &MainWindow::loadSession);
    connect(m_sidebar, &Sidebar::sessionRenameRequested, this, &MainWindow::renameSession);
    connect(m_sidebar, &Sidebar::sessionDeleteRequested, this, &MainWindow::deleteSession);
    splitter->addWidget(m_sidebar);

    m_chatWidget = new ChatWidget(splitter);
    auto *control = new ControlService([this](const QJsonObject &request) {
        return m_chatWidget->handleControlCommand(request);
    }, this);
    QTimer::singleShot(0, control, [control]() {
        if (!control->start()) {
            qWarning() << "Unable to start KylinAgent control interface:" << control->errorString();
        }
    });
    connect(m_chatWidget, &ChatWidget::messageSent, this, &MainWindow::refreshSessionSummary);
    splitter->addWidget(m_chatWidget);
    splitter->setSizes({260, 940});

    rootLayout->addWidget(splitter, 1);

    connect(&ApiService::instance(), &ApiService::connectedChanged,
            this, &MainWindow::showBackendConnectionState);
    connect(&ApiService::instance(), &ApiService::backendChanged, this, [this]() {
        statusBar()->showMessage(QStringLiteral("agent-rt backend: %1").arg(ApiService::instance().networkApiUrl()), 5000);
    });
    statusBar()->showMessage(QStringLiteral("agent-rt backend: %1").arg(ApiService::instance().networkApiUrl()));
    ApiService::instance().checkConnection();
    setupTrayIcon();
}

QPushButton *MainWindow::createTitleButton(const QString &text, const QString &toolTip)
{
    QPushButton *button = new QPushButton(text, this);
    button->setFixedSize(32, 28);
    button->setToolTip(toolTip);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: transparent;"
        "  border: 0;"
        "  border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "  background: #e3e8ef;"
        "}"
    ));
    return button;
}

void MainWindow::setupTrayIcon()
{
    if (m_trayIcon) {
        return;
    }

    m_trayIcon = new QSystemTrayIcon(QIcon(QStringLiteral(":/ai-agent.png")), this);
    m_trayIcon->setToolTip(QStringLiteral("KylinAgent"));

    m_trayMenu = new QMenu(this);
    QAction *showAction = m_trayMenu->addAction(QStringLiteral("显示窗口"));
    QAction *hideAction = m_trayMenu->addAction(QStringLiteral("隐藏窗口"));
    m_trayMenu->addSeparator();
    QAction *quitAction = m_trayMenu->addAction(QStringLiteral("退出"));

    connect(showAction, &QAction::triggered, this, &MainWindow::showFromTray);
    connect(hideAction, &QAction::triggered, this, [this]() {
        hide();
    });
    connect(quitAction, &QAction::triggered, this, []() {
        qApp->quit();
    });
    connect(m_trayIcon, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            toggleFromTaskbar();
        }
    });

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();
}

void MainWindow::showTrayMessage(const QString &title, const QString &message)
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 3000);
    }
}

void MainWindow::showFromTray()
{
    const bool restoreMaximized = windowState().testFlag(Qt::WindowMaximized);

    setWindowState(windowState() & ~Qt::WindowMinimized);
    if (restoreMaximized) {
        showMaximized();
    } else {
        showNormal();
    }

    const auto requestActivation = [this]() {
        show();
        raise();
        activateWindow();
        if (QWindow *handle = windowHandle()) {
            handle->requestActivate();
        }
    };

    requestActivation();
    QTimer::singleShot(50, this, requestActivation);
}

void MainWindow::toggleFromTaskbar()
{
    if (isVisible() && !isMinimized() && isActiveWindow()) {
        hide();
        return;
    }
    showFromTray();
}

void MainWindow::toggleSidebar()
{
    if (!m_sidebar) {
        return;
    }

    m_sidebar->setVisible(!m_sidebar->isVisible());
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::showBackendConnectionState(bool connected)
{
    statusBar()->showMessage(
        QStringLiteral("agent-rt %1: %2")
            .arg(connected ? QStringLiteral("connected") : QStringLiteral("unavailable"),
                 ApiService::instance().networkApiUrl()),
        5000
    );
}

void MainWindow::createNewSession()
{
    const QString sessionId = SessionService::instance().createSession(QStringLiteral("desktop"), QString());
    if (sessionId.isEmpty()) {
        if (m_chatWidget) {
            m_chatWidget->showStatus(QStringLiteral("Failed to create a local session."));
        }
        return;
    }

    refreshSessionSummary();
    loadSession(sessionId);
}

void MainWindow::loadSession(const QString &sessionId)
{
    SessionService::instance().setCurrentSession(sessionId);
    if (m_chatWidget) {
        m_chatWidget->loadSession(sessionId);
    }
}

void MainWindow::renameSession(const QString &sessionId)
{
    const SessionSummary summary = SessionService::instance().getSession(sessionId);
    bool ok = false;
    const QString title = QInputDialog::getText(
        this,
        QStringLiteral("Rename Session"),
        QStringLiteral("Session title:"),
        QLineEdit::Normal,
        summary.title(),
        &ok
    ).trimmed();

    if (ok && !title.isEmpty() && SessionService::instance().updateSessionTitle(sessionId, title)) {
        refreshSessionSummary();
    }
}

void MainWindow::deleteSession(const QString &sessionId)
{
    const int result = QMessageBox::question(
        this,
        QStringLiteral("Delete Session"),
        QStringLiteral("Delete this local session and its messages?")
    );
    if (result != QMessageBox::Yes) {
        return;
    }

    if (SessionService::instance().deleteSession(sessionId)) {
        refreshSessionSummary();
    }
}

void MainWindow::searchSessions(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        refreshSessionSummary();
        return;
    }

    QVector<SessionSummary> summaries;
    const QVector<SearchResult> results = SessionService::instance().searchSessions(text);
    for (const SearchResult &result : results) {
        SessionSummary summary;
        summary.setId(result.sessionId());
        summary.setTitle(result.title().isEmpty() ? result.snippet() : result.title());
        summary.setSource(result.source());
        summary.setStartedAt(result.startedAt());
        summary.setMessageCount(result.messageCount());
        summary.setModel(result.model());
        summary.setPreview(result.snippet());
        summaries.append(summary);
    }
    if (m_sidebar) {
        m_sidebar->setSessions(summaries);
    }
}

void MainWindow::refreshSessionSummary()
{
    DatabaseService::instance().initialize();

    const QVector<SessionSummary> sessions = SessionService::instance().listSessions();
    if (m_sidebar) {
        m_sidebar->setSessions(sessions);
    }

    if (!m_chatWidget) {
        return;
    }

    if (sessions.isEmpty()) {
        m_chatWidget->showStatus(QStringLiteral("No local sessions yet.\nDatabase: %1")
            .arg(DatabaseService::instance().databasePath()));
    } else if (m_chatWidget->currentSessionId().isEmpty() ||
               SessionService::instance().getSession(m_chatWidget->currentSessionId()).id().isEmpty()) {
        loadSession(sessions.first().id());
    }
}
