#include "clipboardtray.h"
#include <QDesktopWidget>
#include <QDebug>
#include <QTimer>
#include <QDebug>
#include <QFileIconProvider>
#include <QRegularExpression>
#include <QApplication>
#include <QProcess>
#include <QMimeDatabase>
#include <QMessageBox>

ClipboardTray::ClipboardTray(QWidget *parent)
    : QWidget(parent, Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint)
{
    InitClipboardTray();
    InitMenu();
    connect(this, &ClipboardTray::firstCopy, [&](){
        mSystemTray->setVisible(true);
    });
    connect(this, &ClipboardTray::styleThemeChanged, [&](){
        fileModel->updateItemImage();
    });
    InitData();
    qDebug() << "ClipboardTray::ClipboardTray, threadId: " << QThread::currentThreadId();
}

ClipboardTray::~ClipboardTray()
{
    delete allModel;
    delete textModel;
    delete imageModel;
    delete fileModel;
    delete linkModel;
    delete collectionModel;
}

/*
*	注册托盘图标，绑定激活事件(包括点击，双击，右键)
*/
void ClipboardTray::InitClipboardTray()
{
    mSystemTray = new QSystemTrayIcon(this);
    mSystemTray->setIcon(QIcon::fromTheme("ukui-clipboard-symbolic"));
    mSystemTray->setToolTip(tr("ukui-clipboard"));
    connect(mSystemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(ActiveTray(QSystemTrayIcon::ActivationReason)));
    QString strMsg = "图标闪变！";
    mSystemTray->showMessage(tr("消息"), strMsg, QSystemTrayIcon::Information, 5000);

      //此悬浮事件仅在2309商业版本上支持，如需使用需要升级任务栏和QT版本
//    connect(mSystemTray, SIGNAL(hovered(QSystemTrayIcon::ActivationReason)),
//            this, SLOT(HoverTray(QSystemTrayIcon::ActivationReason)));
}
void ClipboardTray::ActiveTray(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason) {
        case QSystemTrayIcon::DoubleClick:
            qDebug()<<">>>>>>> 鼠标双击~";
            break;
        case QSystemTrayIcon::Trigger:
            setWindowVisible();
            qDebug()<<">>>>>>> 鼠标左键点击~";
            break;
        case QSystemTrayIcon::MiddleClick:
            qDebug()<<">>>>>>> 鼠标中键点击~";
            break;
        case QSystemTrayIcon::Context:
            qDebug()<<">>>>>>> 鼠标右键点击~";
            ShowMenu(); //应用显示菜单窗口
            break;

            //此悬浮事件仅在2309商业版本上支持，如需使用需要升级任务栏和QT版本
//        case QSystemTrayIcon::Hover:
//            qDebug()<<">>>>>>> 鼠标悬浮~";
//            break;
        default:
            break;
    }
}

void ClipboardTray::HoverTray(QSystemTrayIcon::ActivationReason reason)
{
    qDebug()<<">>>>>>>>>>HoverTray "<<reason;
}

/*
*	初始化菜单，将action添加到菜单，菜单合入托盘
*/
void ClipboardTray::InitMenu()
{
    /* 创建菜单和行为实例 */
    mMenu = new QMenu(this);
    mQuitAction = new QAction(mMenu); 		//退出
    connect(mQuitAction, &QAction::triggered, [this]() {
        //QGuiApplication::quit();
        mSystemTray->setVisible(false);
        m_isFirstCopy = true;
        setWindowVisible(false);
    });
    mShowMainWindowAction = new QAction(mMenu);	//show窗口
    connect(mShowMainWindowAction, &QAction::triggered, [this]() {
        setWindowVisible();
    });
    mQuitAction->setText(tr("Close"));
    mQuitAction->setIcon(QIcon::fromTheme("window-close-symbolic"));
    mShowMainWindowAction->setText(tr("Show Main Window"));
    mShowMainWindowAction->setIcon(QIcon::fromTheme("ukui-clipboard"));

    mMenu->addAction(mShowMainWindowAction);
    mMenu->addSeparator();
    mMenu->addAction(mQuitAction);

    /*setContextMenu设置后，应用的右键菜单会由托盘统一管理，主题会跟随系统主题（推荐使用）；
    如果右键菜单比较复杂（包含子窗口、图标、按钮、动态显示等），建议应用不要使用setContextMenu，
    并且在ActiveTray中右键点击部分自己处理菜单事件。*/
    mSystemTray->setContextMenu(mMenu);
}

void ClipboardTray::ShowMenu()
{
    //如果没有设置setContextMenu的话，点击托盘图标时会走到这个槽函数，由应用自行显示菜单。
    //注意：这里需要指定菜单的显示位置，可以通过获取点击时的鼠标的坐标和桌面可用做坐标计算要显示的位置。
    QSize menuSize = mMenu->sizeHint();
    QPoint point = QCursor::pos();
    QRect deskRect = QApplication::desktop()->availableGeometry(this); //获取桌面可用坐标
    mMenu->move(point.x(), deskRect.height() - menuSize.height());
    mMenu->show();
}

void ClipboardTray::setCurrentModel(Clipboardmodel *model)
{
    qDebug()<<"setCurrentModel in,model szie = " << model->rowCount();
    m_currentModel = model;
    emit currentModelChanged();
}

Clipboardmodel* ClipboardTray::currentModel()
{
    return m_currentModel;
}

QVector<Clipboardmodel*> ClipboardTray::models()
{
    return m_models;
}

void ClipboardTray::InitData()
{
    qDebug()<<"InitData!";
    m_clipboard = QApplication::clipboard();
    m_models.append(allModel);
    m_models.append(textModel);
    m_models.append(imageModel);
    m_models.append(fileModel);
    m_models.append(linkModel);
    m_models.append(collectionModel);
    setCurrentModel(m_models.first());
    connect(m_clipboard,&QClipboard::dataChanged,this,&ClipboardTray::dataAdd);

    if (QGSettings::isSchemaInstalled("org.ukui.control-center.personalise")) {
        settings = new QGSettings("org.ukui.control-center.personalise");
        m_transparency = settings->get("transparency").toDouble();
        connect(settings, &QGSettings::changed, this, &ClipboardTray::transparencyChanged);
    }

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *themeSettings = new QGSettings("org.ukui.style");
        connect(themeSettings, &QGSettings::changed, this, &ClipboardTray::themeChanged);
    }

    wayBackendThread = new WaylandBackendThread();
    mTimer.setSingleShot(true);
    QObject::connect(wayBackendThread, &WaylandBackendThread::selectionNil, this, &ClipboardTray::selectionNilSolt);
    QObject::connect(wayBackendThread, &WaylandBackendThread::selectionNew, this, &ClipboardTray::selectionNewSolt);
    wayBackendThread->start();
    connect(&mTimer, &QTimer::timeout, this, &ClipboardTray::handelSelecPro);
}

void ClipboardTray::selectionNilSolt()
{
    qDebug() << "收到 selection nil 信号";
    if (selectionProIsAlive) {
        selectionProIsAlive = false;
        mTimer.start(100);
    }
}

void ClipboardTray::selectionNewSolt()
{
    qDebug() << "收到 selection new 信号";
    if (!selectionProIsAlive) {
        selectionProIsAlive = true;
        if (mTimer.isActive()) {
            mTimer.stop();
        }
    }
}

QMimeData* getMimedata(const QMimeData* mData)
{
    QMimeData *newMimeData = new QMimeData();

    QStringList mimeTypeList = mData->formats();
    foreach (const QString &mimeType, mimeTypeList) {
        QByteArray data = mData->data(mimeType);
        newMimeData->setData(mimeType,data);
    }
    if (mData->hasImage()) {
        newMimeData->setImageData(mData->imageData());
    }
    return newMimeData;
}

void ClipboardTray::handelSelecPro()
{
    qDebug()<<"原复制来源程序退出，剪切板设置自己为复制来源";
    m_clipboard->setMimeData(getMimedata(allModel->getFirstData()->m_mineData));
}

bool isWebUrl(const QString &str) {
    QUrl url(str);
    // 检查URL是否有效并且其方案是http或https
    return url.isValid() && (url.scheme() == "http" || url.scheme() == "https");
}

QSize calculateProportionalSize(const QSize &originalSize, int maxHeight, int maxWidth) {
    int originalWidth = originalSize.width();
    int originalHeight = originalSize.height();

    double ratio = static_cast<double>(originalWidth) / originalHeight;

    // 首先尝试基于高度缩放
    int newHeight = qMin(originalHeight, maxHeight);
    int newWidth = static_cast<int>(newHeight * ratio);

    // 如果基于高度缩放后的宽度超过了最大宽度，则根据宽度重新计算
    if (newWidth > maxWidth) {
        newWidth = maxWidth;
        newHeight = static_cast<int>(newWidth / ratio);
    }

    return QSize(newWidth, newHeight);
}

bool ClipboardTray::windowVisible()
{
    return m_windowVisible;
}

double ClipboardTray::transparency()
{
    return m_transparency;
}

void ClipboardTray::transparencyChanged()
{
    m_transparency = settings->get("transparency").toDouble();
}

void ClipboardTray::setWindowVisible(bool flag)
{
    if(flag == m_windowVisible) {
        if (flag == false) {
            return;
        } else {
            if (m_Window->isActive()) { //窗口显示，且为激活状态时，点击托盘按钮则隐藏窗口
                m_windowVisible = false;
                emit windowVisibleChanged();
                return;
            } else { //窗口显示，且未激活时，点击托盘按钮则激活窗口
                m_Window->requestActivate();
                return;
            }
        }
    }

    if(m_isFirstShow) {
        m_isFirstShow = false;
        emit firstShowWindow();
    }

    if (!flag) {
        setkeepAbove(false);
    }

    m_windowVisible = flag;
    emit windowVisibleChanged();

}

QString getImageFromFilePath(QString localFilePath)
{
    QFileInfo fileInfo(localFilePath);
    //QFileIconProvider iconProvider;

    QMimeDatabase mimeDataBase;
    QMimeType mimeType = mimeDataBase.mimeTypeForFile(fileInfo);
    QString type;
    if (mimeType.isValid()) {
        type = mimeType.name();
        qDebug()<<"文件类型为" << type;
    } else {
        qWarning()<<"获取文件图标失败，采用默认图标";
        type = "text/plain"; //默认值
    }

    //兼容旧版本wps格式
    if (type == "application/vnd.ms-works") {
        type = "application/wps-office.wps";
    }

    type.replace("/","-");
    QIcon fileIcon = QIcon::fromTheme(type);
    if (fileIcon.isNull()) {
        if (type.contains("audio")) { //主题中不存在这种类型的音乐图标，使用默认音乐图标
            type = "audio-x-generic";
        }
    }

    return type;
}

void ClipboardTray::dataAdd()
{
    qDebug()<<"dataAdd!";

    if (m_clipboard->ownsClipboard()) {
        qDebug()<<"clipboard recopy";
        return;
    }

    const QMimeData *mimeData = m_clipboard->mimeData();
    if (mimeData == nullptr) {
        qWarning()<<"data is empty";
        return;
    }

    if (m_isFirstCopy) {
        m_isFirstCopy = false;
        emit firstCopy();
    }

    qDebug()<<"获取"<<mimeData->urls().size()<<"个urls";
    if (mimeData->urls().size() == 1) { //暂时只支持对一个文件复制
        QImage image(mimeData->urls().at(0).path());
        if (image.isNull()) {//url为系统文件
            qDebug()<<mimeData->urls().at(0).fileName();
            qDebug()<<"txt文本，.wps文件，压缩包，文件夹";
            if (mimeData->urls().at(0).isLocalFile()) {
                // 从 QUrl 获取本地文件路径
                QString localFilePath = mimeData->urls().at(0).toLocalFile();
                QString iconName = getImageFromFilePath(localFilePath);
                QImage image;
                handelData(iconName,"string",image,mimeData->urls().at(0).fileName(),clipboardData::File,mimeData);
            } else {
                // 处理错误情况，比如不是一个本地文件
                qDebug()<<"该url不是个本地文件";
            }
            return;
        }
        //为图片
        QSize newSize = calculateProportionalSize(image.size(), 48, 324);
        QImage scaledImage = image.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        qDebug()<<mimeData->urls().at(0).fileName();
        qDebug()<<"jpg，svg图片";
        handelData("string","string",scaledImage,"scaledImageString",clipboardData::Image,mimeData);
    } else if (mimeData->hasImage()){
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        if (image.isNull()) {
            qDebug()<<"image is null";
            return;
        }
        QSize newSize = calculateProportionalSize(image.size(), 48, 324);
        QImage scaledImage = image.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        qDebug()<<mimeData->urls().at(0).fileName();
        qDebug()<<"jpg，svg图片";
        handelData("string","string",scaledImage,"scaledImageString",clipboardData::Image,mimeData);
    } else if (mimeData->hasHtml() && !mimeData->hasFormat("Art::Table ClipFormat")) {
        QString str = mimeData->html();
        if (str.isEmpty()) {
            qDebug()<<"str is empty";
            return;
        }
        QImage image;
        if( isWebUrl(str) ) {
            handelData("string","string",image,str,clipboardData::Link,mimeData);
        } else {
            handelData("string","string",image,str,clipboardData::Text,mimeData);
        }
            qDebug()<<"qt中复制一段代码，网页复制一段内容，";
    } else if (mimeData->hasText()) {
        QString str = mimeData->text();
        if (str.isEmpty()) {
            qDebug()<<"str is empty";
            return;
        }
        QImage image;
        if( isWebUrl(str) ) {
            handelData("string","string",image,str,clipboardData::Link,mimeData);
        } else {
            handelData("string","string",image,str,clipboardData::Text,mimeData);
        }
        qDebug()<<"获取到了text或link，txt文本中复制的内容，";
    } else {
        qDebug()<<"获取到了未考虑内容";
    }
}

void ClipboardTray::dataChanged()
{
    QObject *sender = this->sender();
    if (clipboardData *mySender = qobject_cast<clipboardData*>(sender)) {
        if (mySender->isCollected()) {
            qDebug()<<"Data add,data string = "<<mySender->sourceText();
            collectionModel->appendData(mySender);
        } else {
            qDebug()<<"Data remove";
            collectionModel->removeData(mySender);
        }
    }
}

void ClipboardTray::dataNeedRemove()
{
    QObject *sender = this->sender();
    if (clipboardData *mySender = qobject_cast<clipboardData*>(sender)) {
        allModel->removeData(mySender);

        if (mySender->mode == clipboardData::Text) textModel->removeData(mySender);
        else if (mySender->mode == clipboardData::Image) imageModel->removeData(mySender);
        else if (mySender->mode == clipboardData::File) fileModel->removeData(mySender);
        else if (mySender->mode == clipboardData::Link) linkModel->removeData(mySender);

        if (mySender->isCollected()) collectionModel->removeData(mySender);
    }
}

void ClipboardTray::removeAllData()
{
//    int ret = QMessageBox::question(this, tr("Conform operation"), tr("Are you sure you want to clear the clipboard?"),
//                                    QMessageBox::Ok | QMessageBox::Cancel);
    QMessageBox *msgBox = new QMessageBox();
    msgBox->setWindowTitle(tr("Conform operation"));
    msgBox->setIcon(QMessageBox::Question);
    msgBox->setText(tr("Are you sure you want to clear the clipboard?"));
    msgBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox->setDefaultButton(QMessageBox::Ok);

    int ret = msgBox->exec();

    if (ret == QMessageBox::Ok) {
        allModel->removeAllData();
        textModel->removeAllData();
        imageModel->removeAllData();
        fileModel->removeAllData();
        linkModel->removeAllData();
        collectionModel->removeAllData();
    }
}

void ClipboardTray::updateModels()
{
    QObject *sender = this->sender();
    if (clipboardData *mySender = qobject_cast<clipboardData*>(sender)) {
        allModel->moveDataToFirst(mySender);
        if (mySender->mode == clipboardData::Text) {
            textModel->moveDataToFirst(mySender);
        } else if (mySender->mode == clipboardData::Image) {
            imageModel->moveDataToFirst(mySender);
        } else if (mySender->mode == clipboardData::File) {
            fileModel->moveDataToFirst(mySender);
        } else if (mySender->mode == clipboardData::Link) {
            linkModel->moveDataToFirst(mySender);
        }
        if (mySender->isCollected()) {
            collectionModel->moveDataToFirst(mySender);
        }
    }
}

bool ClipboardTray::uniqueCheck(clipboardData *data, clipboardData::Mode mode)
{
    qDebug()<<"id="<<data->id();
    int rowItem = allModel->findItem(data);
    qDebug()<<"rowItem"<<rowItem;
    if (rowItem < 0 || rowItem >= allModel->rowCount()) {
        return false;
    }

    allModel->moveDataToFirst(data);
    collectionModel->moveDataToFirst(data);

    if (mode == clipboardData::Text) {
        textModel->moveDataToFirst(data);
    } else if (mode == clipboardData::Image) {
        imageModel->moveDataToFirst(data);
    } else if (mode == clipboardData::File) {
        fileModel->moveDataToFirst(data);
    } else if (mode == clipboardData::Link) {
        linkModel->moveDataToFirst(data);
    }
    return true;
}

void ClipboardTray::handelData(QString iconName, QString m_timeText, QImage m_image, QString m_sourceText, clipboardData::Mode mode, const QMimeData* mData)
{
    bool var = false;
    if (mode == clipboardData::File || mode == clipboardData::Image) var = true;
    num++;
    emit myTextDatasChanged();
    clipboardData *data = new clipboardData(allModel);
    if (mode == clipboardData::Link) {
        data->setIsLinkMode(true);
    } else {
        data->setIsLinkMode(false);
    }
    data->setId(QString::number(num));
    data->setImage(m_image);
    data->setSourceText(m_sourceText);
    data->setSourceTime(QDateTime::currentDateTime());
    data->mode = mode;
    data->m_mineData = getMimedata(mData);
    data->m_dataClipboard = m_clipboard;
    data->setHasImage(var);
    if (mode == clipboardData::File) {
        data->setIconName(iconName);
    }

    if (uniqueCheck(data,mode)) {
        //m_clipboard->setMimeData(getMimedata(mData));
        data->deleteLater();
        return;
    }
    allModel->appendData(data);
    QObject::connect(allModel->getFirstData(),&clipboardData::isCollectedChanged,this,&ClipboardTray::dataChanged);
    QObject::connect(allModel->getFirstData(), &clipboardData::needMoveToFirst, this, &ClipboardTray::updateModels);
    QObject::connect(allModel->getFirstData(), &clipboardData::needToDelete, this, &ClipboardTray::dataNeedRemove);
    QObject::connect(allModel->getFirstData(), &clipboardData::needToDeleteAll, this, &ClipboardTray::removeAllData);
    //m_clipboard->setMimeData(getMimedata(mData));

    //QModelIndex index = allModel->index(0, 0);
    //QString name = allModel->data(index, Clipboardmodel::SourceTextRole).toString();
    //qDebug()<<"currentDateTime="<<QDateTime::currentDateTime();

    if (mode == clipboardData::Text) {
        textModel->appendData(data);
    } else if (mode == clipboardData::Image) {
        imageModel->appendData(data);
    } else if (mode == clipboardData::File) {
        fileModel->appendData(data);
    } else if (mode == clipboardData::Link) {
        linkModel->appendData(data);
    }
}

QColor ClipboardTray::dtColor(QString str)
{
    QColor color;
    if (str == "checkCol" && qApp->property("kgray-alpha3").isValid() && qApp->property("kgray-alpha3").canConvert<QColor>()) {
        color =   qApp->property("kgray-alpha3").value<QColor>();
    } else if (str == "clickCol"&& qApp->property("kgray-alpha4").isValid() && qApp->property("kgray-alpha4").canConvert<QColor>()) {
        color =  qApp->property("kgray-alpha4").value<QColor>();
    } else if (str == "hoverCol" && qApp->property("kgray-alpha2").isValid() && qApp->property("kgray-alpha2").canConvert<QColor>()) {
        color =  qApp->property("kgray-alpha2").value<QColor>();
    } else if (str == "myBtnNolCol" && qApp->property("kgray-0").isValid() && qApp->property("kgray-0").canConvert<QColor>()) {
        color =  qApp->property("kgray-0").value<QColor>();
        color.setAlphaF(0.6);
    } else if (str == "myBtnHovCol" && qApp->property("kgray-0").isValid() && qApp->property("kgray-0").canConvert<QColor>()) {
        color =  qApp->property("kgray-0").value<QColor>();
        color.setAlphaF(0.8);
    } else if (str == "myBtnClkCol" && qApp->property("kgray-0").isValid() && qApp->property("kgray-0").canConvert<QColor>()) {
        color =  qApp->property("kgray-0").value<QColor>();
        color.setAlphaF(0.95);
    } else if (str == "textCol" && qApp->property("kfont-placeholdertext").isValid() && qApp->property("kfont-placeholdertext").canConvert<QColor>()) {
        color =  qApp->property("kfont-placeholdertext").value<QColor>();
    } else if (str == "link-active" && qApp->property("link-active").isValid() && qApp->property("link-active").canConvert<QColor>()) {
        color = qApp->property("link-active").value<QColor>();
    } else {
        qWarning()<<"dtColor str not found";
    }
    return color;
}

void ClipboardTray::setWindowBlurEffect(bool isAcitve)
{
    if (!m_Window || !m_helper) {
        return;
    }

    m_helper->setBlurEffect(QRegion(), 0, isAcitve);
}

void ClipboardTray::getQmlWindow(QWindow *window)
{
    m_Window = window;
    m_Window->setFlags(Qt::Window);
    m_helper = new UkuiWindowHelper(m_Window);
    m_helper->setSkipTaskBar(true);
    m_helper->setSkipSwitcher(true);
    m_helper->setBlurEffect(QRegion(), 0, m_Window->isActive());
    m_helper->removeTitleBar();
    //m_helper->setWindowState(UkuiWindowHelper::States(0x200));
}

bool ClipboardTray::keepAbove()
{
    return m_keepAbove;
}

void ClipboardTray::setkeepAbove(bool arg)
{
    if (arg != m_keepAbove) {
        m_keepAbove = arg;
        emit keepAboveChanged();
        if (arg) {
            m_helper->setWindowState(UkuiWindowHelper::States(0xff) | UkuiWindowHelper::State::KeepAbove);
        } else {
            m_helper->setWindowState(UkuiWindowHelper::States(0xff));
        }
    }
}

void ClipboardTray::themeChanged(QString str)
{
    qDebug()<<"UKUI theme changed = "<<str;
    if (str == "styleName" || str == "widgetThemeName" || str == "iconThemeName") {
        emit styleThemeChanged();
    }
}

bool ClipboardTray::isMirror()
{
    return m_isMirror;
}

void ClipboardTray::setIsMirror(bool arg)
{
    m_isMirror = arg;
    emit isMirrorChanged();
}

void ClipboardTray::xdgOpen(QString str)
{
    QUrl url(str);
    if (url.isValid() && (url.scheme() == "http" || url.scheme() == "https")) {
        QProcess::startDetached("xdg-open", QStringList() << url.toString());
    } else {
        qWarning() << "Invalid or unsupported URL scheme:" << str;
    }
}

void ClipboardTray::openImageOrFile(QString str)
{
    QFileInfo file(str);
    if (file.exists()) {
        if (file.isFile()) {
            QProcess::startDetached("xdg-open", QStringList() << str);
        }
    } else {
        int ret;
        if (str == "") {
            ret = QMessageBox::question(this, tr("Conform operation"), tr("Cannot find file"),
                                        QMessageBox::Yes);
        } else {
            ret = QMessageBox::question(this, tr("Conform operation"), tr("Cannot find file:") + str,
                                            QMessageBox::Yes);
        }
        qDebug()<<"文件不存在，打开失败";
    }
}

void ClipboardTray::openDir(QString str)
{
    QFileInfo file(str);
    if (file.exists()) {
        if (file.isFile()) {
            QProcess::startDetached("peony", QStringList() << str);
        }
    } else {
        int ret = QMessageBox::question(this, tr("Conform operation"), tr("Cannot find file:") + str,
                                        QMessageBox::Yes);
        qDebug()<<"文件不存在，打开失败";
    }
}
