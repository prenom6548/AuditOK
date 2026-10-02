#include "dialogmigrationlogs.h"
#include <QListView>
#include <QLayout>
#include <QDir>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QSizePolicy>
#include <Qt>
#include <gsettingmonitor.h>
#include <QDesktopServices>

DialogMigrationLogs *DialogMigrationLogs::m_instance = nullptr;
DialogMigrationLogsChild *DialogMigrationLogsChild::m_instance = nullptr;
DialogHowToUse *DialogHowToUse::m_instance = nullptr;

DialogMigrationLogs *DialogMigrationLogs::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new DialogMigrationLogs;
    }
    return m_instance;
}

DialogMigrationLogsChild *DialogMigrationLogsChild::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new DialogMigrationLogsChild;
    }
    return m_instance;
}

DialogHowToUse *DialogHowToUse::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new DialogHowToUse;
    }
    return m_instance;
}

void DialogMigrationLogsChild::updateAndActive(QString fileName)
{
    QFile file(m_logPath + fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_textEdit->setText(Constant::g_readLogErrorText + fileName);
        return;
    }
    QString log = file.readAll();
    if (log.endsWith("\n")) {
        log.chop(1);
    }
    file.close();
    m_textEdit->setText(log);
    activateWindow();
    show();
}

void DialogMigrationLogs::updateAndActive(QString fileName)
{
    Q_UNUSED(fileName);
    updateViewPrivate();
    activateWindow();
    show();
}

void DialogMigrationLogs::updateView()
{
    if (isHidden()) {
        return;
    }
    updateViewPrivate();
}

void DialogMigrationLogs::updateViewPrivate()
{
    m_model->clear();
    QStringList list = QDir(m_logPath).entryList(QStringList() << "*.log", QDir::Files, QDir::Name | QDir::Reversed);
    for (QString str : list) {
        QStandardItem *item = new QStandardItem;
        item->setData(str);
        QString itemText = "    " + Constant::g_migrationLogText + " " + str.remove(".log");
        item->setText(itemText);
        item->setEditable(false);
        item->setSelectable(false);
        item->setSizeHint(QSize(0, 36));
        // item->setForeground(QBrush(QPalette().link()));
        m_model->appendRow(item);
    }
}

void DialogHowToUse::updateAndActive(QString fileName)
{
    Q_UNUSED(fileName);
    activateWindow();
    show();
}

void DialogHowToUse::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
//     m_labelPix->setPixmap(m_pix.scaled(m_labelPix->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}


DialogMigrationLogsChild::DialogMigrationLogsChild(QWidget *parent) : DialogMigrationLogsCommon(parent)
{
    QVBoxLayout *vl = new QVBoxLayout;
    vl->setContentsMargins(24, 8, 24, 24);
    m_textEdit = new QTextEdit;
    m_textEdit->setReadOnly(true);
    m_textEdit->setFrameStyle(QFrame::NoFrame);
    vl->addWidget(m_textEdit);
    baseBar()->setLayout(vl);
}

DialogHowToUse::DialogHowToUse(QWidget *parent) : DialogMigrationLogsCommon(parent)
{
//     setMaximumWidth(16777215);
//     setMaximumHeight(16777215);
//     setMinimumWidth(896);
//     setMinimumHeight(568);
    setFixedSize(896, 568);
//     windowButtonBar()->maximumButton()->show();
//     connect(windowButtonBar()->maximumButton(), &QPushButton::clicked, this, [=] {
//         if (windowButtonBar()->maximumButtonState() == kdk::Maximum) {
//             showNormal();
//         } else {
//             showMaximized();
//         }
//     });
    QVBoxLayout *vl = new QVBoxLayout;
    vl->setContentsMargins(24, 8, 24, 24);
    QString str = Constant::g_howToUseText;
    m_textEdit = new QTextBrowser;
    m_textEdit->setMinimumWidth(848);
    m_textEdit->setMaximumHeight(568);
    m_textEdit->setOpenLinks(false);
    m_textEdit->append(str);
    m_textEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_textEdit->setFrameShape(QFrame::NoFrame); // 去除边框
    m_textEdit->moveCursor(QTextCursor::Start); // 顶部显示
    m_textEdit->setReadOnly(true);              // 不可编辑
    setPixmap();
    QString path = QApplication::applicationDirPath() + "/howtouse.html";
    m_textEdit->append("<a href='"+ path +"'> <img src=\"mydata://howToUseImage.png\" /> </a> ");
    connect(m_textEdit, &QTextBrowser::anchorClicked,[=](QUrl url){
        QDesktopServices::openUrl(QUrl(url));
    });
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &DialogHowToUse::setPixmap);
    vl->addWidget(m_textEdit);
    baseBar()->setLayout(vl);
}

void DialogHowToUse::setPixmap()
{
    QString theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
    if (theme == "ukui-default" || theme == "ukui-light") {
        m_pix = QPixmap(":/res/howtouse.png");
    } else {
       m_pix = QPixmap(":/res/howtouseblack.png");
    }
    m_textEdit->document()->addResource(QTextDocument::ImageResource, QUrl("mydata://howToUseImage.png"), QVariant(m_pix.scaled(835, 310, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

DialogMigrationLogs::DialogMigrationLogs(QWidget *parent) : DialogMigrationLogsCommon(parent)
{
    QVBoxLayout *vl = new QVBoxLayout;
    vl->setContentsMargins(24, 8, 24, 24);
    QListView *tableView = new QListView;
    connect(tableView, &QListView::clicked, this, &DialogMigrationLogs::onTreeViewItemClick);
    tableView->setAlternatingRowColors(true);
    m_model = new QStandardItemModel;
    tableView->setModel(m_model);
    m_deleget = new DialogViewDelegate();
    tableView->setItemDelegate(m_deleget);
    vl->addWidget(tableView);
    baseBar()->setLayout(vl);
}

void DialogMigrationLogs::onTreeViewItemClick(const QModelIndex &index)
{
    DialogMigrationLogsChild::getInstance()->updateAndActive(m_model->itemFromIndex(index)->data().toString());
}

DialogMigrationLogsCommon::DialogMigrationLogsCommon(QWidget *parent) : kdk::KWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::Dialog);
    setWindowModality(Qt::ApplicationModal);
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_ShowModal,true);
    
    setLayoutType(kdk::VerticalType);
    setIcon(QIcon::fromTheme(("kylin-win-data-migration")));
    setWidgetName(Constant::g_toolbarText);
    setFixedWidth(700);
    setFixedHeight(450);
    windowButtonBar()->maximumButton()->hide();
    windowButtonBar()->menuButton()->hide();

    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    move(availableGeometry.width() / 2 - width() / 2, availableGeometry.height() / 2 - height() / 2);
}
