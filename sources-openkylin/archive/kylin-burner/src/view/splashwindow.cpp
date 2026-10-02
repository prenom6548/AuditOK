#include "splashwindow.h"
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QIcon>
#include <QPalette>

#include "src/common/utils.h"
#include "src/frame/xatom-helper.h"

#include "v11sdk.h"

SplashWindow::SplashWindow(QWidget *parent)
    : QWidget(parent)
    , m_logoLabel(nullptr)
    , m_mainLayout(nullptr)
    , m_fadeAnimation(nullptr)
{
    setupUI();

    if (parentWidget()) {
        parentWidget()->installEventFilter(this);
        syncToParent();
    } else {
        centerOnScreen();
    }
    
    // 初始化窗口透明度为1.0（完全不透明）
    setWindowOpacity(1.0);

    // 创建淡出动画
    m_fadeAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_fadeAnimation->setDuration(300);
    m_fadeAnimation->setStartValue(1.0);
    m_fadeAnimation->setEndValue(0.0);
    m_fadeAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    // 动画结束时隐藏窗口
    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, &QWidget::hide);
}

SplashWindow::~SplashWindow()
{
}

void SplashWindow::setupUI()
{
    setWindowTitle("SplashWindow");
    if (parentWidget()) {
        setGeometry(parentWidget()->rect());
        setAttribute(Qt::WA_StyledBackground, true);
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, palette().window().color());
        setPalette(pal);
    } else {
        // 设置窗口属性
        ukuiWaylandRemoveTitleBar(this);
        setFixedSize(900, 600);
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);

        //判断是否是wayland环境
        if(SystemInfo::m_isWayland) {
            ukuiWaylandWindowStyleChange(this);
        } else {
            MotifWmHints hints;
            hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
            hints.functions = MWM_FUNC_ALL;
            hints.decorations = MWM_DECOR_BORDER;
            XAtomHelper::getInstance()->setWindowMotifHint(winId(), hints);
        }
    }
    
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // 创建Logo
    m_logoLabel = new QLabel(this);
    m_logoLabel->setPixmap(QIcon::fromTheme("burner").pixmap(100, 100));
    m_logoLabel->setAlignment(Qt::AlignCenter);
    
    // 组装布局
    m_mainLayout->addWidget(m_logoLabel);
    
    setLayout(m_mainLayout);
}

void SplashWindow::centerOnScreen()
{
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        QRect windowGeometry = this->geometry();
        move((screenGeometry.width() - windowGeometry.width()) / 2,
             (screenGeometry.height() - windowGeometry.height()) / 2);
    }
}

bool SplashWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget()) {
        switch (event->type()) {
        case QEvent::Resize:
        case QEvent::Show:
            syncToParent();
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void SplashWindow::syncToParent()
{
    if (!parentWidget()) {
        return;
    }

    setGeometry(parentWidget()->rect());
    raise();
}

void SplashWindow::fadeOut(int duration)
{
    if (m_fadeAnimation) {
        // 如果动画正在运行，先停止
        if (m_fadeAnimation->state() == QPropertyAnimation::Running) {
            m_fadeAnimation->stop();
        }
        m_fadeAnimation->setDuration(duration);
        m_fadeAnimation->setStartValue(windowOpacity());
        m_fadeAnimation->setEndValue(0.0);
        m_fadeAnimation->start();
    } else {
        // 如果动画未初始化，直接隐藏
        hide();
    }
}

