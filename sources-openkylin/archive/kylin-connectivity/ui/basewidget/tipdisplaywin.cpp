#include "tipdisplaywin.h"
#include <QDebug>

TipDisplayWin::TipDisplayWin(QWidget *parent) : QWidget(parent)
{
    setAcceptDrops(true);
}

void TipDisplayWin::setAllowDrag(bool isAllowDrag)
{
    m_isAllowDrag = isAllowDrag;
}

void TipDisplayWin::dragEnterEvent(QDragEnterEvent *event)
{
    if (m_isAllowDrag) {
        event->accept();
    } else {
        event->ignore();
    }
}

void TipDisplayWin::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void TipDisplayWin::dropEvent(QDropEvent *event)
{
    auto urls = event->mimeData()->urls();
    qInfo() << "drag urls" << urls;
    if (urls.isEmpty()) {
        return;
    }

    QStringList localpath;
    for (auto &url : urls) {
        localpath << url.toLocalFile();
    }
    sigUploadFileInfo("", localpath);
}