#include "treeitem.h"
#include "include/globalsignal.h"
#include <gsettingmonitor.h>
#include <QDebug>
#include "hwwidget.h"
#include "include/common.h"

TreeItem::TreeItem(QString name, QString content, QWidget *parent)
        : m_name(name),
          m_content(content),
          QWidget(parent),
          m_nameLabel(new QLabel()),
          m_contentLabel(new K::TextLabel()),
          m_layout(new QHBoxLayout()){
    m_parent = parent;
    initGui();
    initConnect();
    if (QGSettings::isSchemaInstalled(UKUI_THEME_GSETTING_PATH)) {
        m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    }}

void TreeItem::initGui() {
    m_nameLabel->setText(m_name);
    m_contentLabel->SetText(m_content);
    m_contentLabel->setMinimumWidth(200);
    m_nameLabel->setMinimumWidth(148);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addSpacing(28);
    m_layout->addWidget(m_nameLabel);
    m_layout->addWidget(m_contentLabel);
    m_layout->addStretch();
    this->setLayout(m_layout);
    this->setFixedHeight(40);
}

void TreeItem::setLabelText(QString text){
    m_contentLabel->SetText(text);
}

void TreeItem::initConnect() {
    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &TreeItem::setLabelFont);
}

void TreeItem::setLabelFont() {
    QString fontType = m_themeData->get("systemFont").toString();
    float fontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    QFont font(fontType, fontSize);
    m_nameLabel->setFont(font);
    m_contentLabel->setFont(font);
}

void TreeItem::mousePressEvent(QMouseEvent *mouseEvent) {
    if (mouseEvent->button() == Qt::RightButton) {
        if (mouseEvent->type() == QEvent::MouseButtonPress) {
            qDebug() << "右键点击";
            QMenu menu;
            QList<QAction *> actions = static_cast<HwWidget *>(m_parent)->m_menu->actions();
            for (int i = 0; i < actions.length(); ++i) {
                if (actions[i]->text() == tr("Export To Clipboard")){
                    continue;
                }
                menu.addAction(actions[i]);
            }
            QAction action;
            action.setText(tr("Export To Clipboard"));
            menu.addAction(&action);
            connect(&action, &QAction::triggered, [=](){
                static_cast<HwWidget *>(m_parent)->actionExportSlot();
            });
            menu.exec(QCursor::pos());
        }
    }
}
