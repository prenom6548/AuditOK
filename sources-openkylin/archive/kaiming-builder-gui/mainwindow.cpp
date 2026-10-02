#include "mainwindow.h"

#include <KWindowEffects>
#include <KWindowSystem>
#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QScrollArea>
#include <QPushButton>
#include <QToolButton>
#include <QVariant>
#include <QIcon>
#include <QLabel>
#include <QSize>
#include <QX11Info>
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>

#include "manifestwidget.h"
#include "outputwidget.h"
#include "componentwidget.h"
#include "startwidget.h"
#include "permissionwidget.h"
#include "xatom-helper.h"
#include "stackedwidgetmanager.h"
#include "messageboxutils.h"
#include "component/doubleclickwidget.h"
#include "common/subprocess.h"
#include "logic/HistoryManager.h"
#include "logic/BuildSettingManager.h"
// #include "buildsettingdialog.h"
#include "buildsettingwidget.h"

#define PC_TITLE_BAR_SIZE 30

class MainWindowPrivate : public QObject
{
    Q_OBJECT

public:
    MainWindowPrivate(MainWindow * parent);
    virtual ~MainWindowPrivate();

    void setupUi();
    void initTileBar();

private:
    Q_DECLARE_PUBLIC(MainWindow)
    MainWindow * q_ptr;

    QVBoxLayout * m_baseLayout;
    StackedWidgetManager * m_baseStackedWidget;

    // 标题栏
    DoubleClickWidget * m_titleWidget;
    QHBoxLayout * m_titleLayout;
    QLabel * m_mTitleIcon;
    QLabel * m_titleLabel;
    QToolButton *m_menuOptionBtn;
    QPushButton * m_minBtn;
    QPushButton * m_maxBtn;
    QPushButton * m_closeBtn;
    //帮助信息
    QAction *m_backupHelp;

    // 引导页
    StartWidget* m_startWidget;
    // 构建清单配置页
    ManifestWidget* m_manifestWidget;
    // 权限配置页
    PermissionWidget* m_permissionWidget;
    // 构建选项配置页
    BuildSettingWidget* m_buildSettingWidget;
    // 构建日志页
    OutputWidget* m_outputWidget;
    // 构建组件配置页
    ComponentWidget* m_componentWidget;

    BuildSettingManager m_manager;
};

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      d_ptr(new MainWindowPrivate(this))
{
    setWindowIcon(QIcon::fromTheme("kaiming-builder-gui", QIcon(":/images/kaiming-builder-gui.png")));
    setWindowTitle(QObject::tr("kaiming-builder-gui"));
    setAutoFillBackground(true);
    setMinimumHeight(700);
    setMinimumWidth(1024);
    setGeometry(0, 0, 1024, 700);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    setFocusPolicy(Qt::ClickFocus);
    setContentsMargins(0, 0, 0, 0);
    // setBackgroundRole(QPalette::Base);

    Q_D(MainWindow);
    d->setupUi();

    // 指示窗口管理器模糊给定窗口后面指定区域的背景(毛玻璃化背景)
#if KWINDOWSYSTEM_ENABLE_DEPRECATED_SINCE(5, 82)
    KWindowEffects::enableBlurBehind(this->winId(), true);
#else
    KWindowEffects::enableBlurBehind(this->winId());
#endif

    // 添加窗管协议
    if (QX11Info::isPlatformX11())
    {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }
    else
    {
        setWindowFlag(Qt::FramelessWindowHint, true);
    }

    centerToScreen();

    // 用户手册
    connect(d->m_backupHelp, &QAction::triggered, this, [=] {
        QUrl url("https://gitee.com/kaiming-docs/kaiming-builder-gui/blob/master/%E4%BD%BF%E7%94%A8%E6%89%8B%E5%86%8C.md");
        QDesktopServices::openUrl(url);
    });

    //最小化按钮
    connect(d->m_minBtn, &QPushButton::clicked, this, &MainWindow::showMinimized);

    //最大化按钮
    connect(d->m_maxBtn, &QPushButton::clicked, this, [=] {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });

    // 双击标题栏-最大化、还原
    connect(d->m_titleWidget, &DoubleClickWidget::doubleClicked, d->m_maxBtn, &QPushButton::click);
    // 关闭按钮
    connect(d->m_closeBtn, &QPushButton::clicked, this, &MainWindow::closeBtn);

    // 连接 ManifestWidget 的 back 上一步信号到首页
    connect(d->m_manifestWidget, &ManifestWidget::back, this, [d]() {
        d->m_startWidget->loadAndDisplayHistory();
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_START);
    });

    // 连接 ManifestWidget 的 next 下一步信号到权限配置页
    connect(d->m_manifestWidget, &ManifestWidget::next, this, [d]() {
        d->m_permissionWidget->setPermissionData(d->m_manifestWidget->getPermissionData());
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_PERMISSION);
    });

    // 连接 PermissionWidget 的 back 上一步信号到清单配置页
    connect(d->m_permissionWidget, &PermissionWidget::back, this, [d]() {
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_MANIFEST);
    });

    // 连接 PermissionWidget 的 next 下一步信号到构建选项配置页
    connect(d->m_permissionWidget, &PermissionWidget::next, this, [d]() {
        d->m_manifestWidget->setPermissionData(d->m_permissionWidget->getPermissionData());
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_BUILDSETTINGS);
        KMBuilderManifest * manifest = GlobelInfo::instance().getManifest();
        d->m_manifestWidget->getManifest();
        manifest->toYaml(GlobelInfo::instance().getYamlPath());
    });

    // 连接 BuildSettingWidget 的 back 上一步信号到权限配置页
    connect(d->m_buildSettingWidget, &BuildSettingWidget::back, this, [d]() {
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_PERMISSION);
    });

    // 连接 BuildSettingWidget 的 next 下一步信号到构建日志页
    connect(d->m_buildSettingWidget, &BuildSettingWidget::next, this, [d]() {
        QString cmd = d->m_buildSettingWidget->getCommandString();
        qDebug() << "用户选择的命令参数:" << cmd;
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_OUTPUT);
        d->m_manager.saveConfig();
        d->m_outputWidget->startBuild(GlobelInfo::instance().getYamlPath());
    });

    // 连接 OutputWidget 的 back 上一步信号到构建设置页
    connect(d->m_outputWidget, &OutputWidget::back, this, [d]() {
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_BUILDSETTINGS);
    });

    // 连接 OutputWidget 的 backOutputClicked 信号到切换页面的槽函数
    connect(d->m_outputWidget, &OutputWidget::backOutputClicked, this, [d]() {
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_MANIFEST);
    });

    // 连接 OutputWidget 的 backHomeOutputClicked 信号到切换页面的槽函数
    connect(d->m_outputWidget, &OutputWidget::backHomeOutputClicked, this, [d]() {
        d->m_startWidget->loadAndDisplayHistory();
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_START);
    });

    // 连接 ManifestWidget 的 addComponentClicked 信号到切换页面和清理的槽函数
    connect(d->m_manifestWidget, &ManifestWidget::addComponentClicked, this, [d]() {
        d->m_manifestWidget->setEditingIndex(-1); // 设置为添加模式
        d->m_componentWidget->setMode(ComponentWidget::AddMode); // 设置为添加模式
        d->m_componentWidget->cleanup(); // 调用清理函数
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_COMPONENT);
    });

    // 连接 ManifestWidget 的 editComponentClicked 信号（编辑模式）
    connect(d->m_manifestWidget, &ManifestWidget::editComponentClicked, this, [d](int index, const KMBuilderManifest::Component &component) {
        d->m_manifestWidget->setEditingIndex(index); // 设置当前编辑的组件索引
        d->m_componentWidget->setMode(ComponentWidget::EditMode); // 设置为编辑模式
        d->m_componentWidget->loadComponentData(component); // 加载组件数据
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_COMPONENT);
    });

    // 连接 ComponentWidget 的 cancelComponentClicked 信号到切换页面的槽函数
    connect(d->m_componentWidget, &ComponentWidget::cancelComponentClicked, this, [d]() {
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_MANIFEST);
    });

    // 连接 ComponentWidget 的 okComponentClicked 信号到切换页面的槽函数
    connect(d->m_componentWidget, &ComponentWidget::okComponentClicked, this, [d]() {
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_MANIFEST);
    });

    // 在Component保存连接信号
    connect(d->m_componentWidget, &ComponentWidget::componentSaved,
            d->m_manifestWidget, &ManifestWidget::handleComponentSaved);

    // 连接 StartWidget 的 folderSelected 信号到处理路径的槽函数
    connect(d->m_startWidget, &StartWidget::folderSelected, this, [d](const QString &folderPath, const QString &projectID) {
        qDebug() << "Selected Folder:" << folderPath;
        d->m_manifestWidget->setOperationFlag(OperationFlag::New);
        GlobelInfo::instance().setProjectDir(folderPath);
        d->m_manifestWidget->clearAll(folderPath, projectID);
        // 切换到空页面
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_MANIFEST);
    });

    // 连接 StartWidget 的 fileSelected 信号到处理文件的槽函数
    connect(d->m_startWidget, &StartWidget::fileSelected, this, [d](const QString &filePath) {
        qDebug() << "Selected File:" << filePath;
        d->m_manifestWidget->setOperationFlag(OperationFlag::Load);
        // 使用 QFileInfo 切割路径
        QFileInfo fileInfo(filePath);
        QString folderPath = fileInfo.path(); // 获取文件夹路径
        QString fileName = fileInfo.fileName(); // 获取文件名
        GlobelInfo::instance().setProjectDir(folderPath);

        qDebug() << "Folder Path:" << folderPath;
        qDebug() << "File Name:" << fileName;

        d->m_manifestWidget->clearAll(folderPath,"");
        // 执行与文件相关的操作
        KMBuilderManifest* manifest = KMBuilderApi::loadFromYaml(filePath);
        d->m_manifestWidget->load(*manifest, fileName);
        // 切换到导入页面
        d->m_baseStackedWidget->setCurrentPage(StackedPage::PAGE_MANIFEST);
    });

//    emit d->m_maxBtn->click();
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeBtn()
{
    if (this->close()) {
        qApp->quit();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    Q_D(MainWindow);
    if (QEvent::WindowStateChange == event->type()) {
        Qt::WindowStates state = this->windowState();
        switch (state) {
        case Qt::WindowMaximized:
        case Qt::WindowFullScreen:
            d->m_maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
            d->m_maxBtn->setToolTip(tr("Normal"));

            break;
        case Qt::WindowNoState:
            d->m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
            d->m_maxBtn->setToolTip(tr("Maximize"));

            break;
        default:
            break;
        }
    }

    QWidget::changeEvent(event);
}

/**
 * @brief 居中显示窗口
 */
void MainWindow::centerToScreen()
{
    QDesktopWidget* m = QApplication::desktop();
    // QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    QRect desk_rect = m->screenGeometry(this);
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = this->width();
    int y = this->height();
    this->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

MainWindowPrivate::MainWindowPrivate(MainWindow * parent)
    : QObject(parent),
      q_ptr(parent)
{}

MainWindowPrivate::~MainWindowPrivate()
{}

void MainWindowPrivate::setupUi()
{
    Q_Q(MainWindow);
    QWidget* baseWiget = q;
    m_baseLayout = new QVBoxLayout;
    baseWiget->setLayout(m_baseLayout);
    m_baseLayout->setContentsMargins(0, 0, 0, 0);

    m_titleWidget = new DoubleClickWidget;
    m_baseLayout->addWidget(m_titleWidget);
    initTileBar();

    m_baseStackedWidget = new StackedWidgetManager;
    m_baseLayout->addWidget(m_baseStackedWidget);

    m_startWidget = new StartWidget;
    m_baseStackedWidget->addWidget(StackedPage::PAGE_START, m_startWidget);

    m_manifestWidget = new ManifestWidget;
    m_baseStackedWidget->addWidget(StackedPage::PAGE_MANIFEST, m_manifestWidget);

    m_permissionWidget = new PermissionWidget;
    m_baseStackedWidget->addWidget(StackedPage::PAGE_PERMISSION, m_permissionWidget);

    m_buildSettingWidget = new BuildSettingWidget(nullptr, &m_manager);
    m_baseStackedWidget->addWidget(StackedPage::PAGE_BUILDSETTINGS, m_buildSettingWidget);

    m_outputWidget = new OutputWidget;
    m_baseStackedWidget->addWidget(StackedPage::PAGE_OUTPUT, m_outputWidget);

    m_componentWidget = new ComponentWidget;
    m_baseStackedWidget->addWidget(StackedPage::PAGE_COMPONENT, m_componentWidget);
}

void MainWindowPrivate::initTileBar()
{
    m_titleWidget->setAutoFillBackground(true);
    m_titleWidget->setFixedHeight(40);
    m_titleLayout = new QHBoxLayout;
    m_titleWidget->setLayout(m_titleLayout);
    m_titleLayout->setContentsMargins(8, 4, 4, 0);
    m_titleLayout->setSpacing(4);

    QIcon titleIcon = QIcon::fromTheme("kaiming-builder-gui", QIcon(":/images/kaiming-builder-gui.png"));
    m_mTitleIcon = new QLabel;
    m_mTitleIcon->setPixmap(titleIcon.pixmap(titleIcon.actualSize(QSize(24, 24))));

    m_titleLabel = new QLabel;
    m_titleLabel->setText(tr("kaiming-builder-gui"));

    m_menuOptionBtn = new QToolButton;;
    m_minBtn        = new QPushButton;
    m_maxBtn        = new QPushButton;
    m_closeBtn      = new QPushButton;

    m_menuOptionBtn->setToolTip(tr("Options"));
    m_minBtn->setToolTip(tr("Minimize"));
    m_maxBtn->setToolTip(tr("Maximize"));
    m_closeBtn->setToolTip(tr("Close"));

    m_menuOptionBtn->setProperty("useButtonPalette", true);
    m_menuOptionBtn->setProperty("useIconHighlightEffect", 0x2);
    m_menuOptionBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_menuOptionBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
    m_menuOptionBtn->setAutoRaise(true);
    m_menuOptionBtn->setPopupMode(QToolButton::InstantPopup);

    m_minBtn->setProperty("useButtonPalette", true);
    m_minBtn->setProperty("useIconHighlightEffect", 0x2);
    m_minBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
    m_minBtn->setFlat(true);
    m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));

    m_maxBtn->setProperty("useButtonPalette", true);
    m_maxBtn->setProperty("useIconHighlightEffect", 0x2);
    m_maxBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
    m_maxBtn->setFlat(true);
    m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));

    m_closeBtn->setProperty("isWindowButton", 0x2);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
    m_closeBtn->setFixedSize(PC_TITLE_BAR_SIZE, PC_TITLE_BAR_SIZE);
    m_closeBtn->setFlat(true);
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));

    QMenu* backupMain = new QMenu(m_titleWidget);
    backupMain->setObjectName("mainMenu");
    m_menuOptionBtn->setMenu(backupMain);

    m_backupHelp = new QAction(tr("Help"), m_titleWidget);
    backupMain->addAction(m_backupHelp);

    m_titleLayout->addWidget(m_mTitleIcon);
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_menuOptionBtn);
    m_titleLayout->addWidget(m_minBtn);
    m_titleLayout->addWidget(m_maxBtn);
    m_titleLayout->addWidget(m_closeBtn);
}

#include "mainwindow.moc"
#include "moc_mainwindow.cpp"
