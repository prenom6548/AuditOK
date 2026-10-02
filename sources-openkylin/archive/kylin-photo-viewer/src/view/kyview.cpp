/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "kyview.h"
#include "sizedate.h"
#include "X11/Xlib.h"
#include <QX11Info>
#include <QWindow>
#include "global/interactiveqml.h"
extern QElapsedTimer startTime;
KyView *KyView::mutual = nullptr;
KyView::KyView(const QStringList &args)
{
    this->setObjectName("Kylin-photo-viewer_QWidget_mainWindow");
    this->setAccessibleName("Kylin-photo-viewer_QWidget_mainWindow");
    this->setAccessibleDescription("main window");
    //添加ocr命令行唤醒操作
    QStringList fileList = args;
    if ( fileList.count() > 1 && fileList[1] == "--ocr"){
        m_ocrFlag = true;
        fileList.removeAt(1);
    }
    Interaction::getInstance()->creatCore(fileList);
    initInteraction();
    this->setWindowSize();
    initStartupIcon();
    mutual = this;
    //毛玻璃
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->setBlurEffect(QRegion(), 0, true);
    QPalette p = palette();
    m_backColor = p.color(QPalette::Window);
}
KyView::~KyView()
{
    delete hOrVMode;
    hOrVMode = nullptr;
    delete m_windowHelper;
    m_windowHelper = nullptr;
}

void KyView::initStartupIcon()
{
    m_startupWidget = new QWidget(this);
    m_startupWidget->setGeometry(0, 0, this->width(), this->height());
    m_startupIcon = new QLabel(m_startupWidget);
    m_startupIcon->setAlignment(Qt::AlignCenter);
    m_startupIcon->setPixmap(QIcon::fromTheme("kylin-photo-viewer").pixmap(128, 128));
    startupLayout = new QVBoxLayout(m_startupWidget);
    startupLayout->setContentsMargins(0, 0, 0, 0);
    startupLayout->addStretch(1);
    startupLayout->addWidget(m_startupIcon);
    startupLayout->addStretch(1);
    m_startupWidget->setLayout(startupLayout);
    m_startupWidget->show();
    qWarning()<<"1 main()函数 m_startupWidget显示 time = "<<startTime.elapsed()<<" ms";
}

void KyView::hideStartupIcon(){
    if(m_startupWidget){
        m_startupWidget->hide();
        m_startupWidget->deleteLater();
        m_startupWidget = nullptr;
    }
}

void KyView::initKyview()
{
    //检查系统平台
    if (platForm.contains(Variable::platForm)) {
        toolbarHeightValue = TOOLBAR_HEIGHT_INTEL;
        titlebarHeightValue = BAR_HEIGHT_INTEL;
        m_miniSize = MINI_SIZE_INTEL;
    } else {
        toolbarHeightValue = BAR_HEIGHT;
        titlebarHeightValue = BAR_HEIGHT;
        m_miniSize = MINI_SIZE;
    }
    //获取系统语言
    m_locale = QLocale::system().name();
    // 初始化按键键值
    m_keyState = "keyNone";
    /* 初始化横竖屏的初值start */
    hOrVMode = new HorizontalOrVerticalMode;
    //标题栏
    m_titlebar = new TitleBar(this);
    m_titlebar->move(0, 0);
    m_titlebar->resize(this->width(), titlebarHeightValue);
    m_titlebar->g_titleWid->resize(this->width(), titlebarHeightValue);
    m_titlebar->show();
    m_titlebar->installEventFilter(this);
    m_titlebar->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_titlebar->g_fullscreen->installEventFilter(this);
    UkuiWindowHelper *m_titleBarWindowHelper = new UkuiWindowHelper(m_titlebar);
    m_titleBarWindowHelper->setOpaqueRegion(QRegion());
    if (this->isMaximized() || this->isFullScreen() || m_nowFullScreen) {
        m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的全屏图标
        m_titlebar->g_fullscreen->setToolTip(tr("recovery"));
    } else {
        m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-maximize-symbolic")); //主题库的恢复图标
        m_titlebar->g_fullscreen->setToolTip(tr("full srceen"));
    }
    hLayoutFlag = hOrVMode->defaultModeCapture();
    this->setTitleBtnHide();
    this->installEventFilter(this);
    connect(hOrVMode, &HorizontalOrVerticalMode::RotationSig, this, &KyView::slotIntelHVModeChanged);
    this->setWindowTitle(tr("Pictures"));

    //响应mouseMovEevent的无按压事件
    this->setMouseTracking(true);
    //响应拖拽事件
    this->setAcceptDrops(true);
    //焦点不在窗口上时鼠标悬浮可显示tooltips
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    //定时器
    m_timernavi = new QTimer(this);
    //应用居中
    int sw = QGuiApplication::primaryScreen()->availableGeometry().width();
    int sh = QGuiApplication::primaryScreen()->availableGeometry().height();
    leftPoint = QPoint((sw - this->width()) / 2, (sh - this->height()) / 2);
    this->setGeometry(QRect(leftPoint, QSize(this->width(), this->height())));
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, this, [=](const kdk::WindowId& window_id){
            if (getpid() == kdk::WindowManager::getPid(window_id) && m_isFirst)
            {
                m_windowId = window_id;
                m_isFirst = false;
                kdk::WindowManager::activateWindow(m_windowId);
            }
        });
    }
    this->setMinimumSize(m_miniSize);
    //要判断打开状态，是否隐藏主界面--打开按钮widget
    this->openState();
    //初始化手势
    this->initGrabGesture();
    //最大化和还原
    connect(m_titlebar, &TitleBar::recovery, this, &KyView::changOrigSize);
    //右上菜单--打开
    connect(m_titlebar, &TitleBar::openSignal, this, &KyView::titleOpenImagePath);
    //打开关于，两栏隐藏
    connect(m_titlebar, &TitleBar::aboutShow, this, &KyView::aboutShow);
    //打开帮助
    connect(m_titlebar, &TitleBar::helpShow, this, &KyView::callUserManual);
    connect(m_titlebar, &TitleBar::exitApp, this, &KyView::needExitApp);
}

void KyView::startWithOpenImage(QString path)
{
    hideStartupIcon();
    if (path != Variable::API_TYPE) {
        initKyview();
        kdk_dia_upload_program_startup();
    }
    if (path == "") {
        onlyStartup = true;
        initOpenImage();
        return;
    }
    // 判断图片文件有无访问权限
    QFileInfo info(path);
    if (!info.isReadable()) {
        QMessageBox::critical(this, tr("Warning"), tr("permission denied"));
        onlyStartup = true;
        initOpenImage();
        return;
    }

    onlyStartup = false;
    initShowImage();
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    //双击和右键打开图片，将路径写入
    Variable::setSettings("imagePath", info.absolutePath());
    Q_EMIT openEmptyFile(false);
    Q_EMIT openImageSignal(path);
}

void KyView::initOpenImage()
{
    if (m_dragWidget == nullptr) {
        m_dragWidget = new OpenImage(this);
        m_dragWidget->setFixedSize(this->width() / 3 * 2, this->height() / 5 * 3);
        m_dragWidget->move(int((this->width() - m_dragWidget->width()) / 2),
                           int((this->height() - m_dragWidget->height()) / 2));

        connect(m_dragWidget, &OpenImage::getPath, this, &KyView::openImagePath);
        connect(this, &KyView::setIsSupportOpen, m_dragWidget, &OpenImage::setIsSupportOpen);
        this->initGsetting();
    }
    m_dragWidget->show();
}

void KyView::initShowImage()
{
    double scale = this->windowHandle()->devicePixelRatio();
    Variable::g_devicePixelRatio = scale;
    onlyStartup = false;
    //图片展示界面
    if (m_showImageWidget == nullptr) {
        m_showImageWidget = new ShowImageWidget(this);
        m_showImageWidget->setAttribute(Qt::WA_TranslucentBackground, true);
        m_showImageWidget->setMouseTracking(true);
        m_showImageWidget->move(int((this->width() - m_showImageWidget->width()) / 2),
                                int((this->height() - m_showImageWidget->height()) / 2));
        m_showImageWidget->hide();
        showImageChange();
        initconnect();
        Q_EMIT needResize();
    }
    initOtherWidget();
    initGsetting();
    //添加ocr命令行唤醒操作
    if (m_ocrFlag){
        QTimer::singleShot(1500, this, [=](){
            Q_EMIT m_toolbar->ocrSignal();
        });
        m_ocrFlag = false;
    }
}

void KyView::initOtherWidget()
{
    if (m_toolbar == nullptr) {
        m_toolbar = new ToolBar(this);
        m_toolbar->move(int((this->width() - m_toolbar->width()) / 2), this->height() - m_toolbar->height() + 4 - 10);
        //显示相册
        connect(m_toolbar, &ToolBar::showSidebar, this, &KyView::showSidebar);
        //展示或隐藏图片信息窗口
        connect(m_toolbar, &ToolBar::showInfor, this, &KyView::showInforWid);
        //展示标注工具栏
        connect(m_toolbar, &ToolBar::showMarkingWid, this, &KyView::toShowMarkWid);
        //改变顶栏显示的图片名字
        connect(m_showImageWidget, &ShowImageWidget::perRate, m_toolbar, &ToolBar::changePerRate);
        //删除时界面变化
        connect(m_toolbar, &ToolBar::isDelete, m_showImageWidget, &ShowImageWidget::isDelete);
        connect(m_showImageWidget, &ShowImageWidget::needDelete, m_toolbar, &ToolBar::delImage);
        connect(m_toolbar, &ToolBar::startCutting, m_showImageWidget, &ShowImageWidget::startCutColor); //裁剪时改变界面背景颜色
        connect(m_showImageWidget, &ShowImageWidget::sendImageType, m_toolbar, &ToolBar::getImageType);
        connect(m_titlebar, &TitleBar::updateFileInfo, m_toolbar, &ToolBar::updateFileInfo); //更新工具栏文件信息
        connect(m_toolbar, &ToolBar::hideTwoBar, this, &KyView::hideTwoBar); //开始裁剪，隐藏相关控件
        connect(m_toolbar, &ToolBar::viewOCRMode, this, &KyView::viewOCRMode); //隐藏其他控件进入OCR模式
        connect(m_toolbar, &ToolBar::startGetText, m_showImageWidget, &ShowImageWidget::startGetText); //调整为OCR模式对应样式
        connect(m_showImageWidget, &ShowImageWidget::sendSuffixToToolbar, m_toolbar, &ToolBar::getSuffix);
        connect(m_toolbar, &ToolBar::startZoomImage, m_showImageWidget, &ShowImageWidget::operatyImage);
        connect(m_toolbar, &ToolBar::markUpdateImage, m_showImageWidget, &ShowImageWidget::openImage);
        connect(m_showImageWidget, &ShowImageWidget::enterSign, m_toolbar, &ToolBar::enterSign);
        connect(m_showImageWidget, &ShowImageWidget::startSignToToolbar, m_toolbar, &ToolBar::labelbar);
        connect(m_showImageWidget, &ShowImageWidget::reCut, m_toolbar, &ToolBar::reCut);  //不符合限制，重新裁剪
        //背景替换
        connect(m_toolbar, &ToolBar::startMatting, m_showImageWidget, &ShowImageWidget::enterReplaceBackground);
        //扫描黑白件
        connect(m_toolbar, &ToolBar::startScanner, m_showImageWidget, &ShowImageWidget::enterScanImage);
    }
    if (m_sideBar == nullptr) {
        m_sideBar = new SideBar(this);
        m_sideBar->installEventFilter(this);
        m_sideBar->hide();

        //判断为相册处切换图片，刷新动图右键内容
        connect(m_sideBar, &SideBar::changeImage, m_showImageWidget, &ShowImageWidget::albumChangeImage);
        connect(this, &KyView::openEmptyFile, m_sideBar, &SideBar::openEmptyFile);
        //相册尺寸改变，位置改变
        connect(m_sideBar, &SideBar::sizeChange, this, &KyView::albumPosChange);
        connect(m_sideBar,&SideBar::showLoadingImageWid, this, &KyView::showLoadingImageWid);
        //判断打开图片成功与否和无权限删除文件，界面应相应的做出改变
        connect(m_showImageWidget, &ShowImageWidget::fileState, m_toolbar, &ToolBar::setButtonState);
        //设置相册尺寸
        connect(m_showImageWidget, &ShowImageWidget::changeSideSize, [=](const int &type) {
            m_sideBar->getSelect(type);
            this->setSidebarState(type);
        });
    }
    connect(m_timernavi, &QTimer::timeout, this, &KyView::delayHide_navi);
}

void KyView::openImagePath(QString path)
{
    initShowImage();
    //空校验，点击“取消”
    if (path.isEmpty()) {
        Q_EMIT openEmptyFile(true);
        return;
    }
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    //写入配置
    QFileInfo info(path);
    Variable::setSettings("imagePath", info.absolutePath());
    Q_EMIT openEmptyFile(false);
    Q_EMIT openImageSignal(path);
}

void KyView::titleOpenImagePath()
{
    QFileDialog m_fileDialog(this);
    m_fileDialog.update();
    QList<QUrl> list = m_fileDialog.sidebarUrls();
    int sidebarNum = 8; //最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        //华为990、9a0需要屏蔽最小系统挂载的目录
        if (fi.fileName() == "2691-6AB8")
            continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(&m_fileDialog);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, &m_fileDialog,
            [=, &sidebarNum, &mntUrlList, &list, &m_fileDialog](const QString path) {
                QDir wmntDir(path);
                wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
                QFileInfoList wfilist = wmntDir.entryInfoList();
                mntUrlList.clear();
                for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
                    QFileInfo fi = wfilist.at(i);
                    //华为990、9a0需要屏蔽最小系统挂载的目录
                    if (fi.fileName() == "2691-6AB8")
                        continue;
                    mntUrlList << QUrl("file://" + fi.filePath());
                }
                m_fileDialog.setSidebarUrls(list + mntUrlList);
                m_fileDialog.update();
            });

    connect(&m_fileDialog, &QFileDialog::finished, &m_fileDialog, [=, &list, &m_fileDialog]() {
        m_fileDialog.setSidebarUrls(list);
    });

    QString allFormats;
    allFormats = Variable::SUPPORT_FORMATS.join(" *.");
    QString setFilterAll = tr("Image Files") + QString(" (*.%1)").arg(allFormats);
    QStringList formatList = Variable::SUPPORT_FORMATS_CLASSIFY.split(";;", QString::SkipEmptyParts);
    QStringList categorizedFilters;
    for (const QString& formatStr: formatList) {
        categorizedFilters << QString("%1 %2").arg(formatStr).arg(formatStr);
    }
    QStringList filters;
    filters << setFilterAll << categorizedFilters;
    m_fileDialog.setNameFilters(filters);

    //这行要添加，设置左侧导航
    m_fileDialog.setSidebarUrls(list + mntUrlList);
    //设置视图模式
    m_fileDialog.setViewMode(QFileDialog::Detail);
    m_fileDialog.setWindowModality(Qt::WindowModal);
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    m_fileDialog.setFileMode(QFileDialog::ExistingFiles);
    m_fileDialog.setOption(QFileDialog::HideNameFilterDetails);

    QString file_path;
    QString openFile = tr("Open Image");

    //打开文件夹中的图片文件
    //file_path = m_fileDialog.getOpenFileName(this, openFile, Variable::getSettings("imagePath").toString(), format);
    m_fileDialog.setWindowTitle(openFile);
    m_fileDialog.setDirectory(Variable::getSettings("imagePath").toString());
    if (!QFile::exists(m_fileDialog.directory().absolutePath())) {
        // 如果路径无效，则设置默认路径为家目录
        m_fileDialog.setDirectory(QDir::homePath());
    }
    if (m_fileDialog.exec() == QDialog::Accepted){
        file_path = m_fileDialog.selectedFiles().first();
    }
    //空校验，点击“取消”
    if (file_path.isEmpty()) {
        Q_EMIT openEmptyFile(true);
        return;
    }
    initShowImage();
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    //写入配置
    QFileInfo info(file_path);
    Variable::setSettings("imagePath", info.absolutePath());
    Q_EMIT openEmptyFile(false);
    Q_EMIT openImageSignal(file_path);
}

void KyView::showLoadingImageWid()
{
    if (m_loadingImageWid == nullptr) {
        m_loadingImageWid = new QWidget(this);
        m_loadingImageWid->setAttribute(Qt::WA_TranslucentBackground);
        m_loadingImageWid->setWindowFlags(Qt::FramelessWindowHint);
        m_loadingImageMovie = new QMovie(":/res/res/loadgif23.gif");
        m_loadingImageMovie->setParent(this);
        m_loadingImageIcon= new QLabel(m_loadingImageWid);
        m_loadingImageIcon->setAttribute(Qt::WA_TranslucentBackground);
        m_loadingImageIcon->setFixedSize(32, 32);
        m_loadingImageIcon->setMovie(m_loadingImageMovie);
        m_loadingImageText = new QLabel(m_loadingImageWid);
        m_loadingImageText->setAttribute(Qt::WA_TranslucentBackground);
        m_loadingImageText->setText(tr("Loading the image..."));
        m_loadingImageLayout = new QVBoxLayout(m_loadingImageWid);
        m_loadingImageLayout->addWidget(m_loadingImageIcon, 0, Qt::AlignCenter);
        m_loadingImageLayout->setSpacing(12);
        m_loadingImageLayout->addWidget(m_loadingImageText, 0, Qt::AlignCenter);
        m_loadingImageWid->setLayout(m_loadingImageLayout);
        m_loadingImageWid->hide();
        m_loadingImageWid->setFixedHeight(m_loadingImageText->height() + 56);
    }
    m_isLoading = true;
    m_showImageWidget->hide();
    if (m_dragWidget != nullptr) {
        this->m_dragWidget->hide();
    }
    this->m_loadingImageMovie->start();
    this->m_loadingImageWid->show();
    qWarning()<<"3 showLoadingImageWid()函数 加载中界面show time = "<<startTime.elapsed()<<" ms";
    this->m_loadingImageWid->move(int((this->width() - m_loadingImageWid->width()) / 2),
                            int((this->height() - m_loadingImageWid->height()) / 2));
}

void KyView::slotIntelHVModeChanged(deviceMode sig)
{
    hLayoutFlag = hOrVMode->defaultModeCapture();
    this->setWindowSize();
    this->setTitleBtnHide();
}

void KyView::initInteraction()
{
    connect(Interaction::getInstance(), &Interaction::startWithOpenImage, this, &KyView::startWithOpenImage);
    connect(Interaction::getInstance(), &Interaction::fullScreen, this, &KyView::fullScreen);
    connect(Interaction::getInstance(), &Interaction::openFromAlbum, this, &KyView::titleOpenImagePath);
    connect(Interaction::getInstance(), &Interaction::showLoadingImageWid, this, &KyView::showLoadingImageWid);
}

//信号和槽
void KyView::initconnect()
{
    //打开图片
    connect(this, &KyView::openImageSignal, m_showImageWidget, &ShowImageWidget::openImage);
    //显示"载入图片中..."
    connect(m_showImageWidget,&ShowImageWidget::showLoadingImageWid, this, &KyView::showLoadingImageWid);
    //打开图片--主界面的界面显示处理
    connect(m_showImageWidget, &ShowImageWidget::toShowImage, this, &KyView::toShowImage);

    //顶栏图片名字
    connect(m_showImageWidget, &ShowImageWidget::titleName, m_titlebar, &TitleBar::showImageName);
    //清空相册时，展示的界面
    connect(m_showImageWidget, &ShowImageWidget::clearImage, this, &KyView::clearImage);

    //重命名
    connect(m_showImageWidget, &ShowImageWidget::toRename, m_titlebar, &TitleBar::needRename);
    //重命名，显示两栏
    connect(m_showImageWidget, &ShowImageWidget::isRename, this, &KyView::startRename);
    connect(this, &KyView::albumState, m_showImageWidget, &ShowImageWidget::albumSlot);

    connect(this, &KyView::getInfor, m_showImageWidget, &ShowImageWidget::getInfor);

    //    connect(m_showImageWidget,&ShowImageWidget::toSelectHigh,m_sideBar,&SideBar::isDelete);
    connect(m_titlebar, &TitleBar::doubleClick, this, &KyView::dealDoubleClick);
    //判断打开图片成功与否和无权限删除文件，界面应相应的做出改变
    connect(m_showImageWidget, &ShowImageWidget::fileState, m_titlebar, &TitleBar::setRenameEnable);
    connect(m_showImageWidget, &ShowImageWidget::needExitCut, this, &KyView::exitCut); //退出裁剪

    connect(m_showImageWidget, &ShowImageWidget::viewExitOCRMode, this, &KyView::exitOCRMode); //退出OCR模式进入普通模式
    connect(Interaction::getInstance(), &Interaction::getTextFinish, [=](QVector<QString> result) {
        m_showImageWidget->setGetTextResult(result);
    });
    connect(m_titlebar, &TitleBar::updateImageWidName, m_showImageWidget, &ShowImageWidget::updateImagePath);
    connect(m_showImageWidget, &ShowImageWidget::doubleEventToMainWid, this, &KyView::dealMouseDouble);
    //只有非标准字号的控件才需要绑定
    connect(this, &KyView::setFontSizeSignal, this, &KyView::lable2SetFontSizeSlot);
    //背景替换
    connect(m_showImageWidget, &ShowImageWidget::initMattingConnect, this, &KyView::initMattingConnect);
    //扫描黑白件
    connect(m_showImageWidget, &ShowImageWidget::initScannerConnect, this, &KyView::initScannerConnect);
    connect(this, &KyView::needResize, m_showImageWidget, &ShowImageWidget::resizeSlot);
}

void KyView::initMattingConnect()
{
    connect(m_showImageWidget, &ShowImageWidget::toReplaceBackground, this, &KyView::enterReplaceBackgroundMode);
    connect(m_showImageWidget, &ShowImageWidget::quitReplaceBackground, this, &KyView::exitReplaceBackgroundMode);
}

//进入背景替换模式
void KyView::enterReplaceBackgroundMode()
{
    m_showImageWidget->m_replaceBackgroundWid->resize(this->width(), this->height() - titlebarHeightValue);
    m_showImageWidget->m_replaceBackgroundWid->move((this->width() - m_showImageWidget->m_replaceBackgroundWid->width()) / 2,
                                                    this->height() - m_showImageWidget->m_replaceBackgroundWid->height());
    m_isReplacing = true;
    displayMode = DisplayMode::MattingMode;
    this->toolBarHideOrShow(false);
    m_recordSidebarStateBeforeOtherOperate = m_sideBar->isVisible();
    if (m_information != nullptr) {
        m_recordInfoStateBeforeOtherOperate = m_information->isVisible();
        if (!m_information->isHidden()) {
            m_information->hide();
        }
    }
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
    }
    if (!m_showImageWidget->g_back->isHidden()) {
        m_showImageWidget->g_back->hide();
        m_showImageWidget->g_next->hide();
    }
    InteractiveQml::getInstance()->setChangeImageBtnVisible(false);
}
//退出背景替换模式
void KyView::exitReplaceBackgroundMode()
{
    m_isReplacing = false;
    displayMode = DisplayMode::NormalMode;
    this->restoreAllBallState();
    this->setFocus();
}

void KyView::initScannerConnect()
{
    connect(m_showImageWidget, &ShowImageWidget::toScanImage, this, &KyView::enterScanImageMode);
    connect(m_showImageWidget, &ShowImageWidget::quitScanImage, this, &KyView::exitScanImageMode);
}

void KyView::enterScanImageMode()
{
    m_showImageWidget->m_scanImageWid->resize(this->width(), this->height() - titlebarHeightValue);
    m_showImageWidget->m_scanImageWid->m_showImageWidSize = m_showImageWidget->m_scanImageWid->size();
    m_showImageWidget->m_scanImageWid->move((this->width() - m_showImageWidget->m_scanImageWid->width()) / 2,
                                                    this->height() - m_showImageWidget->m_scanImageWid->height());
    m_isScanning = true;
    displayMode = DisplayMode::ScannerMode;
    this->setMinimumSize(QSize(810,600));
    this->toolBarHideOrShow(false);
    m_recordSidebarStateBeforeOtherOperate = m_sideBar->isVisible();
    if (m_information != nullptr) {
        m_recordInfoStateBeforeOtherOperate = m_information->isVisible();
        if (!m_information->isHidden()) {
            m_information->hide();
        }
    }
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
    }
    if (!m_showImageWidget->g_back->isHidden()) {
        m_showImageWidget->g_back->hide();
        m_showImageWidget->g_next->hide();
    }
    InteractiveQml::getInstance()->setChangeImageBtnVisible(false);
}

void KyView::exitScanImageMode()
{
    m_isScanning = false;
    displayMode = DisplayMode::NormalMode;
    this->setMinimumSize(m_miniSize);
    this->restoreAllBallState();
    this->setFocus();
}

//展示标注工具栏
void KyView::toShowMarkWid()
{
    //标记工具栏
    if (m_markWid == nullptr) {
        m_markWid = new Marking(this);
        m_markWid->move(int((this->width() - m_markWid->width()) / 2), this->height() - m_markWid->height() - 16);
        //展示画笔设置小窗口
        connect(m_markWid, &Marking::showSetBar, this, &KyView::toShowBrushWid);
        //设置画笔设置小窗口的位置和小三角位置--保持与按钮一一对应
        connect(m_markWid, &Marking::showSetPos, this, &KyView::toSetBrushPos);
        connect(m_markWid, &Marking::undoSignal, this, &KyView::undoOperate);
        connect(m_markWid, &Marking::exitSign, this, &KyView::exitSign);
        connect(m_markWid, &Marking::finishSign, this, &KyView::finshSign);
    }
    //画笔设置弹窗
    if (m_brushSettings == nullptr) {
        m_brushSettings = new BrushSettings(this);
        m_brushSettings->hide();
    }
    m_recordSidebarStateBeforeOtherOperate = m_sideBar->isVisible();
    if (m_information != nullptr) {
        m_recordInfoStateBeforeOtherOperate = m_information->isVisible();
        if (!m_information->isHidden()) {
            m_information->hide();
            m_inforState = true;
        }
    }
    displayMode = DisplayMode::SignMode;
    Interaction::getInstance()->startMark();
    InteractiveQml::getInstance()->setPainterType(-1);
    m_markWid->clearButtonState();
    m_markWid->show();
    this->toolBarHideOrShow(false);
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
        m_albumState = true;
    } else {
        m_albumState = false;
    }

    if (!m_showImageWidget->g_back->isHidden()) {
        m_showImageWidget->g_back->hide();
        m_showImageWidget->g_next->hide();
    }
    InteractiveQml::getInstance()->setChangeImageBtnVisible(false);
    m_titlebar->hide();
    m_toolbar->hide();
}

void KyView::exitSign()
{
    m_markWid->hide();
    m_brushSettings->hide();
    toolBarHideOrShow(true);
    if (!m_recordSidebarStateBeforeOtherOperate) {
        m_sideBar->hide();
    } else {
        m_sideBar->show();
    }
    m_titlebar->show();
    //给showimage界面发信号，去检查是否需要弹窗提示，进行下一步的操作
    m_showImageWidget->exitSign();
    Interaction::getInstance()->markClear();
    displayMode = DisplayMode::NormalMode;
    this->restoreAllBallState();
    if (m_showImageWidget->g_buttonState == false) {
        InteractiveQml::getInstance()->setChangeImageBtnVisible(false);
    } else {
        if (m_markWid->isHidden()) {
            InteractiveQml::getInstance()->setChangeImageBtnVisible(true);
        }
    }
}

void KyView::finshSign()
{
    displayMode = DisplayMode::NormalMode;
    m_markWid->hide();
    m_brushSettings->hide();
    //    qDebug() << "完成标注";
    //给showimage界面发出信号，去检查是否需要弹窗提示，进行下一步的操作
    m_showImageWidget->finishSign();
    this->restoreAllBallState();
    if (m_showImageWidget->g_buttonState == false) {
        InteractiveQml::getInstance()->setChangeImageBtnVisible(false);
    } else {
        if (m_markWid->isHidden()) {
            InteractiveQml::getInstance()->setChangeImageBtnVisible(true);
        }
    }
}

void KyView::undoOperate()
{
    //    qDebug() << "撤销操作";
    //给showimage界面发出信号，撤销上一步的操作
    m_showImageWidget->undoOperate();
    Interaction::getInstance()->markUndo();
}

void KyView::exitSignComplete()
{
    //    qDebug() << "真正的退出了标注";
    // core::发送给前端显示的图片
    // showimagewid::只有在图片其实什么都没有被改变的时候，调用这个函数，其余情况需要由后端来给前端发信号
    //    Variable::g_allowChangeImage = true;
    //    Variable::g_needImageSaveTip = false;
    //    m_markWid->hide();
}
//标注工具栏位置变化
void KyView::markChange()
{
    if (m_markWid == nullptr) {
        return;
    }
    m_markWid->move(int((this->width() - m_markWid->width()) / 2), this->height() - m_markWid->height() - 16);
    m_recordMarkNowPos = m_markWid->pos();
    m_recordMarkMovePos = m_recordMarkNowPos - m_recordMarkOldPos;
}
//展示画笔设置小弹窗
void KyView::toShowBrushWid(bool isShow, bool isText)
{
    if (isShow) {
        m_brushSettings->setSize(isText);
        m_brushSettings->show();
    } else {
        m_brushSettings->hide();
    }
}
//设置画笔设置小弹窗的小三角起始位置和弹出位置--与按钮一一对应
void KyView::toSetBrushPos(int startPos, int showPosX)
{
    m_recordBrushStartPos = startPos;
    m_brushSettings->setStartPos(startPos);
    m_brushSettings->setShowPos(m_markWid->x() - 95 + showPosX, m_markWid->y() - 6 - m_brushSettings->height());
    m_recordMarkOldPos = m_markWid->pos();
}
//画笔设置小窗口位置变化
void KyView::brushSettingsChange()
{
    if (m_brushSettings == nullptr) {
        return;
    }
    m_brushSettings->setStartPos(m_recordBrushStartPos);
    m_brushSettings->setShowPos(m_brushSettings->x() + m_recordMarkMovePos.x(),
                                m_brushSettings->y() + m_recordMarkMovePos.y());
}

//打开首先检测是否需要展示工具栏
void KyView::openState()
{
    if (m_dragWidget == nullptr || m_toolbar == nullptr) {
        return;
    }
    if (!m_dragWidget->isHidden()) {
        m_toolbar->hide();
        m_toolbar->move(int((this->width() - m_toolbar->width()) / 2), this->height() - m_toolbar->height() + 4 - 10);
    }
}
//打开关于，两栏隐藏
void KyView::aboutShow()
{
    if ((m_dragWidget != nullptr && m_dragWidget->isHidden())
        || (m_showImageWidget != nullptr && !m_showImageWidget->isHidden())) {
        if (!m_titlebar->isHidden()) {
            m_titlebar->hide();
            m_toolbar->hide();
        }
        if (m_information != nullptr) {
            if (!m_information->isHidden()) {
                if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
                    m_information->move(0, 0);
                } else {
                    m_information->move(this->width() - m_information->width(), 0);
                }
            }
        }
    }

    m_aboutDialog = new KAboutDialog(this);
    m_aboutDialog->setObjectName("Kylin-photo-viewer_KAboutDialog_about");
    m_aboutDialog->setAccessibleName("Kylin-photo-viewer_KAboutDialog_about");
    m_aboutDialog->setAccessibleDescription("about dialog");
    m_aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
    m_aboutDialog->setAttribute(Qt::WA_AlwaysShowToolTips);
    m_aboutDialog->setBodyTextVisiable(true);
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(m_aboutDialog);
    windowHelper->removeTitleBar();
    this->initAboutDialog();
    m_aboutDialog->move(this->width() - m_aboutDialog->width() / 2,
                        this->width() - m_aboutDialog->height() / 2);
    windowHelper->setSkipTaskBar(true);
    windowHelper->setSkipSwitcher(true);
    m_aboutDialog->exec();
}


void KyView::callUserManual(QString appName)
{
    if (appName.isEmpty()) {
        return;
    }

    const QString dbusServiceName = QString("com.kylinUserGuide.hotel") + QString("_") + QString::number(getuid());

    QDBusMessage message =
        QDBusMessage::createMethodCall(dbusServiceName, "/", "com.guide.hotel", "showGuide");

    QList<QVariant> args;
    args << appName;
    message.setArguments(args);

    QDBusMessage ret = QDBusConnection::sessionBus().call(message);
    if (ret.type() == QDBusMessage::InvalidMessage || ret.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "user manual call fail!";
    }
}

void KyView::startRename()
{
    if (m_markWid && !m_markWid->isHidden()) {
        m_titlebar->show();
        this->toolBarHideOrShow(false);
        return;
    }
    m_titlebar->show();
    this->toolBarPostionSet();
    this->toolBarHideOrShow(true);
}
//顶栏大小随主界面大小改变的响应函数
void KyView::titlebarChange()
{
    if (m_titlebar == nullptr) {
        return;
    }
    m_titlebar->move(0, 0);
    m_titlebar->resize(this->width(), titlebarHeightValue);
    m_titlebar->g_titleWid->resize(this->width(), titlebarHeightValue);
}
//中间打开图片按钮大小随主界面大小改变的响应函数
void KyView::openImageChange()
{
    if (m_dragWidget == nullptr) {
        return;
    }
    if (m_dragWidget->isHidden()) {
        return;
    }
    m_dragWidget->move(int((this->width() - m_dragWidget->width()) / 2),
                      int((this->height() - m_dragWidget->height()) / 2));
}
//主界面展示的图片大小随主界面大小改变的响应函数
void KyView::showImageChange()
{
    if (m_showImageWidget == nullptr) {
        return;
    }
    m_showImageWidget->move(0, 0);
    m_showImageWidget->resize(this->width(), this->height());
    m_showImageWidget->reMove(this->width(), this->height());
}

void KyView::mattingChange()
{
    m_showImageWidget->m_replaceBackgroundWid->move(0, 0);
    m_showImageWidget->m_replaceBackgroundWid->resize(this->width(), this->height() - titlebarHeightValue);
    m_showImageWidget->m_replaceBackgroundWid->move((this->width() - m_showImageWidget->m_replaceBackgroundWid->width()) / 2,
                                                    this->height() - m_showImageWidget->m_replaceBackgroundWid->height());
    m_showImageWidget->m_replaceBackgroundWid->reMove(this->width(), this->height() - titlebarHeightValue);
}

void KyView::scannerChange()
{
    m_showImageWidget->m_scanImageWid->move(0, 0);
    m_showImageWidget->m_scanImageWid->resize(this->width(), this->height() - titlebarHeightValue);
    m_showImageWidget->m_scanImageWid->move((this->width() - m_showImageWidget->m_scanImageWid->width()) / 2,
                                                    this->height() - m_showImageWidget->m_scanImageWid->height());
}
//工具栏大小随主界面大小改变的响应函数
void KyView::toolbarChange()
{
    if (m_toolbar == nullptr) {
        return;
    }
    if (m_toolbar->isHidden()) {
        return;
    }
    m_toolbar->move(int((this->width() - m_toolbar->width()) / 2), this->height() - m_toolbar->height() + 4 - 10);
}
//信息栏随顶栏位置变化而变化
void KyView::inforChange()
{
    if (m_information == nullptr) {
        return;
    }
    if (m_information->isHidden()) {
        return;
    }
    if (m_titlebar->isHidden()) {
        if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
            m_information->move(0, 0);
        } else {
            m_information->move(this->width() - m_information->width(), 0);
        }
    } else {
        if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
            m_information->move(0, titlebarHeightValue);
        } else {
            m_information->move(this->width() - m_information->width(), titlebarHeightValue);
        }
    }
}

void KyView::albumChange()
{
    if (m_sideBar == nullptr) {
        return;
    }
    if (m_sideBar->isHidden()) {
        m_showImageWidget->g_back->move(LEFT_POS, m_showImageWidget->g_next->y());
        return;
    }
    if (m_sideBar->isHidden()) {
        InteractiveQml::getInstance()->setBackBtnPos(LEFT_POS);
        return;
    }
    if (m_titlebar != nullptr && m_sideBar != nullptr && m_toolbar != nullptr) {
        Variable::g_acturalWidHeight = this->height() - m_titlebar->height() - TOOLBAR_SIZE.height() - 10 - 6 - 50;
        if (m_backWidHeight != Variable::g_acturalWidHeight) {
            m_backWidHeight = Variable::g_acturalWidHeight;
            m_sideBar->setItemNum(Variable::g_acturalWidHeight);
            m_sideBar->dealScrollbarMove(1);
        }
    }
    albumPosChange();
}

void KyView::albumPosChange()
{
    if (m_sideBar == nullptr) {
        return;
    }
    if (m_sideBar->isHidden()) {
        m_showImageWidget->g_back->move(LEFT_POS, m_showImageWidget->g_next->y());
        return;
    }
    if (m_sideBar->isHidden()) {
        InteractiveQml::getInstance()->setBackBtnPos(LEFT_POS);
        return;
    }
    if (Variable::g_needChangeAlbumPos) {
        if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
            m_sideBar->move(this->width() - m_sideBar->width(), (this->height() - m_titlebar->height() - m_toolbar->height() - 6 - m_sideBar->height()) / 2
                                   + m_titlebar->height());
        } else {
            m_sideBar->move(0, (this->height() - m_titlebar->height() - m_toolbar->height() - 6 - m_sideBar->height()) / 2
                                   + m_titlebar->height());
        }
    } else {
        if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
            m_sideBar->move(this->width() - m_sideBar->width(), 39 + m_titlebar->height());
        } else {
            m_sideBar->move(0, 39 + m_titlebar->height());
        }
    }
    m_showImageWidget->g_back->move(m_sideBar->x() + m_sideBar->width() + 42, m_showImageWidget->g_next->y());
    InteractiveQml::getInstance()->setBackBtnPos(m_sideBar->x() + m_sideBar->width() + LEFT_POS);
}

//鼠标离开界面时需要触发，届时会加上对导航器的处理
void KyView::delayHide_navi()
{
    if (displayMode == DisplayMode::OCRMode
            || displayMode == DisplayMode::SignMode
            || displayMode == DisplayMode::MattingMode
            || displayMode == DisplayMode::ScannerMode) {
        return;
    } else if (!m_titlebar->g_myEdit->isHidden() /*|| !m_toolbar->m_editMenu->isHidden()*/) {
        m_titlebar->show();
        this->toolBarPostionSet();
        this->toolBarHideOrShow(true);
        return;
    }
    m_titlebar->hide();
    this->toolBarHideOrShow(false);
    m_sideBar->hide();
    m_showImageWidget->g_next->hide();
    m_showImageWidget->g_back->hide();
    inforChange();
    m_timernavi->stop();
}

//展示信息栏
void KyView::showInforWid()
{
    if (m_information == nullptr) {
        //信息栏
        m_information = new Information(this);
        //计算默认大小
        int detio = 1;
        if (m_local.system().name() == "en_US") {
            m_information->resize((INFOR_SIZE.width() + 7) * detio, INFOR_SIZE.height() * detio);
        } else if (m_local.system().name() == "zh_CN") {
            m_information->resize(INFOR_SIZE * detio);
        } else {
            m_information->resize((INFOR_SIZE.width() + 7) * detio, INFOR_SIZE.height() * detio);
        }
        m_inforSize = m_information->size();
        //设置当前大小(m_detio)
        if (m_local.system().name() == "en_US") {
            m_information->resize((INFOR_SIZE.width() + 7) * m_detio, INFOR_SIZE.height() * m_detio);
        } else if (m_local.system().name() == "zh_CN") {
            m_information->resize(INFOR_SIZE * m_detio);
        } else {
            m_information->resize((INFOR_SIZE.width() + 7) * m_detio, INFOR_SIZE.height() * m_detio);
        }

        if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
            m_information->move(0, titlebarHeightValue);
        } else {
            m_information->move(this->width() - m_information->width(), titlebarHeightValue);
        }
        m_information->setMouseTracking(true);
        m_information->hide();
        m_information->installEventFilter(this);
        initGsetting();

        //更新信息栏
        connect(m_titlebar, &TitleBar::updateInformation, m_information, &Information::updateName);
        connect(this, &KyView::changeInforSize, m_information, &Information::changeEverySize);
        //获取图片详细信息
        connect(m_showImageWidget, &ShowImageWidget::changeInfor, m_information, &Information::contentText);
        Q_EMIT getInfor();
    }
    if (m_information->isHidden()) {
        m_information->show();
        m_inforState = false;
        inforChange();
    } else {
        m_information->hide();
        m_inforState = true;
    }
}
//列表为空-清空图片--回到默认的打开界面
void KyView::clearImage()
{
    onlyStartup = true;
    if (m_timernavi->isActive()) {
        m_timernavi->stop();
    }
    initOpenImage();
    m_dragWidget->show();
    m_showImageWidget->hide();
    m_toolbar->hide();
    if (m_information != nullptr) {
        m_information->hide();
    }
    m_titlebar->show();
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
    }
    m_titlebar->g_imageName->clear();
    m_isCutting = false;                   //重置裁剪标志
    displayMode = DisplayMode::NormalMode; //返回普通模式
    openImageChange();
}

void KyView::dealDoubleClick(bool isleaveOrNot)
{
    m_isLeave = isleaveOrNot;
}
//进入裁剪模式
void KyView::hideTwoBar()
{
    m_recordSidebarStateBeforeOtherOperate = m_sideBar->isVisible();
    if (m_information != nullptr) {
        m_recordInfoStateBeforeOtherOperate = m_information->isVisible();
        if (!m_information->isHidden()) {
            m_information->hide();
            m_inforState = true;
        }
    }
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
        m_albumState = true;
    } else {
        m_albumState = false;
    }
    m_showImageWidget->g_back->hide();
    m_showImageWidget->g_next->hide();
    m_titlebar->hide();
    m_toolbar->hide();
    m_isCutting = true;
    displayMode = DisplayMode::NormalMode; //返回普通模式
    this->setFixedSize(this->size());
}

void KyView::exitCut()
{
    displayMode = DisplayMode::NormalMode;
    toolBarHideOrShow(true);
    if (!m_recordSidebarStateBeforeOtherOperate) {
        m_sideBar->hide();
    } else {
        m_sideBar->show();
    }
    m_titlebar->show();
    this->restoreAllBallState();
    if (m_showImageWidget->g_buttonState == false) {
        m_showImageWidget->g_back->hide();
        m_showImageWidget->g_next->hide();
    } else {
        m_showImageWidget->g_back->show();
        m_showImageWidget->g_next->show();
    }
    m_isCutting = false;
    this->setFixedSize(WIDGETSIZE_MAX, WIDGETSIZE_MAX);
    this->setMinimumSize(m_miniSize);
    this->setFocus();
    if (m_mousePress) {
        m_mousePress = false;
    }
}
void KyView::viewOCRMode()
{
    if (m_information != nullptr) {
        m_recordInfoStateBeforeOtherOperate = m_information->isVisible();
        if (!m_information->isHidden()) {
            m_information->hide();
        }
    }
    m_recordSidebarStateBeforeOtherOperate = m_sideBar->isVisible();
    displayMode = DisplayMode::OCRMode;
    if (!m_sideBar->isHidden()) {
        m_sideBar->hide();
        m_albumState = true;
    }
    m_showImageWidget->g_back->hide();
    m_showImageWidget->g_next->hide();
    m_showImageWidget->startGetText();
    m_toolbar->hide();
    this->resize(this->size());
}

void KyView::exitOCRMode()
{
    displayMode = DisplayMode::NormalMode;
    this->restoreAllBallState();
    if (m_showImageWidget->g_buttonState == false) {
        m_showImageWidget->g_back->hide();
        m_showImageWidget->g_next->hide();
    } else {
        m_showImageWidget->g_back->show();
        m_showImageWidget->g_next->show();
    }
}

void KyView::needExitApp()
{
    if (m_showImageWidget != nullptr) {
        m_showImageWidget->needExitApp();
    }
}

void KyView::setSidebarState(int num)
{
    if (!m_recordFirstOpen) {
        return;
    }
    m_recordFirstOpen = false;
    int sideBarState = Variable::getSettings("sidebarState").toInt();
    if (sideBarState == 0) {
        if (num > 2) {
            m_sideBar->show();
        } else {
            m_sideBar->hide();
        }
    } else if (sideBarState == 1) {
        m_sideBar->show();
    } else if (sideBarState == 2) {
        m_sideBar->hide();
    }
}

void KyView::getMouseStateFromQml()
{
    if (m_isLoading) {
        return;
    }
    if (m_timernavi->isActive()) {
        m_timernavi->stop();
    }
    if (m_toolbar->isHidden()) {
        if (displayMode == DisplayMode::NormalMode
                || displayMode == DisplayMode::OCRMode || displayMode == DisplayMode::MattingMode || displayMode == DisplayMode::ScannerMode) {
            if (m_titlebar->isHidden()) {
                m_titlebar->raise();
                m_titlebar->show();
            }
        }
        if (displayMode == DisplayMode::NormalMode) {
            if (m_toolbar->isHidden()) {
                this->toolBarPostionSet();
                this->toolBarHideOrShow(true);
            }
        }
        m_clickController = true;
    } else {
        if (!m_toolbar->isHidden()) {
            this->toolBarHideOrShow(false);
        }
        if (displayMode != DisplayMode::OCRMode || displayMode == DisplayMode::MattingMode || displayMode == DisplayMode::ScannerMode) {
            if (!m_titlebar->isHidden()) {
                m_titlebar->hide();
            }
        }
        m_clickController = false;
    }
    this->inforChange();
}

//读取主题配置文件
void KyView::initGsetting()
{
    if (QGSettings::isSchemaInstalled(Variable::FITTHEMEWINDOW.toLocal8Bit())) {
        m_pGsettingThemeData = new QGSettings(Variable::FITTHEMEWINDOW.toLocal8Bit());
        connect(m_pGsettingThemeData,&QGSettings::changed,this,[=](const QString &key) {
            if (key == "styleName") {
                themeChange();
            }
            if (key == "systemFont" || key == "systemFontSize") {
                changeFontSize();
            }
        });
    }
    if (QGSettings::isSchemaInstalled(Variable::FITTHEMEWINDOW.toLocal8Bit())) {
        m_pGsettingTransparency = new QGSettings(Variable::FITTRANSPARENCY.toLocal8Bit());
        connect(m_pGsettingTransparency,&QGSettings::changed,this,[=](const QString &key) {
            if (key == "transparency") {
                transChange();
            }
        });
    }
    themeChange();
    transChange();
    changeFontSize();
    return;
}
//主题变化
void KyView::themeChange()
{
    QString nowThemeStyle = m_pGsettingThemeData->get("styleName").toString();
    if (m_showImageWidget != nullptr) {
        InteractiveQml::getInstance()->setChangeImageBtnBg(nowThemeStyle);
    }
    if ("ukui-dark" == nowThemeStyle || "ukui-black" == nowThemeStyle) {
        if (m_information != nullptr) {
            m_information->setStyleSheet(
                "QWidget{background-color:rgba(0,0,0,0.80);border-top-left-radius:0px;border-top-right-radius：0px;border-"
                "bottom-left-radius:4px;border-bottom-right-radius:0px;}");
        }
        if (m_showImageWidget != nullptr) {
            m_showImageWidget->g_next->setIcon(QIcon(":/res/res/1right.svg"));
            m_showImageWidget->g_back->setIcon(QIcon(":/res/res/1left.svg"));
        }

        if (m_sideBar != nullptr) {
            m_sideBar->setStyleSheet(
                "QListView{border:1px "
                ";border-top-left-radius:0px;border-top-right-radius:4px;border-bottom-left-radius:"
                "0px;border-bottom-right-radius:4px;outline:none;background:rgba(63, 69, 77, 1)}");
        }

    } else {
        if (m_information != nullptr) {
            m_information->setStyleSheet("QWidget{background-color:rgba(225, 240, 250, "
                                         "0.85);border-top-left-radius:0px;border-top-right-radius：0px;border-bottom-left-"
                                         "radius:4px;border-bottom-right-radius:0px;}");
        }

        if (m_showImageWidget != nullptr) {
            m_showImageWidget->g_next->setIcon(QIcon(":/res/res/right.svg"));
            m_showImageWidget->g_back->setIcon(QIcon(":/res/res/left.svg"));
        }

        if (m_sideBar != nullptr) {
            m_sideBar->setStyleSheet(
                "QListView{border:1px "
                ";border-top-left-radius:0px;border-top-right-radius:4px;border-bottom-left-radius:0px;border-bottom-right-"
                "radius:4px;outline:none;background:rgba(227, 235, 239, 1)}");
        }
    }
    if (!m_activateStatus && m_windowHelper) {
        m_backColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-inactive");
        m_backColor.setAlpha(255);
        m_titlebar->setWindowDeactivate();
    }

    Interaction::getInstance()->changeOpenIcon(nowThemeStyle);
    if (m_toolbar != nullptr) {
        m_toolbar->changeStyle();
    }
    if (m_markWid != nullptr) {
        m_markWid->changeStyle();
    }
    if (m_brushSettings != nullptr) {
        m_brushSettings->setTextSettings();
    }
    if (m_showImageWidget != nullptr) {
        m_showImageWidget->cutBtnWidChange(nowThemeStyle);
    }
}
void KyView::transChange()
{
    m_tran = m_pGsettingTransparency->get("transparency").toDouble();
    this->update();
}
void KyView::lable2SetFontSizeSlot(double size)
{
    //默认大小12px,换算成pt为9
    double lable2BaseFontSize = DEFAULT_FONT_SIZE;
    double nowFontSize = lable2BaseFontSize * size / DEFAULT_FONT_SIZE; // 11为系统默认大小
    m_detio = nowFontSize / lable2BaseFontSize;
    Q_EMIT changeInforSize(nowFontSize, m_detio, m_inforSize);
    inforChange();
}
void KyView::changeFontSize()
{
    //启动时设置字号
    double fontSizeKey = DEFAULT_FONT; //系统默认字号
    double fontSizeKeyTmp = m_pGsettingThemeData->get("systemFontSize").toDouble();
    if (fontSizeKeyTmp > 0) {
        fontSizeKey = fontSizeKeyTmp;
    }
    if (fontSizeKey > 15) {
        fontSizeKey = 15;
    }
    Q_EMIT setFontSizeSignal(fontSizeKey);
}
//全屏
void KyView::fullScreen()
{
    if (this->isFullScreen()) {
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            if(this->isMaximized() ){
                setWindowState(windowState() & ~Qt::WindowFullScreen);
            }else{
                this->showNormal();
            }
        } else {
            KWindowSystem::clearState(winId(), NET::FullScreen);
        }
        m_nowFullScreen = false;

    } else {
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            setWindowState(windowState() | Qt::WindowFullScreen);
        } else {
            KWindowSystem::setState(winId(), NET::FullScreen);
        }
        m_nowFullScreen = true;
    }
    m_titlebar->dealEditState();
}

void KyView::avoidChange()
{
    //解决点击侧栏，触发图片切换的问题。
    if (m_sideBar != nullptr) {
        if (!m_sideBar->isHidden() && m_sideBar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            m_panTriggered = false;
        }
    }
    //解决点击顶栏，触发图片切换的问题。
    if (m_titlebar != nullptr) {
        if (!m_titlebar->isHidden() && m_titlebar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            m_panTriggered = false;
        }
    }
    //解决点击左右按钮，触发图片切换的问题。
    if (m_showImageWidget != nullptr) {
        if (!m_showImageWidget->g_back->isHidden()
            && (m_showImageWidget->g_back->geometry().contains(this->mapFromGlobal(QCursor::pos()))
                || m_showImageWidget->g_next->geometry().contains(this->mapFromGlobal(QCursor::pos())))) {
            m_panTriggered = false;
        }
    }
    //解决点击工具栏，触发图片切换的问题。
    if (m_toolbar != nullptr) {
        if (!m_toolbar->isHidden() && m_toolbar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            m_panTriggered = false;
        }
    }
}

void KyView::markToolChange()
{
    if (m_markWid != nullptr) {
        if (!m_markWid->isHidden()) {
            m_markWid->move(int((this->width() - m_markWid->width()) / 2), this->height() - m_markWid->height() - 16);
        }
    }
}

//最大化和还原
void KyView::changOrigSize()
{
    if (this->isFullScreen() || m_nowFullScreen) {  //全屏恢复
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            if(this->isMaximized() ){
                setWindowState(windowState() & ~Qt::WindowFullScreen);
            }else{
                this->showNormal();
            }
        } else {
            KWindowSystem::clearState(winId(), NET::FullScreen);
        }
        m_nowFullScreen = false;

    } else if (this->isMaximized()) { //最大化恢复
        if(QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)){
            this->showNormal();
        }else{
            this->showNormal();
        }
    } else {    //最大化
        if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            this->showMaximized();
        }else{
            this->showMaximized();
        }

    }
    m_titlebar->dealEditState();
}

//显示图片--主界面的界面显示
void KyView::toShowImage()
{
    m_isLoading = false;
    if (m_dragWidget != nullptr) {
        this->m_dragWidget->hide();
    }
    this->m_loadingImageWid->hide();
    if (m_loadingImageMovie->state() != QMovie::NotRunning) {
        m_loadingImageMovie->stop();
    }
    this->m_showImageWidget->show();
    if (m_showImageWidget->g_buttonState == false) {
        m_showImageWidget->g_back->hide();
        m_showImageWidget->g_next->hide();
    } else {
        m_showImageWidget->g_back->show();
        m_showImageWidget->g_next->show();
    }
    this->m_titlebar->show();
    this->m_showImageWidget->lower();
    if (displayMode == DisplayMode::OCRMode || displayMode == DisplayMode::SignMode) {
        this->toolBarHideOrShow(false);
        if (displayMode == DisplayMode::SignMode) {
            m_titlebar->hide();
        }
        m_showImageWidget->g_back->hide();
        m_showImageWidget->g_next->hide();
    } else {
        this->toolBarHideOrShow(true);
    }
    inforChange();
    albumChange();
}
//显示相册
void KyView::showSidebar()
{
    if (m_sideBar == nullptr) {
        return;
    }
    if (m_sideBar->isHidden()) {
        m_sideBar->show();
        Variable::setSettings("sidebarState", 1);
        Q_EMIT albumState(true);
        this->toolBarPostionSet();
        albumChange();
        m_albumState = false;
    } else {
        m_sideBar->hide();
        Variable::setSettings("sidebarState", 2);
        m_showImageWidget->g_back->move(LEFT_POS, m_showImageWidget->g_next->y());
        InteractiveQml::getInstance()->setBackBtnPos(LEFT_POS);
        Q_EMIT albumState(false);
        m_albumState = true;
        this->toolBarPostionSet();
    }
}
//先保留--默认打开相册
// void KyView::defaultSidebar()
//{
//    m_sideBar->show();
//    Q_EMIT albumState(true);
//    albumChange();
//    m_albumState = false;
//}
//检测鼠标位置--顶栏和工具栏的出现和隐藏
void KyView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_showImageWidget && !m_isSuppoortOpen) {
        m_showImageWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        m_isSuppoortOpen = true;
    }
    //手指长按时，过滤2像素差的鼠标移动事件
    if (qAbs(event->pos().x() - m_pressPosRecord.x()) < 2 && qAbs(event->pos().y() - m_pressPosRecord.y()) < 2) {
        return;
    }
    if (displayMode == DisplayMode::NormalMode) {
        //鼠标移入特定区域显示/隐藏控件
        if (m_dragWidget != nullptr) {
            if (!m_dragWidget->isHidden() && m_isLoading) {
                this->toolBarHideOrShow(false);
                m_titlebar->show();
            }
        }

    } else if (displayMode == DisplayMode::OCRMode || displayMode == DisplayMode::MattingMode || displayMode == DisplayMode::ScannerMode) {
        this->toolBarHideOrShow(false);
        m_titlebar->show();
    }
    //    }
    //窗口拖拽
    if (m_titlebar == nullptr) {
        return;
    }
    if (m_titlebar->isHidden() || !m_titlebar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        return;
    }
    if (!m_mousePress) {
        return;
    }
    double device = QGuiApplication::primaryScreen()->devicePixelRatio();
    if (device <= 0) {
        device = 1;
    }
    QPoint pos = this->mapToGlobal(event->pos());

    //处理触摸屏-拖动后，触摸释放信号不被发出问题-如果是触摸
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果没有正在捕获鼠标
        if (!this->mouseGrabber()) {
            //抓取鼠标
            this->grabMouse();
            //进行释放--避免是触摸的时候，事件一直无法被释放，无法进行后续拖拽点击等事件
            this->releaseMouse();
        }
    }
    QString platform = QGuiApplication::platformName();
    if (!platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        XEvent xEvent;
        memset(&xEvent, 0, sizeof(XEvent));

        Display *display = QX11Info::display();
        xEvent.xclient.type = ClientMessage;
        xEvent.xclient.message_type = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
        xEvent.xclient.display = display;
        xEvent.xclient.window = (XID)(this->winId());
        xEvent.xclient.format = 32;
        xEvent.xclient.data.l[0] = pos.x() * device;
        xEvent.xclient.data.l[1] = pos.y() * device;
        xEvent.xclient.data.l[2] = 8;
        xEvent.xclient.data.l[3] = Button1;
        xEvent.xclient.data.l[4] = 1;

        XUngrabPointer(display, CurrentTime);
        XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()), False,
                   SubstructureNotifyMask | SubstructureRedirectMask, &xEvent);
        XFlush(display);
    } else {
        hLayoutFlag = hOrVMode->defaultModeCapture();
        if (hLayoutFlag != deviceMode::PCMode) {
            return;
        }else{
            this->windowHandle()->startSystemMove();
            leftPoint = leftPoint + (event->globalPos() - m_dragPos);
            //kdk::WindowManager::setGeometry(this->windowHandle(),QRect(leftPoint, QSize(this->width(), this->height())));
        }
	}
	

    event->accept();
}
void KyView::mousePressEvent(QMouseEvent *event)
{
    //只能是鼠标左键移动和改变大小
    if (event->button() == Qt::LeftButton) {
        m_mousePress = true;
        this->m_dragPos = event->globalPos();
    }
    m_pressPosRecord = event->pos();
}

void KyView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_mousePress = false;
    m_pressPosRecord = QPoint(0, 0);
}

//拖拽主窗口--各控件需要改变位置或尺寸
void KyView::resizeEvent(QResizeEvent *event)
{
    titlebarChange();
    openImageChange();
    showImageChange();
    toolbarChange();
    inforChange();
    albumChange();
    markChange();
    brushSettingsChange();
    if (m_isReplacing) {
        mattingChange();
    }
    if (m_isScanning) {
        scannerChange();
    }
    if (m_titlebar != nullptr) {
        if (this->isMaximized() || this->isFullScreen() || m_nowFullScreen) {
            m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的全屏图标
            m_titlebar->g_fullscreen->setToolTip(tr("recovery"));
        } else {
            m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-maximize-symbolic")); //主题库的恢复图标
            m_titlebar->g_fullscreen->setToolTip(tr("full srceen"));
        }
    }
    if (m_dragWidget != nullptr) {
        m_dragWidget->setFixedSize(this->width() / 3 * 2, this->height() / 5 * 3);
        m_dragWidget->move(int((this->width() - m_dragWidget->width()) / 2),
                           int((this->height() - m_dragWidget->height()) / 2));
    }
    if (m_loadingImageWid != nullptr) {
        m_loadingImageWid->move(int((this->width() - m_loadingImageWid->width()) / 2),
                                int((this->height() - m_loadingImageWid->height()) / 2));
    }
    if(displayMode == DisplayMode::MattingMode || displayMode == DisplayMode::ScannerMode) {
        return;
    }
    Variable::g_widSize = this->size();
    Q_EMIT needResize();
}
//鼠标离开事件
void KyView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_titleState = false;
    //点出顶栏的下拉菜单时，不隐藏
    if (!m_titlebar->isHidden() && !m_titlebar->g_menu->m_menu->isHidden() || displayMode == DisplayMode::OCRMode) {
        return;
    }
    if (!onlyStartup) {
        if (!m_timernavi->isActive()) {
            if (!m_clickController) {
                m_timernavi->start(2500);
            }
        }
    }
}
//鼠标进入事件
void KyView::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_titleState = true;
    if (m_timernavi->isActive()) {
        m_timernavi->stop();
    }
    if (m_isLoading) {
        return;
    }
    if (displayMode == DisplayMode::OCRMode
            || displayMode == DisplayMode::SignMode
            || displayMode == DisplayMode::CuttingMode
            || displayMode == DisplayMode::MattingMode
            || displayMode == DisplayMode::ScannerMode) {
        return;
    }
    if (!onlyStartup){
        toolBarHideOrShow(true);
        m_titlebar->show();
        if (Variable::getSettings("sidebarState").toInt() == 2 || !Variable::g_sidebarState) {
            m_sideBar->hide();
        } else {
            m_sideBar->show();
        }
    }
    if (m_sideBar != nullptr) {
        if (m_sideBar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            m_showImageWidget->g_back->show();
            m_showImageWidget->g_next->show();
        }
    }

    if (m_showImageWidget != nullptr) {
        if (m_showImageWidget->g_buttonState == false) {
            m_showImageWidget->g_back->hide();
            m_showImageWidget->g_next->hide();
        } else {
            m_showImageWidget->g_back->show();
             m_showImageWidget->g_next->show();
        }
    }
    toolbarChange();
    titlebarChange();
    albumChange();
    inforChange();
}

void KyView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), m_backColor);
    if (m_showImageWidget != nullptr) {
        m_showImageWidget->changeQmlcomponentColor(m_backColor);
        if (m_isReplacing) {
            m_showImageWidget->m_replaceBackgroundWid->changeQmlColor(m_backColor);
        }
    }
    QWidget::paintEvent(event);
}

void KyView::keyReleaseEvent(QKeyEvent *event)
{
    if (displayMode == DisplayMode::NormalMode && m_showImageWidget != nullptr && !m_showImageWidget->isHidden()) {
        switch (event->key()){
        case Qt::Key_Control:
            m_keyState = "keyNone";
            break;
        case Qt::Key_Alt:
            m_keyState = "keyNone";
            break;
        default:
            break;
        }
    }

}
// F1快捷键打开用户手册
void KyView::keyPressEvent(QKeyEvent *event)
{
    // F1快捷键打开用户手册
    if (event->key() == Qt::Key_F1) {
        callUserManual("kylin-photo-viewer");
    }
    //全屏
    if (displayMode == DisplayMode::NormalMode) {
        if (event->key() == Qt::Key_F11) {
            this->fullScreen();
        }
        if (event->key() == Qt::Key_Escape) {
            if (this->isFullScreen() || m_nowFullScreen) {
                this->fullScreen();
            }
        }
    }

    // 新增快捷键功能
    if (displayMode == DisplayMode::NormalMode && m_showImageWidget != nullptr && !m_showImageWidget->isHidden()) {
        switch (event->key()){
        case Qt::Key_Control:
            m_keyState = "Ctrl";
            break;
        case Qt::Key_Alt:
            m_keyState = "Alt";
            break;
        case Qt::Key_Return:
            if (m_keyState == "Alt"){
                showInforWid();
            }
            break;
        case Qt::Key_Plus:
            if (m_keyState == "Ctrl")
                if (m_toolbar != nullptr) {
                    m_toolbar->enlargeImageSlot();
                }
            break;
        case Qt::Key_Equal:
            if (m_keyState == "Ctrl")
                if (m_toolbar != nullptr) {
                    m_toolbar->enlargeImageSlot();
                }
            break;
        case Qt::Key_Minus:
            if (m_keyState == "Ctrl")
                if (m_toolbar != nullptr) {
                    m_toolbar->reduceImageSlot();
                }
            break;
        case Qt::Key_0:
            if (m_keyState == "Ctrl")
                if (m_toolbar != nullptr) {
                    m_toolbar->adaptiveWidget();
                }
            break;
        case Qt::Key_R:
            if (m_keyState == "Ctrl")
                if (m_toolbar != nullptr) {
                    m_toolbar->rotate();
                }
            break;
        case Qt::Key_P:
            if (m_keyState == "Ctrl")
                if (m_showImageWidget != nullptr) {
                    m_showImageWidget->initPrint();
                }
            break;
        case Qt::Key_O:
            Interaction::getInstance()->openImageInfile();
            break;
        default:
                break;
        }
    }

    //上一张，下一张，delete按键响应
    if (m_dragWidget != nullptr && !m_dragWidget->isHidden()) {
        return;
    } else {
        if (displayMode == DisplayMode::NormalMode) {
            if (event->key() == Qt::Key_Delete) {
                m_toolbar->delImage();
            }
            if (event->key() == Qt::Key_Left) {
                if (m_showImageWidget != nullptr) {
                    m_showImageWidget->m_canSet = true;
                    m_showImageWidget->backImage();
                }
            }
            if (event->key() == Qt::Key_Right) {
                if (m_showImageWidget != nullptr) {
                    m_showImageWidget->m_canSet = true;
                    m_showImageWidget->nextImage();
                }
            }
            if (event->matches(QKeySequence::Copy)) {
                if (m_showImageWidget != nullptr) {
                    m_showImageWidget->copy();
                }
            }
        }
    }
    //裁剪
    if (displayMode != DisplayMode::CuttingMode) {
        return;
    } else {
        if (event->key() == Qt::Key_Escape) {
            //退出裁剪模式--弹出保存提示框
            if (m_showImageWidget != nullptr) {
                m_showImageWidget->exitCut();
                m_showImageWidget->cutFinishUi();
            }

        }
        if (event->key() == 16777220 || event->key() == Qt::Key_Enter) {
            //保存并退出裁剪模式
            if (m_showImageWidget != nullptr) {
                if (m_showImageWidget->btnWidState()) {
                    m_showImageWidget->needSave();
                }
            }
        }
    }
}
//添加左键双击事件--最大化和还原
void KyView::mouseDoubleClickEvent(QMouseEvent *event)
{
    //判断左键双击
    if (event->button() != Qt::LeftButton) {
        return;
    }
    this->dealMouseDouble();
}

//拖文件进界面
void KyView::dragEnterEvent(QDragEnterEvent *event)
{
    if (operayteMode == OperayteMode::Cut || operayteMode == OperayteMode::Ocr || operayteMode == OperayteMode::Sign) {
        return;
    }
    //获得所有可被查看的图片格式
    QStringList formatList;
    QString format;
    format = "";
    for (const QString &str : Variable::SUPPORT_FORMATS) {
        format = str;
        formatList.append(format);
    }
    //检查当前拖入的否为url，暂时只支持以url形式拖入
    if (!event->mimeData()->hasUrls()) {
        return;
    }
    //极端情况下，拖拽文件时list有可能为空，不做异常处理时，会闪退。
    QList<QUrl> list = event->mimeData()->urls();
    if (list.isEmpty()) {
        return;
    }
    QUrl url = list.first();
    if (url.isEmpty()) {
        return;
    }
    QString str = url.fileName();
    if (str.isEmpty()) {
        return;
    }
    //判断图片是否支持被查看
    if (formatList.contains(QFileInfo(str).suffix().toLower())) {
        m_isSuppoortOpen = true;
        Q_EMIT setIsSupportOpen(true);
        event->acceptProposedAction();
    } else {
        m_isSuppoortOpen = false;
        Q_EMIT setIsSupportOpen(false);
        event->ignore(); //否则不接受鼠标事件
    }
}
//放下文件之后响应
void KyView::dropEvent(QDropEvent *event)
{
    initShowImage();
    QList<QUrl> urls = event->mimeData()->urls();
    //判断是否为空
    if (urls.isEmpty()) {
        return;
    }
    //拿路径，发信号打开图片
    QString path = urls.first().toLocalFile();
    if (path == "") {
        return;
    }
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));

    QFileInfo info(path);
    Variable::setSettings("imagePath", info.absolutePath());
    m_showImageWidget->openImage(path);
}

void KyView::dealMouseDouble()
{
    //在标题栏的图片名字处双击不响应全屏或恢复
    if (m_titlebar->g_imageName->geometry().contains(this->mapFromGlobal(QCursor::pos())) && m_isLeave == true) {
        //有问题--后续需要更改
        if (m_titlebar->g_imageName->x() != 0) {
            return;
        }
    }
    if (displayMode != DisplayMode::NormalMode) {
        return;
    }
    if (!m_isResponseDbClick) {
        return;
    }
    if (m_toolbar != nullptr && m_toolbar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        return;
    }
    if (!m_titlebar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        //在信息栏处双击不响应全屏
        if (m_information != nullptr && !m_information->isHidden() && (m_information->geometry().contains(this->mapFromGlobal(QCursor::pos())))) {
            return;
        }
        //图片区域双击全屏
        this->fullScreen();

    } else if (m_titlebar->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        //标题栏处双击最大化/还原
        this->changOrigSize();
    }
}

void KyView::initGrabGesture()
{
    //手势需要关闭主题事件
    this->setProperty("useStyleWindowManager", false);
}

bool KyView::event(QEvent *event)
{
    //判断列表中是否只有一张图，一张图片左右按钮不显示--解决静止鼠标，按钮仍然显示的问题
    if (m_showImageWidget != nullptr) {
        if (m_showImageWidget->g_buttonState == false) {
            this->m_showImageWidget->g_next->hide();
            this->m_showImageWidget->g_back->hide();
        }
    }
    avoidChange();
    //使用X11接管窗口移动事件后，鼠标离开事件不触发，此处作为备用方案
    if (m_mousePress) {
        if (event->type() == QEvent::Move) {
            //本次不响应切换手势
            m_panTriggered = false;

            x11EventEnd();
        }
    }
    return QWidget::event(event);
}

bool KyView::eventFilter(QObject *obj, QEvent *event){

    if(obj == m_titlebar->g_fullscreen && event->type() == QEvent::ToolTip){
            event->setAccepted(true);
            event->accept();
            QRect btnRect = m_titlebar->g_fullscreen->geometry();
            auto pos = m_titlebar->g_fullscreen->mapTo(m_titlebar->g_fullscreen, QPoint(btnRect.width()/2, 0));
            btnRect.translate(pos.x() - 16, 0);
            m_windowHelper->showTileFlyout(btnRect);
            return true;
    }

    if (obj == this && event->type() == QEvent::WindowActivate) {
        m_backColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-normal");
        m_backColor.setAlphaF(m_tran);
        m_activateStatus = true;
    } else if (obj == this && event->type() == QEvent::WindowDeactivate) {
        if (QApplication::focusWindow()) {
            if (QApplication::focusWindow()->type() != Qt::Popup) {
                m_backColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-inactive");
                m_backColor.setAlpha(255);
                m_activateStatus = false;
            }
        } else {
            m_backColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-inactive");
            m_backColor.setAlpha(255);
            m_activateStatus = false;
        }
    }

    if (obj == m_titlebar && event->type() == QEvent::WindowActivate) {
        m_titlebar->setWindowActivate();
    } else if (obj == m_titlebar && event->type() == QEvent::WindowDeactivate) {
        if (QApplication::focusWindow()) {
            if (QApplication::focusWindow()->type() != Qt::Popup) {
                m_titlebar->setWindowDeactivate();
            }
        } else {
           m_titlebar->setWindowDeactivate();
        }
    }

    return QObject::eventFilter(obj, event);
}

void KyView::x11EventEnd()
{
    double device = QGuiApplication::primaryScreen()->devicePixelRatio();
    if (device <= 0) {
        device = 1;
    }
    QPoint globalPoints = QCursor::pos();

    QString platform = QGuiApplication::platformName();
    if (!platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        XEvent xEvent;
        memset(&xEvent, 0, sizeof(XEvent));
        Display *display = QX11Info::display();
        xEvent.type = ButtonRelease;
        xEvent.xbutton.button = Button1;
        xEvent.xbutton.window = this->effectiveWinId();
        if (globalPoints.y() < 5) {
            xEvent.xbutton.x = globalPoints.x() * device;
            xEvent.xbutton.y = globalPoints.y() * device;
        } else {
            xEvent.xbutton.x = this->mapFromGlobal(globalPoints).x() * device;
            xEvent.xbutton.y = this->mapFromGlobal(globalPoints).y() * device;
        }
        xEvent.xbutton.x_root = globalPoints.x() * device;
        xEvent.xbutton.y_root = globalPoints.y() * device;
        xEvent.xbutton.display = display;

        //在release之前，先模拟一次press操作，防止release不生效
        XEvent xEvent2 = xEvent;
        xEvent2.type = ButtonPress;
        XSendEvent(display, this->effectiveWinId(), False, ButtonPressMask, &xEvent2);
        XFlush(display);

        XSendEvent(display, this->effectiveWinId(), False, ButtonReleaseMask, &xEvent);
        XFlush(display);
        m_mousePress = false;
    }
}

// 手势识别
bool KyView::gestureEvent(QEvent *event)
{
    QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(event);
    if (QGesture *gesture = gestureEvent->gesture(Qt::PinchGesture)) {
        pinchTriggered(static_cast<QPinchGesture *>(gesture));
        return true;
    }
    if (QGesture *gesture = gestureEvent->gesture(Qt::TapAndHoldGesture)) {
        tapAndHoldGesture(static_cast<QTapAndHoldGesture *>(gesture));
        return true;
    }
    return false;
}

void KyView::pinchTriggered(QPinchGesture *gesture)
{
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags(); // 获取已改变的手势属性

    if (gesture->centerPoint() != QPointF(-1, -1)) {
        return;
    }
    if (changeFlags & QPinchGesture::RotationAngleChanged) { // 手势覆盖角度改变
        if (!m_isResize) {
            const qreal lastAngle = gesture->lastRotationAngle();
            if (touchRotate(lastAngle)) {
                m_isRotate = true;
                m_touchPoint.setX(-1); //特殊标记
            }
        }
    }

    if (changeFlags & QPinchGesture::ScaleFactorChanged) { // 两个输入点的距离改变
        if (!m_isRotate) {
            qreal scaleFactor = gesture->totalScaleFactor();
            qreal tmpNum = scaleFactor - m_lastDistance;
            QPoint zoomFocus = this->mapFromGlobal(gesture->centerPoint().toPoint());
            //避免操作太频繁，移动幅度超过某值才做处理
            if (tmpNum > m_minDistance) {
                //                Interaction::getInstance()->watchBigImage(zoomFocus);
                m_lastDistance = scaleFactor;
            } else if (tmpNum < m_minDistance * -0.5) {
                //                Interaction::getInstance()->watchSmallImage(zoomFocus);
                m_lastDistance = scaleFactor;
            }
            if (m_lastDistance != 1) {
                m_isResize = true;
                m_touchPoint.setX(-1); //特殊标记
                if (recordCurrentProportion < 150) {
                    m_minDistance = 0.4;
                } else if (recordCurrentProportion < 300) {
                    m_minDistance = 0.3;
                } else if (recordCurrentProportion < 500) {
                    m_minDistance = 0.2;
                } else {
                    m_minDistance = 0.1;
                }
            }
        }
    }

    if (gesture->state() == Qt::GestureFinished) { // 手势结束
        m_rotateLevel = 0;
        m_isRotate = false;
        m_isResize = false;
        m_lastDistance = 1;
        m_minDistance = 0.5;
        m_needCLoseGrabGesture = false;
    }
}

bool KyView::touchRotate(const qreal &lastAngle)
{
    bool result = false;
    // 最后的手势覆盖角度介于-45度和45度之间
    if (lastAngle > -45 && lastAngle < 45 && lastAngle != 0) {
        if (m_rotateLevel == -1) {
            //顺时针转到此区域
            Interaction::getInstance()->rotate();
            result = true;
        } else if (m_rotateLevel == 1) {
            //逆时针转到此区域
            Interaction::getInstance()->rotate(false);
            result = true;
        }
        //记录区域标签
        m_rotateLevel = 0;
        return result;
    }

    if (lastAngle > 45 && lastAngle < 135) {
        if (m_rotateLevel == 0) {
            Interaction::getInstance()->rotate();
            result = true;
        } else if (m_rotateLevel == 2) {
            Interaction::getInstance()->rotate(false);
            result = true;
        }
        m_rotateLevel = 1;
        return result;
    }
    if (lastAngle > 135 && lastAngle < 180) {
        if (m_rotateLevel == 1) {
            Interaction::getInstance()->rotate();
            result = true;
        }
        m_rotateLevel = 2;
        return result;
    }

    if (lastAngle < -45 && lastAngle > -135) {
        if (m_rotateLevel == 0) {
            Interaction::getInstance()->rotate(false);
            result = true;
        } else if (m_rotateLevel == -2) {
            Interaction::getInstance()->rotate();
            result = true;
        }
        m_rotateLevel = -1;
        return result;
    }
    if (lastAngle < -90 && lastAngle > -180) {
        if (m_rotateLevel == -1) {
            Interaction::getInstance()->rotate(false);
            result = true;
        }
        m_rotateLevel = -2;
        return result;
    }
    return result;
}

void KyView::setWindowSize()
{
    if (hLayoutFlag == PADHorizontalMode) {
        m_isResponseDbClick = false;
        this->resize(DEFAULT_HORIZONTAL_WIDTH, DEFAULT_HORIZONTAL_HEIGHT);
    } else if (hLayoutFlag == PADVerticalMode) {
        m_isResponseDbClick = false;
        this->resize(DEFAULT_VERTICAL_WIDTH, DEFAULT_VERTICAL_HEIGHT);
    } else {
        m_isResponseDbClick = true;
        this->resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }
}

void KyView::setTitleBtnHide()
{
    //处理标题栏，工具栏，标注栏的显示
    if (m_titlebar != nullptr) {
        m_titlebar->setBtnShowOrHide();
    }
    if (m_toolbar != nullptr) {
        m_toolbar->platformType();
        m_toolbar->initControlQss();
    }
    if (m_markWid != nullptr) {
        m_markWid->resetMarkWidQss();
    }
    if (m_brushSettings != nullptr) {
        if (!m_brushSettings->isHidden()) {
            if (m_markWid != nullptr) {
                m_brushSettings->setSize(m_markWid->getTextState());
                m_brushSettings->move(m_brushSettings->x(), m_markWid->y() - 6 - m_brushSettings->height());
            }
        }
    }
}

QString KyView::getAppVersion()
{
    FILE *file = NULL;
    char *line = NULL;
    size_t size = 0;
    ssize_t read;
    char *q = NULL;
    QString version = nullptr;
    file = popen("dpkg -l kylin-photo-viewer", "r");
    if(NULL == file)
        return version;
    while((read = getline(&line, &size, file)) != -1){
        q = strrchr(line, '\n');
        *q = '\0';
        QString content = line;
        QStringList list = content.split(" ");
        list.removeAll("");
        if (list.size() >= 3)
            version = list.at(2);
    }
    free(line);
    pclose(file);
    return version;
}

void KyView::initAboutDialog()
{
    m_aboutDialog->setAppIcon(QIcon::fromTheme("kylin-photo-viewer"));
    m_aboutDialog->setAppName(tr("Pictures"));
    if (m_locale == "kk_KZ" || m_locale =="ug_CN" || m_locale == "ky_KG") {
        m_aboutDialog->setAppVersion(getAppVersion()+tr(":Version"));
    } else {
        m_aboutDialog->setAppVersion(tr("Version:")+getAppVersion());
    }
    m_aboutDialog->setBodyText(tr("A system picture tool that can quickly open common formats. It provides zoom,flip "
                                  "and other processing simplely."));
}

void KyView::toolBarPostionSet()
{
    if (m_toolbar == nullptr) {
        return;
    }
    if (Variable::g_mainlineVersion) {
        m_toolbar->move(int((this->width() - m_toolbar->width()) / 2), this->height() - m_toolbar->height() + 4 - 10);
        return;
    }
    if (m_showImageWidget != nullptr) {
        if (!m_toolbar->isHidden()) {
            double tempPoint = m_toolbar->y() - 20;
            m_showImageWidget->setTiffToolbarPos(tempPoint);
        }
    }
}

void KyView::toolBarHideOrShow(bool needShow)
{
    if (m_toolbar == nullptr) {
        return;
    }
    if (m_showImageWidget == nullptr) {
        return;
    }
    double tempPoint;
    if (needShow) {
        m_toolbar->move(int((this->width() - m_toolbar->width()) / 2), this->height() - m_toolbar->height() + 4 - 10);
        m_toolbar->raise();
        m_toolbar->show();
        tempPoint = m_toolbar->y() - 20;
        m_showImageWidget->setTiffToolbarPos(tempPoint);
    } else {
        m_toolbar->hide();
        tempPoint = this->height() - 20;
        m_showImageWidget->setTiffToolbarPos(tempPoint);
    }
}

void KyView::restoreAllBallState()
{
    if (m_recordSidebarStateBeforeOtherOperate) {
        m_sideBar->show();
        this->albumPosChange();
        this->toolBarPostionSet();
    }
    if (m_recordInfoStateBeforeOtherOperate) {
        m_information->show();
        this->inforChange();
    }
}

void KyView::tapAndHoldGesture(QTapAndHoldGesture *gesture)
{
    if (m_panTriggered) {
        return;
    }
    m_touchPoint = gesture->position();
    if (!m_dragWidget->isHidden()) {
        return;
    }
    m_panTriggered = true;
}

void KyView::changeEvent(QEvent *event)
{
    if (m_titlebar == nullptr) {
        return;
    }
    if (QEvent::WindowStateChange == event->type()) {
        Qt::WindowStates state = windowState();
        if (state == Qt::WindowMinimized) {
            m_wasMaximizedBeforeMinimize = (m_previousWindowState & Qt::WindowMaximized);
        }
        if (state == Qt::WindowNoState) {
            if (m_wasMaximizedBeforeMinimize) {
                m_wasMaximizedBeforeMinimize = false;
                QTimer::singleShot(10, this, [this]() {
                    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
                        this->showMaximized();
                    } else {
                        this->showMaximized();
                    }
                });
                return;
            }
            m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-maximize-symbolic")); //主题库的全屏图标
            m_titlebar->g_fullscreen->setToolTip(tr("full srceen"));
            if (!m_isPcModeStarted) {
                m_isPcModeStarted = true;
                resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

            if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
                int sw = QGuiApplication::primaryScreen()->availableGeometry().width();
                int sh = QGuiApplication::primaryScreen()->availableGeometry().height();
                leftPoint = QPoint((sw - this->width()) / 2, (sh - this->height()) / 2);
                qDebug() << "= "<< __FUNCTION__<<" "<<__LINE__ << this->geometry() << " leftPoint = " << leftPoint;
                this->setGeometry(QRect(leftPoint, QSize(this->width(), this->height())));
                //kdk::WindowManager::setGeometry(this->windowHandle(), QRect(leftPoint, QSize(this->width(), this->height())));
             } else {
                QScreen *screen = QGuiApplication::primaryScreen();
                this->move((screen->geometry().width() - this->width()) / 2,
                           (screen->geometry().height() - this->height()) / 2);
            }

            }
        } else {
            m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的恢复图标
            m_titlebar->g_fullscreen->setToolTip(tr("recovery"));
        }
        m_previousWindowState = state;
    }
    if (m_nowFullScreen) {
        m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的恢复图标
        m_titlebar->g_fullscreen->setToolTip(tr("recovery"));
    } else {
        if (m_titlebar == nullptr) {
            return;
        }
        if (this->isMaximized()) {
            m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的恢复图标
            m_titlebar->g_fullscreen->setToolTip(tr("recovery"));
        } else {
            m_titlebar->g_fullscreen->setIcon(QIcon::fromTheme("window-maximize-symbolic")); //主题库的全屏图标
            m_titlebar->g_fullscreen->setToolTip(tr("full srceen"));
        }
    }
    return;
}

void KyView::closeEvent(QCloseEvent *event)
{
    if (m_showImageWidget && m_showImageWidget->m_replaceBackgroundWid)
    {
        m_showImageWidget->m_replaceBackgroundWid->closeColorDialog();
    }

    QWidget::closeEvent(event);
}
