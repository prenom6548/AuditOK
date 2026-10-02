#include "videoform.h"
#include <QDesktopWidget>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QShortcut>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>
#include <QWindow>
#include <QThread>
#include <uibase/tipwidget.h>
#include <KWindowSystem>
#include <scrollsettingwidget.h>
#include "windowmanager/windowmanager.h"
#include <ukuistylehelper/ukuistylehelper.h>

#include "config.h"
#include "controller.h"
#include "basedevice.h"
#include "qyuvopenglwidget.h"
#include "ui_videoform.h"
#include "generatetools.h"

extern "C" {
#include "libavutil/frame.h"
}

#ifndef UKUI_TASK_GSETTING_PATH
#    define UKUI_TASK_GSETTING_PATH "org.ukui.panel.settings"
#endif

const int TITLE_HEIGHT = 40;
const int TRIPE_HEIGHT = 40;
const int ERROR_LENGTH = 1;

VideoForm::VideoForm(QWidget *parent, int screenIndex) : QWidget(parent), ui(new Ui::videoForm)
{
    m_screenIndex = screenIndex;
    m_titleHeight = TITLE_HEIGHT;
    initUI();
    m_loadTimer = new QTimer(this);

    // 注册快捷键
    installShortcut();

    m_tableModeListener = new TableModeListener(this);
    onModeChanged(m_tableModeListener->getMode());
    onModeRotationsChange(m_tableModeListener->getRotations());

    initConnect();
    initPanelSetting();
}

VideoForm::~VideoForm()
{
    if (ui) {
        delete ui;
        ui = nullptr;
    }
}

void VideoForm::initUI()
{
    if (m_screenIndex != 0) {
        QDesktopWidget *desktop = QApplication::desktop();
        setGeometry(desktop->screenGeometry(m_screenIndex));
    }
    ui->setupUi(this);

    kdk::UkuiStyleHelper::self()->removeHeader(this);
    setWindowIcon(QIcon::fromTheme("kylin-connectivity"));
    setAttribute(Qt::WA_DeleteOnClose);
    m_videoWidget = new QYUVOpenGLWidget();
    m_videoWidget->hide();

    m_loadingWidget = new LoadAnimation(this);
    m_loadingWidget->setHighLight();
    ui->keepRatioWidget->setWidget(m_loadingWidget);
    ui->keepRatioWidget->setWidget(m_videoWidget);
    ui->keepRatioWidget->setParent(this);

    installEventFilter(this);
    ui->keepRatioWidget->installEventFilter(this);
    ui->toolform->installEventFilter(this);

    setMouseTracking(true);
    m_videoWidget->setMouseTracking(true);
    ui->keepRatioWidget->setMouseTracking(true);
}

void VideoForm::initConnect()
{
    connect(ui->title, &VideoTitle::sigChangeNavigationStatus, this, &VideoForm::onToolFormStatusChange);
    connect(ui->title, &VideoTitle::sigScroll, this, &VideoForm::onScrollTrigger);
    connect(m_loadTimer, &QTimer::timeout, this, [this]() { m_isLoading = true; });
    connect(m_tableModeListener, &TableModeListener::sigModeChange, this, &VideoForm::onModeChanged);
    connect(m_tableModeListener, &TableModeListener::sigRotationsChange, this, &VideoForm::onModeRotationsChange);
}

void VideoForm::deleteUI()
{
    m_isLoading = false;
    this->hide();
    if (!m_device) {
        return;
    }
    Config::getInstance().setRect(m_device->getSerial(), geometry());
}

QRect VideoForm::getGrabCursorRect()
{
    QRect rc;
    rc = QRect(ui->keepRatioWidget->mapToGlobal(m_videoWidget->pos()), m_videoWidget->size());
    rc.setTopLeft(rc.topLeft() * m_videoWidget->devicePixelRatio());
    rc.setBottomRight(rc.bottomRight() * m_videoWidget->devicePixelRatio());

    rc.setX(rc.x() + 10);
    rc.setY(rc.y() + 10);
    rc.setWidth(rc.width() - 20);
    rc.setHeight(rc.height() - 20);
    return rc;
}

const QSize &VideoForm::frameSize()
{
    return m_frameSize;
}

void VideoForm::updateRender(const AVFrame *frame)
{
    if (m_videoWidget->isHidden()) {
        show();
        activateWindow();
        setFocus();
        m_resizeAuto = false;
        updateShowSize(QSize(frame->width, frame->height));
        if (m_loadTimer) {
            m_loadTimer->start(500);
        }
        if (m_isLoading) {
            if (m_loadingWidget) {
                m_loadingWidget->close();
                m_loadingWidget->deleteLater();
                m_loadingWidget = nullptr;
            }
            m_resizeAuto = false;
            m_videoWidget->show();
        } else {
            m_resizeAuto = false;
            m_loadingWidget->resize(this->size());
            m_loadingWidget->show();
        }
    } else {
        m_resizeAuto = false;
        updateShowSize(QSize(frame->width, frame->height));
    }

    m_videoWidget->setFrameSize(QSize(frame->width, frame->height));
    m_videoWidget->updateTextures(frame->data[0], frame->data[1], frame->data[2], frame->linesize[0],
                                  frame->linesize[1], frame->linesize[2]);
}

QRect VideoForm::getScreenRect()
{
    QDesktopWidget *desktop = QApplication::desktop();
    if (m_isTable) {
        return desktop->screenGeometry();
    }
    QRect screenRect;
    if (m_screenIndex == 0) {
        screenRect = desktop->availableGeometry();
        if ((desktop->screenCount() > 1) && m_panelSetting) {
            int panelPosition = m_panelSetting->get("panelposition").toInt();
            int panelHeight = 0;
            int panelWidth = 0;
            switch (panelPosition) {
                case 0: {  // 下
                    panelHeight = m_panelSetting->get("panelsize").toInt();
                    break;
                }
                case 1: {  // 上
                    panelHeight = m_panelSetting->get("panelsize").toInt();
                    break;
                }
                case 2: {  // 左
                    panelWidth = m_panelSetting->get("panelsize").toInt();
                    break;
                }
                case 3: {  // 右
                    panelWidth = m_panelSetting->get("panelsize").toInt();
                    break;
                }
                default:
                    break;
            }
            screenRect.setHeight(screenRect.height() - panelHeight);
            screenRect.setWidth(screenRect.width() - panelWidth);
        }
    } else {
        QList<QScreen *> screenList = QGuiApplication::screens();
        screenRect = screenList[m_screenIndex]->geometry();
    }
    return screenRect;
}

void VideoForm::setScreenIndex()
{
    QDesktopWidget *desktop = QApplication::desktop();
    for (int i = 0; i < desktop->screenCount(); i++) {
        if (desktop->screenGeometry(i).contains(this->geometry())) {
            m_screenIndex = i;
            return;
        }
    }
}

void VideoForm::moveCenter()
{
    QRect screenRect = getScreenRect();

    if (screenRect.isEmpty()) {
        qWarning() << "getScreenRect is empty";
        return;
    }
    // 垂直窗口在最右侧，横屏时窗口居中
    if (!m_isTable && m_isVertical) {
        kdk::WindowManager::setGeometry(
            windowHandle(),
            QRect((screenRect.topRight() - QRect(0, 0, size().width(), size().height()).topRight()), size()));
    } else {
        kdk::WindowManager::setGeometry(
            windowHandle(),
            QRect((screenRect.center() - QRect(0, 0, size().width(), size().height()).center()), size()));
    }
}

void VideoForm::installShortcut()
{
    QShortcut *shortcut = nullptr;

    // postGoHome
    shortcut = new QShortcut(QKeySequence("Ctrl+h"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postGoHome();
    });

    // postGoBack
    shortcut = new QShortcut(QKeySequence("Ctrl+b"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postGoBack();
    });

    // postAppSwitch
    shortcut = new QShortcut(QKeySequence("Ctrl+s"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postAppSwitch();
    });

    // postGoMenu
    shortcut = new QShortcut(QKeySequence("Ctrl+m"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postGoMenu();
    });

    // postVolumeUp
    shortcut = new QShortcut(QKeySequence("Ctrl+up"), this);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postVolumeUp();
    });

    // postVolumeDown
    shortcut = new QShortcut(QKeySequence("Ctrl+down"), this);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postVolumeDown();
    });

    // postPower
    shortcut = new QShortcut(QKeySequence("Ctrl+p"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postPower();
    });

    // setScreenPowerMode(ControlMsg::SPM_OFF)
    shortcut = new QShortcut(QKeySequence("Ctrl+o"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->setScreenPowerMode(ControlMsg::SPM_OFF);
    });

    // expandNotificationPanel
    shortcut = new QShortcut(QKeySequence("Ctrl+n"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->expandNotificationPanel();
    });

    // collapsePanel
    shortcut = new QShortcut(QKeySequence("Ctrl+Shift+n"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->collapsePanel();
    });

    // copy
    shortcut = new QShortcut(QKeySequence("Ctrl+c"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postCopy();
    });

    // cut
    shortcut = new QShortcut(QKeySequence("Ctrl+x"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postCut();
    });

    // clipboardPaste
    shortcut = new QShortcut(QKeySequence("Ctrl+v"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->setDeviceClipboard();
    });

    // setDeviceClipboard
    shortcut = new QShortcut(QKeySequence("Ctrl+Shift+v"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->clipboardPaste();
    });

    // 全屏
    shortcut = new QShortcut(QKeySequence("F11"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (!m_device) {
            return;
        }
        if (!m_isFull) {
            ui->title->fullScreen();
        }
    });
}

void VideoForm::updateShowSize(const QSize &newSize)
{
    m_widthHeightRatio = 1.0f * newSize.width() / newSize.height();
    ui->keepRatioWidget->setWidthHeightRatio(m_widthHeightRatio);
    m_isVertical = m_widthHeightRatio < 1.0f;
    if (m_frameSize != newSize) {
        m_frameSize = newSize;
        windowZoom();
        moveCenter();
    }
}

void VideoForm::setDevice(BaseDevice *device)
{
    m_device = device;
    ui->toolform->setDevice(m_device);
}

bool VideoForm::eventFilter(QObject *watch, QEvent *event)
{
    if (watch == this) {
        switch (event->type()) {
            case QEvent::InputMethod: {
                this->changeInputBoxPos();
                QInputMethodEvent *inputMethodEvent = (QInputMethodEvent *)event;
                if (!hasFocus()) {  //失去焦点时，输入法事件优先于FocusOut事件被处理
                    inputMethodEvent->setCommitString("");  //将失去焦点后提交的字符串设置为空
                } else {
                    QString commitText = inputMethodEvent->commitString();
                    if (!commitText.isEmpty()) {
                        qDebug() << "commitText = " << commitText;
                        Q_EMIT m_device->sigInputChinese(commitText);
                    }
                }
                return true;
            }
            case QEvent::KeyPress: {
                QKeyEvent *keyEvent = (QKeyEvent *)event;
                if (keyEvent->key() == Qt::Key_Escape && m_isFull) {
                    ui->title->fullScreen();
                    return true;
                }
                if (!m_device) {
                    return false;
                }
                Q_EMIT m_device->keyEvent(keyEvent, m_videoWidget->frameSize(), m_videoWidget->size());
                return true;
            }
            case QEvent::KeyRelease: {
                QKeyEvent *keyEvent = (QKeyEvent *)event;
                if (!m_device) {
                    return false;
                }

                Q_EMIT m_device->keyEvent(keyEvent, m_videoWidget->frameSize(), m_videoWidget->size());
                return true;
            }
            case QEvent::Paint: {
                QStyleOption opt;
                opt.init(this);
                QPainter p(this);
                style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
                return true;
            }
            case QEvent::Close: {
                Q_EMIT m_device->sigVideoClose();
                return true;
            }
            case QEvent::Resize: {
                if (windowState() != Qt::WindowNoState) {
                    return true;
                }
                if (m_isMax || m_isFull) {
                    return true;
                }
                if (!m_resizeAuto) {
                    m_oldSize = size();
                    m_resizeAuto = true;
                    return true;
                }

                if (m_timerId) {
                    killTimer(m_timerId);
                    m_timerId = 0;
                }
                m_timerId = startTimer(400);
                if (m_toolForm) {
                    // m_toolForm->setFixedWidth(width());
                    setGeometry(QRect(QPoint(0, height() - 40), m_toolForm->size()));
                }
            }
            default:
                break;
        }
    }
    if (watch == ui->keepRatioWidget) {
        switch (event->type()) {
            case QEvent::MouseButtonPress: {
                QMouseEvent *mouseEvent = (QMouseEvent *)event;
                if (mouseEvent->button() == Qt::MiddleButton) {
                    if (m_device && !m_device->isCurrentCustomKeymap()) {
                        return false;
                    }
                }
                if (mouseEvent->button() == Qt::RightButton) {
                    if (m_device && !m_device->isCurrentCustomKeymap()) {
                        return false;
                    }
                }

                if (m_videoWidget->geometry().contains(mouseEvent->pos())) {
                    if (!m_device) {
                        return false;
                    }
                    mouseEvent->setLocalPos(
                        m_videoWidget->mapFrom(ui->keepRatioWidget, mouseEvent->localPos().toPoint()));
                    Q_EMIT m_device->mouseEvent(mouseEvent, m_videoWidget->frameSize(), m_videoWidget->size());
                } else {
                    if (mouseEvent->button() == Qt::LeftButton) {
                        m_dragPosition = mouseEvent->globalPos() - frameGeometry().topLeft();
                        mouseEvent->accept();
                    }
                }
                return true;
            }
            case QEvent::Enter: {
                this->setCursor(Qt::ArrowCursor);
                return true;
            }
            case QEvent::MouseMove: {
                QMouseEvent *mouseEvent = (QMouseEvent *)event;
                if (m_videoWidget->geometry().contains(mouseEvent->pos())) {
                    if (!m_device) {
                        return false;
                    }
                    mouseEvent->setLocalPos(
                        m_videoWidget->mapFrom(ui->keepRatioWidget, mouseEvent->localPos().toPoint()));
                    Q_EMIT m_device->mouseEvent(mouseEvent, m_videoWidget->frameSize(), m_videoWidget->size());
                } else if (!m_dragPosition.isNull()) {
                    if (mouseEvent->buttons() & Qt::LeftButton) {
                        setGeometry(QRect((mouseEvent->globalPos() - m_dragPosition), size()));
                        mouseEvent->accept();
                    }
                }
                return true;
            }
            case QEvent::MouseButtonRelease: {
                QMouseEvent *mouseEvent = (QMouseEvent *)event;
                if (m_dragPosition.isNull()) {
                    if (!m_device) {
                        return false;
                    }
                    mouseEvent->setLocalPos(
                        m_videoWidget->mapFrom(ui->keepRatioWidget, mouseEvent->localPos().toPoint()));
                    // local check
                    QPointF local = mouseEvent->localPos();
                    if (local.x() < 0) {
                        local.setX(0);
                    }
                    if (local.x() > m_videoWidget->width()) {
                        local.setX(m_videoWidget->width());
                    }
                    if (local.y() < 0) {
                        local.setY(0);
                    }
                    if (local.y() > m_videoWidget->height()) {
                        local.setY(m_videoWidget->height());
                    }
                    mouseEvent->setLocalPos(local);
                    Q_EMIT m_device->mouseEvent(mouseEvent, m_videoWidget->frameSize(), m_videoWidget->size());
                } else {
                    m_dragPosition = QPoint(0, 0);
                }
                return true;
            }
            case QEvent::MouseButtonDblClick: {
                QMouseEvent *mouseEvent = (QMouseEvent *)event;
                if (m_videoWidget->geometry().contains(mouseEvent->pos())) {
                    if (!m_device) {
                        return false;
                    }
                    mouseEvent->setLocalPos(
                        m_videoWidget->mapFrom(ui->keepRatioWidget, mouseEvent->localPos().toPoint()));
                    Q_EMIT m_device->mouseEvent(mouseEvent, m_videoWidget->frameSize(), m_videoWidget->size());
                }
                return true;
            }
            case QEvent::Wheel: {
                QMouseEvent *mouseEvent = (QMouseEvent *)event;
                if (m_videoWidget->geometry().contains(mouseEvent->pos())) {
                    if (!m_device) {
                        return false;
                    }
                    QPoint globalPos = mouseEvent->globalPos();
                    QPointF pos = m_videoWidget->mapFrom(ui->keepRatioWidget, mouseEvent->pos());
                    QWheelEvent *wheelEvent1 = (QWheelEvent *)event;
                    QWheelEvent wheelEvent(pos, globalPos, wheelEvent1->pixelDelta(), wheelEvent1->angleDelta(),
                                           wheelEvent1->buttons(), wheelEvent1->modifiers(), wheelEvent1->phase(),
                                           wheelEvent1->inverted());
                    Q_EMIT m_device->wheelEvent(&wheelEvent, m_videoWidget->frameSize(), m_videoWidget->size());
                }
                return true;
            }
            default:
                break;
        }
    }
    if (watch == ui->toolform) {
        switch (event->type()) {
            case QEvent::Enter: {
                this->setCursor(Qt::ArrowCursor);
                return true;
            }
            default:
                break;
        }
    }
    return QWidget::eventFilter(watch, event);
}

void VideoForm::timerEvent(QTimerEvent *event)
{
    qInfo() << "Start scaling to scale...";
    m_resizeAuto = false;
    killTimer(event->timerId());
    m_timerId = 0;
    windowZoom();
}

void VideoForm::changeEvent(QEvent *event)
{
    if (QEvent::WindowStateChange == event->type()) {
        Qt::WindowStates state = windowState();
        qInfo() << "VideoForm state change: " << state;
        switch (state) {
            case Qt::WindowNoState: {
                m_isMax = false;
                m_isFull = false;
                ui->title->show();
                if (m_isToolFormShow) {
                    ui->toolform->show();
                }
                ui->title->setMaxBtnMode(false);
                if (m_isWindeStateChanged) {
                    m_isWindeStateChanged = false;
                    windowZoom();
                    moveCenter();
                }
            } break;
            case Qt::WindowMaximized: {
                m_isMax = true;
                m_isFull = false;
                ui->title->show();
                if (m_isToolFormShow) {
                    ui->toolform->show();
                }
                ui->title->setMaxBtnMode(true);
            } break;
            case Qt::WindowFullScreen: {
                m_isFull = true;
                m_isMax = false;
                ui->title->hide();
                ui->toolform->hide();
            } break;
        }
    }
}

void VideoForm::onGetInputState()
{
    qInfo() << "get chinese input state";
    this->setAttribute(Qt::WA_InputMethodEnabled, true);
    this->setFocusPolicy(Qt::WheelFocus);
    this->setFocus();
}

void VideoForm::onGetInputPos(int x, int y)
{
    m_inputX = x;
    m_inputY = y;
    changeInputBoxPos();
}

void VideoForm::onLostInputState()
{
    qInfo() << "lost chinese input state";
    this->setFocusPolicy(Qt::NoFocus);
    this->setAttribute(Qt::WA_InputMethodEnabled, false);
}

void VideoForm::changeInputBoxPos()
{
    if (testAttribute(Qt::WA_InputMethodEnabled)) {
        QTransform t;
        QPoint p = cursorPoint();
        t.translate(p.x() - this->width() / 2, p.y() - this->height());  //平移x,y
        QGuiApplication::inputMethod()->setInputItemTransform(t);
        QGuiApplication::inputMethod()->setInputItemRectangle(this->rect());
        QGuiApplication::inputMethod()->update(Qt::ImQueryInput);
    }
}

QPoint VideoForm::cursorPoint()
{
    QPoint point;
    point = this->mapTo(this->parentWidget(), QPoint(m_inputX * this->width() / m_frameSize.width(),
                                                     m_inputY * this->height() / m_frameSize.height()));

    point.setY(point.y() + 30);
    //如果输入坐标位置 + 50 > 屏幕分辨率端高度，此时输入法框会盖住输入框，需要进行如下处理，其中使用的50和150待优化
    const QDesktopWidget *desktop = QApplication::desktop();
    if (desktop) {
        QRect rect = desktop->screenGeometry();
        if (point.y() + 50 > rect.height()) {
            point.setY(point.y() - 200);
        }
    }

    return point;
}

void VideoForm::windowZoom()
{
    // QSize currentSize = size();
    int w = width();
    int h = height();
    setScreenIndex();
    QRect screenRect = getScreenRect();
    if (m_oldSize.height() == 0 || (m_isVertical != m_normalVertical)) {
        if (m_isMax || m_isFull) {
            m_isWindeStateChanged = true;
            m_resizeAuto = false;
        } else {
            h = screenRect.height();
            w = (screenRect.height() - m_titleHeight) * m_widthHeightRatio;
            m_resizeAuto = false;
            resize(w, h);
        }
        m_normalVertical = m_isVertical;
        return;
    }
    if (m_oldSize == size()) {
        return;
    }
    if (m_oldSize.height() == h) {
        h = (w / m_widthHeightRatio) + m_titleHeight;
    } else {
        w = (h - m_titleHeight) * m_widthHeightRatio;
    }
    if (h >= screenRect.height()) {
        h = screenRect.height();
        w = (screenRect.height() - m_titleHeight) * m_widthHeightRatio;
    } else if (h <= (screenRect.height() / 2)) {
        h = screenRect.height() / 2;
        w = (h - m_titleHeight) * m_widthHeightRatio;
    }
    if (m_toolForm) {
        setGeometry(QRect(QPoint(0, h - TITLE_HEIGHT), m_toolForm->size()));
    }
    m_resizeAuto = false;
    resize(w, h);
}

void VideoForm::showToolForm(bool show)
{
    if (show) {
        m_isToolFormShow = true;
        m_titleHeight = TITLE_HEIGHT + TRIPE_HEIGHT;
        ui->toolform->setFixedHeight(40);
    } else {
        m_isToolFormShow = false;
        m_titleHeight = TITLE_HEIGHT;
        ui->toolform->setFixedHeight(0);
    }
}

void VideoForm::onToolFormStatusChange()
{
    if (m_device->getDeviceStatus()) {
        showToolForm(!m_isToolFormShow);
    } else {
        TipWidget *tipWidget = new TipWidget();
        tipWidget->setParent(this);
        tipWidget->SetMesseage(tr("Control device not supported"), nullptr, 20);
    }
}

void VideoForm::onModeChanged(bool isTable)
{
    m_isTable = isTable;
    ui->title->changeMode(m_isTable);
}

void VideoForm::onModeRotationsChange(QString rotaion)
{
    m_isWindeStateChanged = true;
}

void VideoForm::onScrollTrigger()
{
    ScrollSettingWidget *scrollWidget = new ScrollSettingWidget(this);
    scrollWidget->show();
}

void VideoForm::initPanelSetting()
{
    if (QGSettings::isSchemaInstalled(UKUI_TASK_GSETTING_PATH)) {
        qInfo() << "init panel gsettings";
        m_panelSetting = new QGSettings(UKUI_TASK_GSETTING_PATH);
    }
}