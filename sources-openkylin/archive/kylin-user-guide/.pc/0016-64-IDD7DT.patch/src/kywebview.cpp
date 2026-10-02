#include "kywebview.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QAction>
#include <QMenu>

KyWebView::KyWebView(QWidget *parent)
    : QWebEngineView(parent)
{
}

void KyWebView::dragEnterEvent(QDragEnterEvent *event)
{
    event->ignore();
}

void KyWebView::dropEvent(QDropEvent *event)
{
    event->ignore();
}

void KyWebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);

    QAction *copyAction = new QAction(QIcon::fromTheme("edit-copy-symbolic"), tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, [this]() { this->page()->triggerAction(QWebEnginePage::Copy); });
    menu->addAction(copyAction);

    QAction *copyAllAction = new QAction(tr("Select All"), this);
    copyAllAction->setShortcut(QKeySequence::SelectAll);
    connect(copyAllAction, &QAction::triggered, [this]() { this->page()->triggerAction(QWebEnginePage::SelectAll); this->page()->triggerAction(QWebEnginePage::Copy); });
    menu->addAction(copyAllAction);

    menu->exec(event->globalPos());
    delete menu;
}
