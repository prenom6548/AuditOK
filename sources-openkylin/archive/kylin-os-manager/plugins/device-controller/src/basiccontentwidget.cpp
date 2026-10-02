#include "basiccontentwidget.h"
#include <QDebug>
#include "include/globalsignal.h"

BasicContentWidget::BasicContentWidget(QWidget *parent)
    : QWidget(parent),
    m_menu(new QMenu(this)),
    m_actionExport(new QAction){

}

void BasicContentWidget::mousePressEvent(QMouseEvent *mouseEvent) {
    if (mouseEvent->button() == Qt::RightButton) {
        if (mouseEvent->type() == QEvent::MouseButtonPress) {
            qDebug() << "右键点击";
            rightClickedMenuRequest(); //右击弹窗:1另存为、2打开文件
        }
    }}

void BasicContentWidget::rightClickedMenuRequest() {
    m_actionExport->setText(tr("Export To Clipboard"));
    connect(m_actionExport, &QAction::triggered, g_signal, &GlobalSignal::exportBasicContentSignal);
    m_menu->addAction(m_actionExport);
    m_menu->exec(QCursor::pos());
}
