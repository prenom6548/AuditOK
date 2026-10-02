#include "widgetmigration.h"
#include <QLayout>
#include <QFileDialog>
#include "kom-buriedpoint.h"
WidgetMigration::WidgetMigration(QWidget *parent) : WidgetCommon(parent)
{
    initUI();
    initSavePath();
    initDialog();
}

void WidgetMigration::setModel(QStandardItemModel *itemModel)
{
    if (m_itemModel) {
        return;
    }
    m_itemModel = itemModel;
    m_winDataTreeView->setModel(m_itemModel);
}

void WidgetMigration::setAppLogic(AppLogic *appLogic)
{
    if (m_appLogic) {
        return;
    }
    m_appLogic = appLogic;
    connect(this, &WidgetMigration::startMigration, m_appLogic, &AppLogic::startMigration);
    connect(m_appLogic, &AppLogic::connetcResult, m_dialog, &DialogMessage::connetcResult);
    connect(m_winDataTreeView, &QTreeView::expanded, m_appLogic, &AppLogic::treeViewExpanded);
    connect(m_appLogic, &AppLogic::saveProgressValue, m_dialog, &DialogMessage::saveProgressValue);
    connect(m_appLogic, &AppLogic::saveProgressMax, m_dialog, &DialogMessage::saveProgressMax);
    connect(m_appLogic, &AppLogic::downloadingFile, m_dialog, &DialogMessage::downloadingFile);
    connect(m_appLogic, &AppLogic::cancelFileMigrate, m_dialog, &DialogMessage::cancelFileMigrate);
    connect(m_dialog, &DialogMessage::resave, m_appLogic, &AppLogic::resave);
}

void WidgetMigration::setCancelHandle(bool *cancelHandle)
{
    m_dialog->setCancelHandle(cancelHandle);
}

void WidgetMigration::onSavebtnClick()
{
    emit startMigration(m_lineEditSavePath->text());
    m_dialog->clearAndShow();
    kom::BuriedPoint::uploadMessage(kom::PluginsWinDataMigration, "startmigration");
}

void WidgetMigration::initUI()
{
    setFixedWidth(800);
    setFixedHeight(600);

    QVBoxLayout *vl = new QVBoxLayout;
    vl->setContentsMargins(24, 20, 24, 20);

    QLabel *labelTreeView = new QLabel(this);
    labelTreeView->setText(Constant::g_chooseFileText);
    vl->addWidget(labelTreeView);

    m_winDataTreeView = new QTreeView(this);
    m_winDataTreeView->setFixedHeight(438);
    m_winDataTreeView->setAnimated(true);
    vl->addWidget(m_winDataTreeView);

    QHBoxLayout *hl = new QHBoxLayout;
    QLabel *labelSavePath = new QLabel(this);
    labelSavePath->setText(Constant::g_saveAsPathText);
    hl->addWidget(labelSavePath);

    m_lineEditSavePath = new QLineEdit(this);
    m_lineEditSavePath->setFixedWidth(400);
    m_lineEditSavePath->installEventFilter(this);
    m_lineEditSavePath->setReadOnly(true);
    QAction *action = new QAction(m_lineEditSavePath);
    action->setIcon(QIcon(":/res/opendir.svg"));
    connect(action, &QAction::triggered, this, &WidgetMigration::changelineEditSavePath);
    m_lineEditSavePath->addAction(action, QLineEdit::TrailingPosition);
    hl->addWidget(m_lineEditSavePath);

    hl->addStretch(9);

    QPushButton *savebtn = new QPushButton(this);
    connect(savebtn, &QPushButton::clicked, this, &WidgetMigration::onSavebtnClick);
    savebtn->setMinimumWidth(96);
    savebtn->setProperty("isImportant", true);
    savebtn->setText(Constant::g_startSaveText);
    hl->addWidget(savebtn);

    vl->addSpacing(6);
    vl->addLayout(hl);
    vl->addStretch(9);
    baseBar()->setLayout(vl);
}

void WidgetMigration::initSavePath()
{
    QString configPath = m_configPath + m_configFileName;
    QString savePath;
    QFile file(configPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        savePath = file.readAll();
        file.close();
    }
    if (!QFile::exists(savePath)) {
        savePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    m_lineEditSavePath->setText(savePath);
}

void WidgetMigration::initDialog()
{
    if (m_dialog) {
        return;
    }
    m_dialog = new DialogMessage(this);
}

void WidgetMigration::changelineEditSavePath()
{
    QString savePath =
        QFileDialog::getExistingDirectory(this, Constant::g_chooseFilePathText, m_lineEditSavePath->text());
    if (!savePath.isEmpty()) {
        m_lineEditSavePath->setText(savePath);
        QString configPath = m_configPath + m_configFileName;
        QFile file(configPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            file.write(savePath.toLocal8Bit());
            file.close();
        }
    }
}

bool WidgetMigration::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress && obj == m_lineEditSavePath) {
        changelineEditSavePath();
    }
    return WidgetCommon::eventFilter(obj, e);
}
