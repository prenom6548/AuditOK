#include "clipboardbackend.h"
#include "utils.h"
#ifdef HAVE_COREAI_PATH
#include "ocrhandler.h"
#endif

#include <QBuffer>
#include <QClipboard>
#include <QCursor>
#include <QGuiApplication>
#include <QMimeData>
#include <QAction>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QWindow>
#include <QtConcurrent>
#include <QtDBus>

// kf5
#include <KF5/KGlobalAccel/KGlobalAccel>
#include <KF5/KWindowSystem/KWindowSystem>
#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>

// kysdk
#include <kysdk/applications/windowmanager/windowmanager.h>
#include <ukuistylehelper/ukui-shell-manager.h>
#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>

ClipboardBackend::ClipboardBackend(QObject *parent)
    : QObject{parent}
    , m_imageProvider(new InMemoryImageProvider())
    , m_isWayland(QGuiApplication::platformName().startsWith(QStringLiteral("wayland")))
{
    //TODO：屏蔽快捷键设置，由usd统一设置
    monitorShortCut();
    // 加0.1s的延时保证快捷键生效
    m_timer.setInterval(200);
    m_timer.setSingleShot(true);
    if (m_isWayland) {
        m_pNormalTip = new kdk::KBallonTip();
        m_pNormalTip->setWindowFlag(Qt::FramelessWindowHint);
        m_pNormalTip->setAttribute(Qt::WA_TranslucentBackground);
        m_pNormalTip->setTipType(kdk::TipType::Normal);
        m_pNormalTip->setText(tr("Successfully copied, please paste at the corresponding location"));
    }
    connect(&m_timer, &QTimer::timeout, this, &ClipboardBackend::pasteClicked);

    //监听主题变化
    monitorIconThemeChange();

    setThemeSettings("styleName");
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *themeSettings = new QGSettings("org.ukui.style");
        connect(themeSettings, &QGSettings::changed, this, &ClipboardBackend::setThemeSettings);
    }

    if (QGSettings::isSchemaInstalled("org.ukui.control-center.personalise")) {
        m_settings = new QGSettings("org.ukui.control-center.personalise");
        m_transparency = m_settings->get("transparency").toDouble();
        connect(m_settings, &QGSettings::changed, [&](){
            m_transparency = m_settings->get("transparency").toDouble();
            emit transparencyChanged();
        });
    }
#ifdef HAVE_COREAI_PATH
    connect(&OcrHandler::instance(), &OcrHandler::readToGetResult, this, &ClipboardBackend::getResult);
#endif
}

void ClipboardBackend::getResult(const QString &text)
{
    setPlainText(text);
}

void ClipboardBackend::setDisplayingType(int type)
{
    if (m_displayingType == type) {
        return;
    }

    m_displayingType = type;
    emit displayingTypeChanged();

    switch (m_displayingType) {
    case ContentMenu::PlainText:
        m_displayedIconName = "ukui-idtext-symbolic";
        break;
    case ContentMenu::RichText:
        m_displayedIconName = "ukui-idtext-symbolic";
        break;
    case ContentMenu::Image:
        m_displayedIconName = "image-x-generic-symbolic";
        break;
    case ContentMenu::Original:
        m_displayedIconName = "";
        break;
    default:
        m_displayedIconName = "";
        break;
    }
    emit displayedIconNameChanged();
}

void ClipboardBackend::setOriginalType(int type)
{
    if (m_originalType == type) {
        return;
    }

    m_originalType = type;
    emit originalTypeChanged();
}

void ClipboardBackend::setImageSize(QSize size)
{
    if (m_imageSize == size) {
        return;
    }

    m_imageSize = size;
    emit imageSizeChanged();
}


void ClipboardBackend::addContentMenu(ContentMenu *menu)
{
    m_contentMenus.append(menu);
    emit contentMenusChanged();
}

void ClipboardBackend::removeContentMenu(int index)
{
    m_contentMenus.removeAt(index);
    emit contentMenusChanged();
}

void ClipboardBackend::clearContentMenu()
{
    qDeleteAll(m_contentMenus);
    m_contentMenus.clear();
}

InMemoryImageProvider *ClipboardBackend::getImageProvider()
{
    return m_imageProvider;
}

void ClipboardBackend::addImage(const QImage &image)
{
    if (m_imageProvider) {
        m_imageProvider->addImage(image);
        setImageSize(image.size());
    }
}

void ClipboardBackend::clearImages()
{
    if (m_imageProvider) {
        m_imageProvider->clear();
    }
}

bool ClipboardBackend::processClipboardData()
{
    //从qclipboard中获取当前复制的内容
    qDebug()<<"从qclipboard中获取当前复制的内容";
    const QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData == nullptr) {
        return false;
    }

    //qclipoard中没有数据则返回false，不清楚为啥要判断是否在X环境
    if (KWindowSystem::isPlatformX11() && mimeData->html().isEmpty() && mimeData->text().isEmpty()
            && mimeData->urls().size() == 0 && !mimeData->hasImage()) {
        return false;
    }

    this->clearContentMenu();
    this->clearImages();

    qDebug()<<"mimeData->urls().size()="<<mimeData->urls().size();
    if (mimeData->urls().size() > 0) {
        qDebug()<<"获取"<<mimeData->urls().size()<<"个urls";
        // TODO: 目前只支持对图片的复制，且只支持一张图片的复制识别操作
        QImage image(mimeData->urls().at(0).path());
        if (image.isNull()) {
            qDebug()<<"转换为图片失败,获取到了未考虑内容";
            return false;
        }
        this->addImage(image);
#ifdef HAVE_COREAI_PATH
        OcrHandler::instance().setImage(image);
#else
        QtConcurrent::run([this, &image]() {
            setPlainText(convertImageToPlainText(image));
        });
#endif
        setOriginalType(ContentMenu::Image);
        this->addContentMenu(new ContentMenu(ContentMenu::PlainText, tr("text recognition")));
        this->addContentMenu(new ContentMenu(ContentMenu::Original, tr("original paste")));
    } else if (mimeData->hasHtml()/* && !mimeData->hasFormat("Art::Table ClipFormat")*/) {
        qDebug()<<"获取到了html";
        this->m_richText = mimeData->html();
        this->m_plainText = clipboard->text();
        setOriginalType(ContentMenu::RichText);
        this->addImage(convertRichTextToImage(this->m_richText));
        this->addContentMenu(new ContentMenu(ContentMenu::Image, tr("convert to picture")));
        this->addContentMenu(new ContentMenu(ContentMenu::PlainText, tr("convert to plaintext")));
        this->addContentMenu(new ContentMenu(ContentMenu::Original, tr("original paste")));
    } else if (mimeData->hasImage()) {
        qDebug()<<"获取到了Image";
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        this->addImage(image);
#ifdef HAVE_COREAI_PATH
        OcrHandler::instance().setImage(image);
#else
        QtConcurrent::run([this, &image]() {
            setPlainText(convertImageToPlainText(image));
        });
#endif
        setOriginalType(ContentMenu::Image);
        this->addContentMenu(new ContentMenu(ContentMenu::PlainText, tr("text recognition")));
        this->addContentMenu(new ContentMenu(ContentMenu::Original, tr("original paste")));
    } else if (mimeData->hasText()){
        qDebug()<<"获取到了文本";
        this->m_plainText = clipboard->text();
        setOriginalType(ContentMenu::PlainText);
        this->addImage(convertPlainTextToImage(this->m_plainText));
        this->addContentMenu(new ContentMenu(ContentMenu::Image,tr("convert to picture")));
        this->addContentMenu(new ContentMenu(ContentMenu::Original, tr("original paste")));
    } else {
        qDebug()<<"获取到了未考虑内容";
        return false;
    }
    return true;
}

void ClipboardBackend::setClipboardWindowVisible(bool visible) {
    m_clipboardWindowVisible = visible;
}

void ClipboardBackend::open()
{
    qDebug()<<"Backend open!";
    if (processClipboardData()) {
        showContentMenu();
    }
}

void setWindowSettings()
{
    auto m_windowList = qGuiApp->allWindows();
    foreach (QWindow *window, m_windowList) {
        if (window->title() == "Function Menu Window" ||window->title() ==  "option menu") {
            auto *helper = new UkuiWindowHelper(window);
            helper->setSkipSwitcher(true);
            helper->setSkipTaskBar(true);
            helper->setBlurEffect(QRegion());
            helper->removeTitleBar();
            helper->openUnderCursor(QPoint(0, 0));
        } else if (window->title() == "Preview Window") {
            auto *helper = new UkuiWindowHelper(window);
            helper->setSkipSwitcher(true);
            helper->setSkipTaskBar(true);
            helper->setBlurEffect(QRegion());
            helper->removeTitleBar();
        }
    }
}

void ClipboardBackend::monitorShortCut()
{
    // TODO: 监听快捷键
    auto* action = new QAction(this);
    action->setObjectName("ShowContentMenu");
    action->setText("Show Content Menu");

    QKeySequence keySeq(Qt::META + Qt::Key_V);
    KGlobalAccel::self()->setDefaultShortcut(action, QList<QKeySequence>() << keySeq);
    KGlobalAccel::self()->setShortcut(action, QList<QKeySequence>() << keySeq);
    connect(action, &QAction::triggered, this, [this]() {
        qDebug() <<"快捷键触发" ;
        if(!m_firstTriggered) {
            emit actionTriggered();
            m_firstTriggered = true;
            setWindowSettings();
        }
        if (m_clipboardWindowVisible) {
            return;
        }
        if (processClipboardData()) {
            showContentMenu();
        }
    });
}

void ClipboardBackend::showContentMenu()
{
    emit contentMenuVisibleChanged(true);
}

void ClipboardBackend::showClipboardWindow()
{
    emit clipboardWindowVisibleChanged(true);
    setClipboardWindowVisible(true);
}

void ClipboardBackend::pasteClicked() const
{
    if (m_isWayland) {
        QProcess pro;
        pro.start("wlcctrl",QStringList()<<"--keydown"<<"control_l"<<"--key"<<"v"<<"--keyup"<<"control_l");
        if (!pro.waitForFinished(1000) || pro.exitCode() != 0) {
            qWarning()<<"wlcctrl模拟快捷键ctrl+v失败";
        }
    } else {
        Display *disp = XOpenDisplay(NULL);
        if (disp == NULL) {
            return;
        }

        const KeySym ctrlKeySym = XK_Control_L;
        const KeySym vKeySym = XK_V;
        KeyCode vCode = XKeysymToKeycode(disp, vKeySym);
        KeyCode ctrlCode = XKeysymToKeycode(disp, ctrlKeySym);
        if (vCode == 0 || ctrlCode == 0) {
            XCloseDisplay(disp);
            return;
        }

        XTestGrabControl(disp, True);

        XTestFakeKeyEvent(disp, ctrlCode, True, 0);
        XTestFakeKeyEvent(disp, vCode, True, 0);
        XTestFakeKeyEvent(disp, vCode, False, 0);
        XTestFakeKeyEvent(disp, ctrlCode, False, 0);

        XTestGrabControl(disp, False);
        // Flush the display to ensure all events are sent
        XFlush(disp);

        XCloseDisplay(disp);
    }
}

void ClipboardBackend::monitorIconThemeChange() {
    if (!QGSettings::isSchemaInstalled("org.ukui.style")) {
        qWarning() << __FUNCTION__ << __FILE__ << "can not find schema org.ukui.style";
        return;
    }
    QGSettings* iconThemeSettings = new QGSettings("org.ukui.style", "/org/ukui/style/");
    connect(iconThemeSettings, &QGSettings::changed, [&](const QString& value) {
        if (value == "iconThemeName") {
            emit iconThemeChanged();
        }
    });
}

int ClipboardBackend::imageNumber() const
{
    if (m_imageProvider) {
        return m_imageProvider->imageNumber();
    }

    return 0;
}

void ClipboardBackend::copyToClipboard(int contentType)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard == nullptr) {
        return;
    }

    if (contentType == ContentMenu::PlainText) {
        clipboard->setText(m_plainText);
    } else if (contentType == ContentMenu::RichText) {
        QMimeData *mimedata = new QMimeData();
        // 必须同时设置富文本和纯文本才能复制到剪切板
        mimedata->setHtml(m_richText);
        mimedata->setText(convertRichTextToPlainText(m_richText));
        clipboard->setMimeData(mimedata);
    } else {
        QList<QImage> images = m_imageProvider->requestAllImage();
        clipboard->setImage(images[0]);
    }
}

void ClipboardBackend::performPasteOperation()
{
    m_timer.start();
}

QPoint ClipboardBackend::getGlobalMousePosition() const
{
    return QCursor::pos();
}

void ClipboardBackend::setRichText(const QString &richText)
{
    if (m_richText == richText) {
        return;
    }
    m_richText = richText;
    emit richTextChanged();
}

void ClipboardBackend::setPlainText(const QString &plainText)
{
    if (m_plainText == plainText) {
        return;
    }
    m_plainText = plainText;

    emit plainTextChanged();
}

void ClipboardBackend::setThemeSettings(QString str)
{
    qDebug()<<"UKUI theme changed = "<<str;
    if (str == "styleName" || str == "widgetThemeName") {
        if (qApp->property("kcontain-general-normal").isValid() && qApp->property("kcontain-general-normal").canConvert<QColor>()) {
            m_backgroundColor = qApp->property("kcontain-general-normal").value<QColor>();
            emit backgroundColorChanged();
        } else {
            qWarning()<<"Error: get background color--kcontain-General-normal ";
        }
    }
}
