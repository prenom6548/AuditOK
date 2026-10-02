#include "passivitywidget.h"
#include <glib.h>
#include <QIcon>
#include <QLabel>
#include <QBoxLayout>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QApplication>
#include <QTextEdit>
#include <QScreen>
#include <QScrollArea>
#include <kwindowinfo.h>
#include <kwindowsystem.h>
#include <gsettingmonitor.h>
#include "kom-label.h"
#include "kom_buriedpoint.h"
#include "blacklist.h"

#define CALL_400 "400-089-1870"
#define MAIL_400 "support@kylinos.cn"

PassivityWidget::PassivityWidget(QWidget *parent)
    : kdk::KWidget(parent)
{
    exec_desktop_map_ = {
        {"kybackup", "yhkylin-backup-tools"},
        {"kylin-log-viewer", "logview"}
    };
}

void PassivityWidget::run() {
    if (m_isInit) {
        return;
    }
    m_isInit = true;

    if (m_mode == "2") {
        /*
         * 带界面应用崩溃后的业务逻辑：
         * 1. 先进行上报
         * 2. 上报结束后，弹出界面（不论上报成功还是失败）
         */
        connect(FeedbackManager::getInstance(),&FeedbackManager::creatFinish,this,&PassivityWidget::creatFinish, Qt::ConnectionType::UniqueConnection);
    } else if (m_mode == "3") {
        connect(FeedbackManager::getInstance(),&FeedbackManager::creatFinish,this,&PassivityWidget::creatFinishNoUI, Qt::ConnectionType::UniqueConnection);
    }

    // 上报
    startCollecting(true);
}

void PassivityWidget::creatFinish(FeedBackFinishType type, QString str) {
    switch (type) {
        case Success:
            qInfo() << "Received a crash from " << m_exe << ", problem reported successfully";
            break;
        case Oversize:
            qCritical() << "Received a crash from " << m_exe << ", problem reporting failed because the annex is too large";
            break;
        case UploadFail:
            qCritical() << "Received a crash from " << m_exe << ", problem reporting failed";
            break;
        default:
            qCritical() << "Received a crash from " << m_exe << ", don't know the ending type";
            break;
    }

    // 如果 restart 参数为空，上报完成后则不弹出界面，直接退出
    if (m_resart.isEmpty()) {
        qWarning() << "Received a crash from " << m_exe << ", the interface does not display without a restart command";
        // 不会立刻退出，需进入事件循环后才会退出，所以仍需要 return 关键字
        qApp->quit();
        return;
    }

    // 上报完成，显示界面
    initUI();
}

void PassivityWidget::creatFinishNoUI(FeedBackFinishType type, QString str)
{
    switch (type) {
        case Success: {
            QMap<QString, QString> custom_property = {
                {"applicationName", m_exe},
                {"errorCode",       m_code},
                {"submitTime",      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")},
                {"taskNumber",      str}
            };
            kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kServiceSupportMini,
                                            kom::BuriedPoint::EventCode::kFaultDiagnosisReport,
                                            custom_property);

            qDebug() << "提交成功";
            exit(0);
            break;
        }
        case Oversize:
            qDebug() << "压缩包总大小超过限制";
            exit(-2);
            break;
        case UploadFail:
            qDebug() << "提交失败";
            exit(-3);
            break;
        default:
            break;
    }
}

void PassivityWidget::initUI() {
    setFixedSize(452, 257);
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setWindowIcon(QIcon::fromTheme("kylin-os-manager"));
    setWindowTitle(tr("Kylin Os Manager"));
    setIcon(QIcon(":/res/dialog-warning.png"));
    setWidgetName(tr("Unexpected exit reminder"));

    windowButtonBar()->menuButton()->hide();
    windowButtonBar()->minimumButton()->hide();
    windowButtonBar()->maximumButton()->hide();

    // 隐藏任务栏图标
    const KWindowInfo window_info(winId(), NET::WMState);
    if (!window_info.hasState(NET::SkipTaskbar) || !window_info.hasState(NET::SkipPager)) {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    }

    // 标题
    auto *title_label = new kom::KomLabel(this);
    title_label->setBold(true);
    title_label->setText(tr("\"%1\" unexpected exit").arg(ExecName()));

    // 描述
    auto *text_edit = new QTextEdit(this);
    text_edit->setReadOnly(true);
    text_edit->setFrameShape(QFrame::Shape::NoFrame);
    text_edit->setText(tr("Click \"Reopen\" to reopen the application, and this report will be automatically sent to kylin operations personnel."));
    // 在驱动阶段调整 QTextEdit 的高度与文本高度一致，只有在界面完全显示结束进入事件循环后，才能获取到正确的 QTextDocument 的大小
    auto adjust_text_edit_heigth = [this, text_edit]() {
        QTimer::singleShot(0, this, [text_edit]() {
            auto height = text_edit->document()->size().toSize().height();
            text_edit->setFixedHeight(height);
        });
    };
    QTimer::singleShot(0, this, adjust_text_edit_heigth);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, adjust_text_edit_heigth);

    // 7 天内该程序崩溃不再弹出此窗口
    auto *prohibit_popup_check_box = new QCheckBox(this);
    prohibit_popup_check_box->setChecked(true);
    auto *prohibit_popup_label = new kom::KomLabel(this);
    prohibit_popup_label->setText(tr("%1 unexpectedly exited within 7 days and no longer pops up").arg(ExecName()));
    auto *prohibit_popup_hbox_layout = new QHBoxLayout;
    prohibit_popup_hbox_layout->setContentsMargins(0, 0, 0, 0);
    prohibit_popup_hbox_layout->setSpacing(0);
    prohibit_popup_hbox_layout->addWidget(prohibit_popup_check_box);
    prohibit_popup_hbox_layout->addWidget(prohibit_popup_label);
    prohibit_popup_hbox_layout->addStretch();
    connect(prohibit_popup_check_box, &QCheckBox::toggled, this, &PassivityWidget::on_UpdateBlacklist);

    // 默认 7 天内不再弹出
    on_UpdateBlacklist(true);

    // 按钮
    auto *cancel_btn = new QPushButton(this);
    cancel_btn->setText(tr("Cancel"));
    connect(cancel_btn, &QPushButton::clicked, this, []{
        qApp->quit();
    });

    auto *reopen_btn = new QPushButton(this);
    reopen_btn->setText(tr("Reopen"));
    reopen_btn->setProperty("isImportant", true);
    connect(reopen_btn, &QPushButton::clicked, this, [this]() {
        QProcess::startDetached(m_resart);
        qApp->quit();
    });

    auto *btn_hbox_layout = new QHBoxLayout;
    btn_hbox_layout->setContentsMargins(0, 0, 0, 0);
    btn_hbox_layout->setSpacing(0);
    btn_hbox_layout->addStretch();
    btn_hbox_layout->addWidget(cancel_btn);
    btn_hbox_layout->addSpacing(8);
    btn_hbox_layout->addWidget(reopen_btn);

    // 整体布局
    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(24, 16, 24, 24);
    vbox_layout->setSpacing(0);
    vbox_layout->addWidget(title_label);
    vbox_layout->addWidget(text_edit);
    vbox_layout->addSpacing(8);
    vbox_layout->addLayout(prohibit_popup_hbox_layout);
    vbox_layout->addStretch();
    vbox_layout->addLayout(btn_hbox_layout);

    // 将布局放入滚动区域
    auto *scroll_widget = new QWidget(this);
    scroll_widget->setFixedWidth(width());
    scroll_widget->setLayout(vbox_layout);
    auto *scroll_area = new QScrollArea(this);
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setFrameShape(QScrollArea::NoFrame);
    scroll_area->setWidget(scroll_widget);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setContentsMargins(0, 0, 0, 0);
    hbox_layout->setSpacing(0);
    hbox_layout->addWidget(scroll_area);
    baseBar()->setLayout(hbox_layout);

    scroll_area->setWidgetResizable(true);

    const auto primary_screen = QGuiApplication::screenAt(QCursor::pos())->geometry();
    const auto x = primary_screen.width() / 2 - width() / 2;
    const auto y = primary_screen.height() / 2 - height() / 2;
    move(x, y);
    show();
}

void PassivityWidget::on_UpdateBlacklist(bool ok) {
    if (ok) {
        Blacklist::Add(m_exe);
    } else {
        Blacklist::Remove(m_exe);
    }
}
