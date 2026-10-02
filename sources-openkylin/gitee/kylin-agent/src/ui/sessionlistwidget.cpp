#include "ui/sessionlistwidget.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QListWidgetItem>
#include <QMenu>

SessionListWidget::SessionListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setAlternatingRowColors(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSpacing(2);
    setStyleSheet(QStringLiteral(
        "QListWidget {"
        "  background: transparent;"
        "  border: 0;"
        "  outline: 0;"
        "}"
        "QListWidget::item {"
        "  padding: 8px 10px;"
        "  border-radius: 5px;"
        "}"
        "QListWidget::item:selected {"
        "  background: #dfeeff;"
        "  color: #175fa8;"
        "}"
        "QListWidget::item:hover {"
        "  background: #e8eef6;"
        "}"
    ));

    connect(this, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (item) {
            emit sessionSelected(item->data(Qt::UserRole).toString());
        }
    });
}

SessionListWidget::~SessionListWidget() = default;

void SessionListWidget::setSessions(const QVector<SessionSummary> &sessions)
{
    clear();
    for (const SessionSummary &session : sessions) {
        addSession(session.id(), session.title());
    }
}

void SessionListWidget::addSession(const QString &id, const QString &title)
{
    QListWidgetItem *item = new QListWidgetItem(fallbackTitle(title, id), this);
    item->setData(Qt::UserRole, id);
    item->setToolTip(id);
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void SessionListWidget::updateSessionTitle(const QString &id, const QString &title)
{
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *entry = item(i);
        if (entry && entry->data(Qt::UserRole).toString() == id) {
            entry->setText(fallbackTitle(title, id));
            return;
        }
    }
}

QString SessionListWidget::firstSessionId() const
{
    const QListWidgetItem *entry = item(0);
    return entry ? entry->data(Qt::UserRole).toString() : QString();
}

void SessionListWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QListWidgetItem *entry = itemAt(event->pos());
    if (!entry) {
        return;
    }

    const QString sessionId = entry->data(Qt::UserRole).toString();
    QMenu menu(this);
    QAction *renameAction = menu.addAction(QStringLiteral("Rename"));
    QAction *deleteAction = menu.addAction(QStringLiteral("Delete"));

    QAction *selected = menu.exec(event->globalPos());
    if (selected == renameAction) {
        emit sessionRenameRequested(sessionId);
    } else if (selected == deleteAction) {
        emit sessionDeleteRequested(sessionId);
    }
}

QString SessionListWidget::fallbackTitle(const QString &title, const QString &id) const
{
    if (!title.trimmed().isEmpty()) {
        return title.trimmed();
    }

    return id.isEmpty() ? QStringLiteral("New Session") : id.left(8);
}
