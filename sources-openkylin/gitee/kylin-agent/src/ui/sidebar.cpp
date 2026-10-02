#include "ui/sidebar.h"

#include "ui/sessionlistwidget.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
    , m_sessionCountLabel(nullptr)
    , m_searchEdit(nullptr)
    , m_newSessionButton(nullptr)
    , m_sessionList(nullptr)
{
    setupUi();
}

Sidebar::~Sidebar() = default;

void Sidebar::setupUi()
{
    setObjectName(QStringLiteral("sidebar"));
    setStyleSheet(QStringLiteral(
        "#sidebar {"
        "  background: #eef2f6;"
        "  border-right: 1px solid #d8dde6;"
        "}"
    ));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    QLabel *title = new QLabel(QStringLiteral("Sessions"), this);
    title->setStyleSheet(QStringLiteral("font-weight: 600; color: #20242a;"));
    layout->addWidget(title);

    m_newSessionButton = new QPushButton(QStringLiteral("New Session"), this);
    m_newSessionButton->setCursor(Qt::PointingHandCursor);
    connect(m_newSessionButton, &QPushButton::clicked, this, &Sidebar::newSessionRequested);
    layout->addWidget(m_newSessionButton);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("Search sessions"));
    connect(m_searchEdit, &QLineEdit::textChanged, this, &Sidebar::searchChanged);
    layout->addWidget(m_searchEdit);

    m_sessionCountLabel = new QLabel(this);
    m_sessionCountLabel->setStyleSheet(QStringLiteral("color: #5c6675;"));
    layout->addWidget(m_sessionCountLabel);

    m_sessionList = new SessionListWidget(this);
    connect(m_sessionList, &SessionListWidget::sessionSelected, this, &Sidebar::sessionSelected);
    connect(m_sessionList, &SessionListWidget::sessionRenameRequested, this, &Sidebar::sessionRenameRequested);
    connect(m_sessionList, &SessionListWidget::sessionDeleteRequested, this, &Sidebar::sessionDeleteRequested);
    layout->addWidget(m_sessionList, 1);

    setSessionCount(0);
}

void Sidebar::setSessionCount(int count)
{
    if (!m_sessionCountLabel) {
        return;
    }

    m_sessionCountLabel->setText(QStringLiteral("%1 local sessions").arg(count));
}

void Sidebar::setSessions(const QVector<SessionSummary> &sessions)
{
    setSessionCount(sessions.size());
    if (m_sessionList) {
        m_sessionList->setSessions(sessions);
    }
}
