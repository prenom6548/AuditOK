#include "ui/settingsdialog.h"

#include "services/apiservice.h"
#include "services/configservice.h"
#include "ui/modelsettingsdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>
#include <QSizeGrip>
#include <QVBoxLayout>

SettingsTitleBar::SettingsTitleBar(QWidget *parent)
    : QWidget(parent)
{
    setCursor(Qt::SizeAllCursor);
    setMinimumHeight(36);
}

void SettingsTitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit dragPressed(event->globalPos());
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void SettingsTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        emit dragMoved(event->globalPos());
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void SettingsTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit dragReleased();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_backendEdit(nullptr)
    , m_saveButton(nullptr)
{
    setupUi();
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::setupUi()
{
    setWindowTitle(QStringLiteral("Settings"));
    setMinimumWidth(460);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(14);

    QLabel *backendTitle = new QLabel(QStringLiteral("agent-rt Backend"), this);
    backendTitle->setStyleSheet(QStringLiteral("font-weight: 600;"));
    root->addWidget(backendTitle);

    QFormLayout *form = new QFormLayout;
    m_backendEdit = new QLineEdit(ConfigService::instance().agentRuntimeHost(), this);
    m_backendEdit->setPlaceholderText(QStringLiteral("127.0.0.1:8642"));
    form->addRow(QStringLiteral("Host:"), m_backendEdit);
    root->addLayout(form);

    QHBoxLayout *backendButtons = new QHBoxLayout;
    backendButtons->addStretch();
    m_saveButton = new QPushButton(QStringLiteral("Save Backend"), this);
    connect(m_saveButton, &QPushButton::clicked, this, &SettingsDialog::saveBackend);
    backendButtons->addWidget(m_saveButton);
    root->addLayout(backendButtons);

    QLabel *modelTitle = new QLabel(QStringLiteral("Models"), this);
    modelTitle->setStyleSheet(QStringLiteral("font-weight: 600; margin-top: 10px;"));
    root->addWidget(modelTitle);

    QPushButton *modelsButton = new QPushButton(QStringLiteral("Manage Models"), this);
    connect(modelsButton, &QPushButton::clicked, this, &SettingsDialog::openModelSettings);
    root->addWidget(modelsButton);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);
}

void SettingsDialog::saveBackend()
{
    const QString host = m_backendEdit->text().trimmed();
    if (host.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Settings"),
                             QStringLiteral("Backend host cannot be empty."));
        return;
    }

    ApiService::instance().setApiUrl(host);
    ApiService::instance().checkConnection();
    m_backendEdit->setText(ConfigService::instance().agentRuntimeHost());
}

void SettingsDialog::openModelSettings()
{
    ModelSettingsDialog dialog(this);
    dialog.exec();
}

void SettingsDialog::showFromOwner(const QRect &ownerGeometry)
{
    const QRect current = geometry();
    bool visibleOnScreen = false;
    for (QScreen *screen : QApplication::screens()) {
        if (screen && current.intersects(screen->availableGeometry())) {
            visibleOnScreen = true;
            break;
        }
    }

    if (!visibleOnScreen) {
        QScreen *primary = QApplication::primaryScreen();
        const QPoint center = ownerGeometry.isValid()
            ? ownerGeometry.center()
            : (primary ? primary->availableGeometry().center() : QPoint(0, 0));
        move(center - QPoint(width() / 2, height() / 2));
    }

    show();
    raise();
    activateWindow();
}

void SettingsDialog::setupUI()
{
    setupUi();
}

void SettingsDialog::setupConnections()
{
}

void SettingsDialog::updateTitleBarStyle()
{
    if (m_titleBar) {
        m_titleBar->setStyleSheet(QStringLiteral("background: #f7f8fa;"));
    }
}

void SettingsDialog::positionSizeGrip()
{
    if (!m_sizeGrip) {
        return;
    }
    const int margin = 2;
    m_sizeGrip->move(width() - m_sizeGrip->width() - margin,
                     height() - m_sizeGrip->height() - margin);
}

void SettingsDialog::beginWindowDrag(const QPoint &globalPos)
{
    m_dragging = true;
    m_dragPosition = globalPos - frameGeometry().topLeft();
}

void SettingsDialog::dragWindowTo(const QPoint &globalPos)
{
    if (!m_dragging) {
        return;
    }
    move(globalPos - m_dragPosition);
}

void SettingsDialog::setDragBypassEnabled(bool enabled)
{
    m_dragBypassActive = enabled;
}
