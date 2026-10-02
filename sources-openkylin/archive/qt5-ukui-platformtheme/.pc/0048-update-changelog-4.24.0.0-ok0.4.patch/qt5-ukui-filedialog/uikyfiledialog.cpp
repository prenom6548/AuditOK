#include <QFileDialog>
#include <peony-qt/global-settings.h>
#include "plugin-manager.h"
#include "kyfiledialog.h"
#include "debug.h"
#include "uikyfiledialog.h"

#include <peony-qt/plugin-manager.h>
#include <peony-qt/menu-plugin-iface.h>
#include <peony-qt/vfs-plugin-iface.h>
#include <peony-qt/vfs-plugin-manager.h>
using namespace UKUIFileDialog;
//Ui_KyFileDialog::Ui_KyFileDialog(QDialog *parent)
//{
//    setupUi(parent);
//}

Ui_KyFileDialog::Ui_KyFileDialog(QWidget *parent) : QWidget(parent)
{
    bool hasFileSafe = false;
    auto vfsMgr = Peony::VFSPluginManager::getInstance();
    auto plugins = vfsMgr->registeredPlugins();
    for (auto plugin : plugins) {
        if (plugin && plugin->uriScheme().contains("filesafe://")) {
            hasFileSafe = true;
        }
    }
    if(!hasFileSafe){
        auto pluginManager = Peony::PluginManager::getInstance();
        const char* c = FILEBOX_PLUGIN_INSTALL_FILES;
        QString path = QString::fromUtf8(c) + "/libpeony-filesafe-menu-plugin.so";
        if (QFile::exists(path)) {
            m_loaderFileSafeMenu = new QPluginLoader(path);
            if(m_loaderFileSafeMenu && m_loaderFileSafeMenu->load())
            {
                auto plugin = m_loaderFileSafeMenu->instance();
                pluginManager->registerPlugin(dynamic_cast<Peony::MenuPluginInterface*>(plugin), plugin);
            }
        }
        QString path1 = QString::fromUtf8(c) + "/libpeony-filesafe-vfs-plugin.so";
        if (QFile::exists(path1)) {
            m_loaderFileSafeVFS = new QPluginLoader(path1);
            if(m_loaderFileSafeVFS && m_loaderFileSafeVFS->load())
            {
                auto plugin = m_loaderFileSafeVFS->instance();
                pluginManager->registerPlugin(dynamic_cast<Peony::VFSPluginIface*>(plugin), plugin);
            }
        }
    }
}

Ui_KyFileDialog::~Ui_KyFileDialog()
{
    /*
    pDebug << "Ui_KyFileDialog destory111..........";
    if(m_loaderFileSafeVFS){
        m_loaderFileSafeVFS->unload();
        delete m_loaderFileSafeVFS;
        m_loaderFileSafeVFS = nullptr;
    }
    if(m_loaderFileSafeMenu){
        m_loaderFileSafeMenu->unload();
        delete m_loaderFileSafeMenu;
        m_loaderFileSafeMenu = nullptr;
    }

    if(m_pathbarWidget){
        m_pathbarWidget->deleteLater();
        m_pathbarWidget = nullptr;
    }
    qDebug() << "Ui_KyFileDialog destory122222..........";

    if(m_pathbar){
        m_pathbar->deleteLater();
        m_pathbar = nullptr;
    }
    if(m_searchBtn){
        m_searchBtn->deleteLater();
        m_searchBtn = nullptr;
    }
    if(m_backButton){
        m_backButton->deleteLater();
        m_backButton = nullptr;
    }
    qDebug() << "Ui_KyFileDialog destory3223..........";

    if(m_forwardButton){
        m_forwardButton->deleteLater();
        m_forwardButton = nullptr;
    }
    if(m_toParentButton){
        m_toParentButton->deleteLater();
        m_toParentButton = nullptr;
    }
    if(m_modeButton){
        m_modeButton->deleteLater();
        m_modeButton = nullptr;
    }
    if(m_sortButton){
        m_sortButton->deleteLater();
        m_sortButton = nullptr;
    }
    if(m_maximizeAndRestore){
        m_maximizeAndRestore->deleteLater();
        m_maximizeAndRestore = nullptr;
    }
    if(m_closeButton){
        m_closeButton->deleteLater();
        m_closeButton = nullptr;
    }
    if(m_modeMenu){
        m_modeMenu->deleteLater();
        m_modeMenu = nullptr;
    }
    qDebug() << "Ui_KyFileDialog destory3555423..........";

    if(m_sortMenu){
        m_sortMenu->deleteLater();
        m_sortMenu = nullptr;
    }
    if(m_listModeAction){
        m_listModeAction->deleteLater();
        m_listModeAction = nullptr;
    }
    if(m_iconModeAction){
        m_iconModeAction->deleteLater();
        m_iconModeAction = nullptr;
    }
    if(m_sider){
        m_sider->deleteLater();
        m_sider = nullptr;
    }
    qDebug() << "Ui_KyFileDialog destory223322..........";

    if(m_frame){
        m_frame->deleteLater();
        m_frame = nullptr;
    }
    if(m_splitter){
        m_splitter->deleteLater();
        m_splitter = nullptr;
    }
    if(vboxLayout){
        vboxLayout->deleteLater();
        vboxLayout = nullptr;
    }
    if(m_treeView){
        m_treeView->deleteLater();
        m_treeView = nullptr;
    }
    if(m_fileNameLabel){
        m_fileNameLabel->deleteLater();
        m_fileNameLabel = nullptr;
    }
    if(m_fileNameEdit){
       m_fileNameEdit->deleteLater();
       m_fileNameEdit = nullptr;
    }
    qDebug() << "Ui_KyFileDialog destory1245677..........";

    if(m_fileTypeCombo){
       m_fileTypeCombo->deleteLater();
       m_fileTypeCombo = nullptr;
    }
    if(m_newFolderButton){
       m_newFolderButton->deleteLater();
       m_newFolderButton = nullptr;
    }
    if(m_acceptButton){
       m_acceptButton->deleteLater();
       m_acceptButton = nullptr;
    }
    if(m_rejectButton){
       m_rejectButton->deleteLater();
       m_rejectButton = nullptr;
    }
    qDebug() << "Ui_KyFileDialog destory23467899..........";

    if(m_upSeperate){
       m_upSeperate->deleteLater();
       m_upSeperate = nullptr;
    }
    if(m_downSeperate){
       m_downSeperate->deleteLater();
       m_downSeperate = nullptr;
    }
    if(m_widget){
       m_widget->deleteLater();
       m_widget = nullptr;
    }
    qDebug() << "destory345..........";

    if(m_container){
        qDebug() << "destory555..........";

       m_container->deleteLater();
       m_container = nullptr;
       qDebug() << "destory666..........";

    }
    qDebug() << "destory777..........";

    if(m_siderWidget){
       m_siderWidget->deleteLater();
       m_siderWidget = nullptr;
    }

    if(m_sortTypeGroup){
       m_sortTypeGroup->deleteLater();
       m_sortTypeGroup = nullptr;
    }
    if(m_fileName){
       m_fileName->deleteLater();
       m_fileName = nullptr;
    }
    if(m_modifiedDate){
       m_modifiedDate->deleteLater();
       m_modifiedDate = nullptr;
    }
    if(m_fileType){
       m_fileType->deleteLater();
       m_fileType = nullptr;
    }
    if(m_fileSize){
       m_fileSize->deleteLater();
       m_fileSize = nullptr;
    }
    if(m_originalPath){
       m_originalPath->deleteLater();
       m_originalPath = nullptr;
    }
    if(m_sortOrderGroup){
       m_sortOrderGroup->deleteLater();
       m_sortOrderGroup = nullptr;
    }
    if(m_descending){
       m_descending->deleteLater();
       m_descending = nullptr;
    }
    if(m_ascending){
       m_ascending->deleteLater();
       m_ascending = nullptr;
    }
    if(m_useGlobalSortAction){
       m_useGlobalSortAction->deleteLater();
       m_useGlobalSortAction = nullptr;
    }
    qDebug() << "Ui_KyFileDialog destory8888..........";

    if(m_siderLayout){
       m_siderLayout->deleteLater();
       m_siderLayout = nullptr;
    }
//    if(m_gridLayout){
//        m_gridLayout->deleteLater();
//        m_gridLayout= nullptr;
//    }

    if(m_hHeaderLayout){
        m_hHeaderLayout->deleteLater();
        m_hHeaderLayout = nullptr;
    }
    if(m_hLineEditLayout){
        m_hLineEditLayout->deleteLater();
        m_hLineEditLayout = nullptr;
    }
    if(m_hBtnLayout){
        m_hBtnLayout->deleteLater();
        m_hBtnLayout = nullptr;
    }
    qDebug() << "Ui_KyFileDialog destory123..........";
*/
}

void Ui_KyFileDialog::initSortMenu(QDialog *mKyFileDialog)
{
    m_sortButton = new MenuToolButton(this);
    m_sortMenu = new QMenu(m_sortButton);

    m_sortTypeGroup = new QActionGroup(m_sortMenu);
    m_sortTypeGroup->setExclusive(true);

    m_fileName = m_sortMenu->addAction(QObject::tr("File Name"));
    m_fileName->setCheckable(true);
    m_sortTypeGroup->addAction(m_fileName);

    m_modifiedDate = m_sortMenu->addAction(QObject::tr("Modified Date"));
    m_modifiedDate->setCheckable(true);
    m_sortTypeGroup->addAction(m_modifiedDate);

    m_fileType = m_sortMenu->addAction(QObject::tr("File Type"));
    m_fileType->setCheckable(true);
    m_sortTypeGroup->addAction(m_fileType);

    m_fileSize = m_sortMenu->addAction(QObject::tr("File Size"));
    m_fileSize->setCheckable(true);
    m_sortTypeGroup->addAction(m_fileSize);

    m_originalPath = m_sortMenu->addAction(QObject::tr("Original Path"));
    m_originalPath->setCheckable(true);
    m_sortTypeGroup->addAction(m_originalPath);

    m_sortMenu->addSeparator();

    m_sortOrderGroup = new QActionGroup(m_sortMenu);
    m_sortOrderGroup->setExclusive(true);

    m_descending = m_sortMenu->addAction(QObject::tr("Descending"));
    m_descending->setCheckable(true);
    m_sortOrderGroup->addAction(m_descending);

    //switch defautl Descending Ascending order, fix bug#99924
    m_ascending = m_sortMenu->addAction(QObject::tr("Ascending"));
    m_ascending->setCheckable(true);
    m_sortOrderGroup->addAction(m_ascending);

    m_sortMenu->addSeparator();

    // m_useGlobalSortAction = m_sortMenu->addAction(QObject::tr("Use global sorting"));
    m_useGlobalSortAction = m_sortMenu->addAction(QObject::tr("Use current sorting for all folders"));
    m_useGlobalSortAction->setCheckable(true);

    m_sortButton->setMenu(m_sortMenu);
    m_sortButton->setPopupMode(QToolButton::InstantPopup);
    m_sortButton->setAutoRaise(true);
    m_sortButton->setFixedSize(QSize(57, 36));
    m_sortButton->setIconSize(QSize(16, 16));
    m_sortButton->setAccessibleName(tr("Sort"));

}

void Ui_KyFileDialog::initModeMenu(QDialog *mKyFileDialog)
{
    m_modeButton = new MenuToolButton(this);
    m_modeButton->setPopupMode(QToolButton::InstantPopup);
    m_modeButton->setAutoRaise(true);
    m_modeButton->setFixedSize(QSize(57, 36));
    m_modeButton->setIconSize(QSize(16, 16));

    m_modeMenu = new QMenu(m_modeButton);
    m_listModeAction = m_modeMenu->addAction(QIcon::fromTheme("view-list-symbolic"), QObject::tr("List View"));
    m_iconModeAction = m_modeMenu->addAction(QIcon::fromTheme("view-grid-symbolic"), QObject::tr("Icon View"));
    m_listModeAction->setCheckable(true);
    m_iconModeAction->setCheckable(true);
    m_modeButton->setMenu(m_modeMenu);
    m_modeButton->setAccessibleName(tr("View mode"));
}

void Ui_KyFileDialog::initSiderBar(QDialog *mKyFileDialog)
{
    m_siderWidget = new QWidget(this);
    m_siderWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_siderLayout = new QVBoxLayout();
    m_siderLayout->setContentsMargins(0,0,0,0);

    m_sider = new FileDialogSideBar(mKyFileDialog);
    m_sider->setAttribute(Qt::WA_TranslucentBackground);
    m_sider->setAccessibleName(tr("Places"));
}

void Ui_KyFileDialog::initHeaderBar(QDialog *mKyFileDialog)
{
    m_hHeaderLayout = new QHBoxLayout(this);
    initModeMenu(mKyFileDialog);
    initSortMenu(mKyFileDialog);

    m_hHeaderLayout->setContentsMargins(0,0,8,0);
    m_hHeaderLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    m_backButton = new QToolButton(this);
    m_forwardButton = new QToolButton(this);
    m_toParentButton = new QToolButton(this);

    m_backButton->setAccessibleName(tr("Back"));
    m_forwardButton->setAccessibleName(tr("Forward"));
    m_toParentButton->setAccessibleName(tr("Go to parent folder"));

    m_pathbarWidget = new FileDialogPathBar(this);//new Peony::AdvancedLocationBar(mKyFileDialog);//
    m_pathbar = m_pathbarWidget->getPathBar();
    m_pathbar->setMinimumWidth(250);
    m_pathbar->setFocusPolicy(Qt::FocusPolicy(m_pathbar->focusPolicy() & ~Qt::TabFocus));

    // m_searchBtn = new QToolButton(this);

    m_maximizeAndRestore = new QToolButton(this);

    m_closeButton = new QToolButton(this);
    m_maximizeAndRestore->setAccessibleName(tr("Maximize or restore"));
    m_closeButton->setAccessibleName(tr("Close"));
    m_hHeaderLayout->setAlignment(Qt::AlignVCenter);

    m_hHeaderLayout->addWidget(m_backButton);
    m_hHeaderLayout->addWidget(m_forwardButton);
    m_hHeaderLayout->addWidget(m_toParentButton);
    m_hHeaderLayout->addWidget(m_pathbarWidget);
    // m_hHeaderLayout->addWidget(m_searchBtn);
    m_hHeaderLayout->addWidget(m_modeButton);
    m_hHeaderLayout->addWidget(m_sortButton);
    m_hHeaderLayout->addWidget(m_maximizeAndRestore);
    m_hHeaderLayout->addWidget(m_closeButton);

}

void Ui_KyFileDialog::initLineEditLayout(QDialog *mKyFileDialog)
{
    m_hLineEditLayout = new QHBoxLayout(this);
    m_hLineEditLayout->setContentsMargins(4,0,24,0);
    m_fileNameLabel = new QLabel(this);
    m_fileNameLabel->setObjectName(QString::fromUtf8("fileNameLabel"));
    m_fileNameLabel->setAccessibleName(tr("File name"));
    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(m_fileNameLabel->sizePolicy().hasHeightForWidth());
    m_fileNameLabel->setSizePolicy(sizePolicy2);
    m_fileNameLabel->setMinimumSize(QSize(0, 0));
    pDebug << "3333333333333333333";

    m_fileNameEdit = new QLineEdit(mKyFileDialog);
    m_fileNameEdit->setObjectName("fileNameEdit");
    m_fileNameEdit->setAccessibleName(tr("File name"));
    m_fileNameEdit->setMaxLength(-1);
    QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy3.setHorizontalStretch(1);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(m_fileNameEdit->sizePolicy().hasHeightForWidth());
    m_fileNameEdit->setSizePolicy(sizePolicy3);
    pDebug << "44444444444444444";

    m_fileTypeCombo = new FileDialogComboBox(this);
    m_fileTypeCombo->setProperty("needShowToolTip", true);
    m_fileTypeCombo->setAccessibleName(tr("File type"));
    QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(m_fileTypeCombo->sizePolicy().hasHeightForWidth());
    m_fileTypeCombo->setSizePolicy(sizePolicy4);
//    m_fileTypeCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    //m_fileTypeCombo->setVisible(false);

    m_hLineEditLayout->addWidget(m_fileNameLabel);
    m_hLineEditLayout->addWidget(m_fileNameEdit);
    m_hLineEditLayout->addWidget(m_fileTypeCombo);
    m_hLineEditLayout->setSpacing(15);
}

void Ui_KyFileDialog::intiBtnLayout(QDialog *mKyFileDialog)
{
    m_hBtnLayout = new QHBoxLayout(this);
    m_hBtnLayout->addSpacing(20);
    m_hBtnLayout->setContentsMargins(0,0,24,0);
    m_newFolderButton =new QPushButton(mKyFileDialog);
    m_newFolderButton->setObjectName(QString::fromUtf8("newFolderButton"));
    m_newFolderButton->setAccessibleName(tr("New Folder"));
    m_hBtnLayout->addWidget(m_newFolderButton);


    m_acceptButton = new QPushButton(mKyFileDialog);
    m_rejectButton = new QPushButton(mKyFileDialog);
    m_acceptButton->setAccessibleName(tr("Open"));
    m_rejectButton->setAccessibleName(tr("Cancel"));
    m_acceptButton->setEnabled(false);
    m_hBtnLayout->addStretch();
    m_hBtnLayout->addWidget(m_rejectButton, 0, Qt::AlignRight | Qt::AlignVCenter);
    m_hBtnLayout->addSpacing(15);
    m_hBtnLayout->addWidget(m_acceptButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    m_acceptButton->setObjectName(QString::fromUtf8("acceptButton"));
    m_rejectButton->setObjectName(QString::fromUtf8("rejectButton"));

    pDebug << "6666666666666666";
}

void Ui_KyFileDialog::setupUi(QDialog *mKyFileDialog)
{
    if(mKyFileDialog->objectName().isEmpty()) {
        mKyFileDialog->setObjectName(QString::fromUtf8("KyNativeFileDialog"));
    }
    auto sliderWidth = Peony::GlobalSettings::getInstance()->getValue(DEFAULT_SIDEBAR_WIDTH).toInt();
    int viewWidth = 125 * 6 + 24;
    //qDebug() << "width..." << sliderWidth + viewWidth;
    mKyFileDialog->resize(sliderWidth + viewWidth, 635);

    mKyFileDialog->setSizeGripEnabled(true);
    mKyFileDialog->setAttribute(Qt::WA_TranslucentBackground);
    mKyFileDialog->setContentsMargins(0,0,0,0);

    //FIXME: should I load plugins async?
//    Peony::PluginManager::init();//文管插件（保护箱 多端协同等）


    m_gridLayout = new QGridLayout(mKyFileDialog);
    m_gridLayout->setObjectName(QString::fromUtf8("gridLayout"));

    m_container = new Peony::DirectoryViewContainer(this);
    m_container->setProperty("KFileDialog", QVariant::fromValue(mKyFileDialog));
    m_frame = new QFrame(this);
    vboxLayout = new QVBoxLayout(this);

    initSiderBar(mKyFileDialog);

    initHeaderBar(mKyFileDialog);
    initLineEditLayout(mKyFileDialog);
    intiBtnLayout(mKyFileDialog);

    vboxLayout->addSpacing(8);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout->setContentsMargins(0, 0, 0, 0);

    vboxLayout->addLayout(m_hHeaderLayout);
    vboxLayout->addSpacing(6);
    m_upSeperate = new QFrame(mKyFileDialog);
    m_upSeperate->setFrameShape(QFrame::HLine);
    m_upSeperate->setFrameShadow(QFrame::Plain);
    vboxLayout->addWidget(m_upSeperate);
    vboxLayout->addSpacing(6);
    vboxLayout->addWidget(m_container);
    vboxLayout->addSpacing(15);

    vboxLayout->addLayout(m_hLineEditLayout);
    vboxLayout->addSpacing(16);

    m_downSeperate = new QFrame(mKyFileDialog);
    m_downSeperate->setFrameShape(QFrame::HLine);
    m_downSeperate->setFrameShadow(QFrame::Plain);
    vboxLayout->addWidget(m_downSeperate);
    vboxLayout->addSpacing(16);

    vboxLayout->addLayout(m_hBtnLayout);
    vboxLayout->addSpacing(25);

    m_frame->setFrameShadow(QFrame::Raised);
    m_frame->setFrameShape(QFrame::NoFrame);
    m_frame->setLayout(vboxLayout);

    m_splitter = new QSplitter(this);
    m_splitter->setAttribute(Qt::WA_TranslucentBackground);
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(m_splitter->sizePolicy().hasHeightForWidth());
    m_splitter->setSizePolicy(sizePolicy1);
    m_splitter->setOrientation(Qt::Horizontal);
    m_splitter->setHandleWidth(0);
    m_siderLayout->addWidget(m_sider);
    m_siderWidget->setLayout(m_siderLayout);
    m_siderWidget->setMinimumWidth(180);
    m_splitter->addWidget(m_siderWidget);
    m_splitter->addWidget(m_frame);
    QList<int> list;
    list << sliderWidth << viewWidth;
    m_splitter->setSizes(list);
    int siderWidIndex = m_splitter->indexOf(m_siderWidget);
    int frameIndex = m_splitter->indexOf(m_frame);
    m_splitter->setCollapsible(siderWidIndex, false);
    m_splitter->setCollapsible(frameIndex, false);
    m_gridLayout->addWidget(m_splitter);
    m_gridLayout->setContentsMargins(0,0,0,0);

    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
}
