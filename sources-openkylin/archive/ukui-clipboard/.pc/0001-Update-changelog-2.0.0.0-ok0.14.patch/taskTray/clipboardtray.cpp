#include "clipboardtray.h"
#include <QDesktopWidget>
#include <QDebug>
#include <QTimer>
#include <QDebug>
#include <QFileIconProvider>
#include <QRegularExpression>
#include <QApplication>

ClipboardTray::ClipboardTray(QWidget *parent)
    : QWidget(parent, Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint)
{
    InitClipboardTray();
    InitMenu();
    mSystemTray->setVisible(true);
    InitData();
}

/*
*	注册托盘图标，绑定激活事件(包括点击，双击，右键)
*/
void ClipboardTray::InitClipboardTray()
{
    mSystemTray = new QSystemTrayIcon(this);
    mSystemTray->setIcon(QIcon(":/source/data/clipboard.svg"));
    mSystemTray->setToolTip(QString::fromLocal8Bit("ukui-clipboard"));
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
        QGuiApplication::quit();
    });
    mShowMainWindowAction = new QAction(mMenu);	//show窗口
    connect(mShowMainWindowAction, &QAction::triggered, [this]() {
        setWindowVisible();
    });
    mQuitAction->setText(tr("Log Out"));
    mShowMainWindowAction->setText(tr("Show Main Window"));

    mMenu->addAction(mShowMainWindowAction);
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

void ClipboardTray::setWindowVisible(bool flag)
{
    if(flag == m_windowVisible)
        return;

    m_windowVisible = flag;
    emit windowVisibleChanged();

}

void ClipboardTray::dataAdd()
{
    qDebug()<<"dataAdd!";
    const QMimeData *mimeData = m_clipboard->mimeData();
    if (mimeData == nullptr) {
        qWarning()<<"data is empty";
        return;
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
                QFileInfo fileInfo(localFilePath);
                QFileIconProvider iconProvider;
                QIcon fileIcon = iconProvider.icon(fileInfo);
                int width = 48, height = 48;  // 你可以根据需要调整这个尺寸
                QPixmap pixmap = fileIcon.pixmap(width, height);
                QImage image = pixmap.toImage();

                handelData("string","string",image,mimeData->urls().at(0).fileName(),clipboardData::File,mimeData);
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
    } else if (mimeData->hasHtml() && !mimeData->hasFormat("Art::Table ClipFormat")) {
        QString str = mimeData->html();
        QImage image;
        if( isWebUrl(str) ) {
            handelData("string","string",image,str,clipboardData::Link,mimeData);
        } else {
            handelData("string","string",image,str,clipboardData::Text,mimeData);
        }
        qDebug()<<"qt中复制一段代码，网页复制一段内容，";
    } else if (mimeData->hasText()) {
        QString str = mimeData->text();
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
        textModel->removeData(mySender);
        imageModel->removeData(mySender);
        fileModel->removeData(mySender);
        linkModel->removeData(mySender);
    }
}

QMimeData* getMimedata(const QMimeData* mData)
{
    QMimeData *newMimeData = new QMimeData();

    // 如果QMimeData包含URL列表，则也复制URL列表
    if (mData->hasUrls()) {
        QList<QUrl> urls = mData->urls();
        newMimeData->setUrls(urls);
    }

    // 遍历QMimeData支持的所有格式并复制数据
    foreach (QString format, mData->formats()) {
        QByteArray data = mData->data(format);
        newMimeData->setData(format, data);
    }

    // 如果QMimeData包含图像，则也复制图像
    if (mData->hasImage()) {
        QImage image = qvariant_cast<QImage>(mData->imageData());
        newMimeData->setImageData(QVariant::fromValue(image));
    }

    // 如果QMimeData包含HTML，则也复制HTML
    if (mData->hasHtml()) {
        QString html = mData->html();
        newMimeData->setHtml(html);
    }

    // 如果QMimeData包含文本，则也复制文本
    if (mData->hasText()) {
        QString text = mData->text();
        newMimeData->setText(text);
    }
    return newMimeData;
}

void ClipboardTray::handelData(QString m_fromText, QString m_timeText, QImage m_image, QString m_sourceText, clipboardData::Mode mode, const QMimeData* mData)
{
    bool var = false;
    if (mode == clipboardData::File || mode == clipboardData::Image) var = true;
    num++;
    emit myTextDatasChanged();
    clipboardData *data = new clipboardData();
    data->setId(QString::number(num));
    data->setImage(m_image);
    data->setSourceText(m_sourceText);
    data->setSourceTime(QDateTime::currentDateTime());
    data->mode = mode;
    data->m_mineData = getMimedata(mData);
    data->m_dataClipboard = m_clipboard;
    data->setHasImage(var);
    allModel->appendData(data);
    QObject::connect(allModel->getFirstData(),&clipboardData::isCollectedChanged,this,&ClipboardTray::dataChanged);
    QObject::connect(allModel->getFirstData(),&clipboardData::dataCopyed,this,&ClipboardTray::dataNeedRemove);

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
