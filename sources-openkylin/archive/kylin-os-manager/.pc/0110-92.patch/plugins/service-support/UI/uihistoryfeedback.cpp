#include "uihistoryfeedback.h"
#include <QHeaderView>
#include <QBoxLayout>
#include <QDebug>
#include <gsettingmonitor.h>
#include <kborderlessbutton.h>
#include <QToolTip>
#include <QDateTime>
#include <QMessageBox>
#include <libkydate.h>
#include "feedbackmanager.h"
#include "service-support-backend/src/settings.h"

UiHistoryFeedback::UiHistoryFeedback(QWidget *parent) : QWidget(parent)
{
    setObjectName(m_objectName);
    //正常情况下显示的表格
    initTableUI();
    //异常情况下显示的占位图和文字提示
    initErrorUI();

    QHBoxLayout *hlBase = new QHBoxLayout(this);
    hlBase->setMargin(0);
    hlBase->addWidget(m_tableWidget);
    hlBase->addWidget(m_otherWidget);

    connect(FeedbackManager::getInstance(), &FeedbackManager::historyInfo, this, &UiHistoryFeedback::historyInfo);
}

void UiHistoryFeedback::indexChanged(QString name)
{
    if (name == m_objectName) {
        //切换到此标签页时，向服务端请求一次数据
        m_otherWidget->show();
        m_tableWidget->hide();
        QString bugs = Settings::getHistoryBug();
        QStringList tmp = bugs.split(",");
        tmp.removeAll("");
        if (tmp.isEmpty()) {
            m_iconType = Empty;
            m_labelText->setText(m_emptyText);
            setIconPix();
            return;
        }
        m_pageWidget->setTotalItem(tmp.length());
        m_pageWidget->refresh();
        FeedbackManager::getInstance()->getHistoryData(m_PageIndex);
        m_iconType = Loading;
        m_labelText->setText(m_loadingText);
        m_retryBtn->hide();
        setIconPix();
    }
}

void UiHistoryFeedback::historyInfo(const QList<HistoryInfo> &infolist)
{
    if (infolist.isEmpty()) {
        //历史记录为空不会去发起请求，所以为空的情况是错误
        m_iconType = Error;
        m_labelText->setText(m_errorText);
        m_retryBtn->show();
        setIconPix();
        return;
    }
    m_table->clear();
    for (int i = 0; i < infolist.length(); i++) {
        HistoryInfo info = infolist.at(i);
        QString active = info.progress;
        QString resolution = info.solution;
        if (resolution == "bydesign") {
            resolution = tr("bydesign");
        } else if (resolution == "duplicate") {
            resolution = tr("duplicate");
        } else if (resolution == "external") {
            resolution = tr("external");
        } else if (resolution == "fixed") {
            resolution = tr("fixed");
        } else if (resolution == "notrepro") {
            resolution = tr("notrepro");
        } else if (resolution == "postponed") {
            resolution = tr("postponed");
        } else if (resolution == "willnotfix") {
            resolution = tr("willnotfix");
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(
            QStringList() << info.creatTime.toString(QString(kdk_system_get_shortformat()) + " hh:mm:ss") << info.type
                          << info.message);
        m_table->addTopLevelItem(item);
        QLabel *widget = new QLabel(m_table);
        widget->setContentsMargins(8, 0, 0, 0);
        widget->setFixedHeight(40);
        if (active == "已验收" || active == "已发布" || active == "已完成" || active == "已拒绝") {
            QPalette pal;
            pal.setColor(QPalette::Text, QColor(Qt::lightGray));
            widget->setPalette(pal);
        } else if (active == "已修复") {
            QVBoxLayout *vl = new QVBoxLayout(widget);
            kdk::KBorderlessButton *btn = new kdk::KBorderlessButton(widget);
            btn->setText(tr("verify"));
            btn->setWhatsThis(info.id);
            connect(btn, &kdk::KBorderlessButton::clicked, this, [=] {
                QMessageBox *box = new QMessageBox(QMessageBox::Question, "", tr("Has the issue been resolved?"),
                                                   QMessageBox::NoButton, btn);
                box->setInformativeText(
                    tr("Once identified, the issue will be closed and no further action will be taken."));
                box->setAttribute(Qt::WA_DeleteOnClose);
                QPushButton *resolved = new QPushButton(tr("resolved"), box);
                resolved->setProperty("isImportant", true);
                QPushButton *cancel = new QPushButton(tr("cancel"), box);
                cancel->setProperty("useButtonPalette", true);
                box->setDefaultButton(cancel);
                box->addButton(cancel, QMessageBox::RejectRole);
                box->addButton(resolved, QMessageBox::AcceptRole);
                box->setDefaultButton(resolved);
                connect(resolved, &QPushButton::clicked, this, [=] {
                    btn->setEnabled(false);
                    FeedbackManager::getInstance()->closeBug(btn->whatsThis());
                });
                box->exec();
            });
            vl->addWidget(btn);
            vl->setMargin(0);
            active = "";
        }
        widget->setText(active);
        m_table->setItemWidget(item, 3, widget);
    }
    m_otherWidget->hide();
    m_tableWidget->show();
}

void UiHistoryFeedback::setTextToolTips(QTreeWidgetItem *item, int indexCol)
{
    QString tempText = item->text(indexCol);

    QFontMetrics fontWidth = QFontMetrics(item->font(indexCol));

    if (fontWidth.width(tempText) > m_table->columnWidth(indexCol) - 16) {
        QToolTip::showText(QCursor::pos(), tempText);
    }
}


void UiHistoryFeedback::initTableUI()
{
    m_tableWidget = new QWidget(this);
    m_table = new QTreeWidget(m_tableWidget);
    m_table->setFixedHeight(444);
    // 将表格变为禁止编辑,取消焦点
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setFocusPolicy(Qt::NoFocus);
    // 设置表格不可选中
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    // 设置水平方向的表头标签与垂直方向上的表头标签，注意必须在初始化行列之后进行，否则，没有效果
    QStringList header = {tr("Creation time"), tr("Type"), tr("Description"), tr("Progress")}; //处理方案
    m_table->setHeaderLabels(header);
    //设置表头文字左对齐
    m_table->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    //设置每列最小宽度
    m_table->header()->setMinimumSectionSize(100);
    //设置交替背景色
    m_table->setAlternatingRowColors(true);
    m_table->setColumnWidth(0, 175);
    m_table->setColumnWidth(1, 107);
    m_table->setColumnWidth(2, 300);
    m_table->setColumnWidth(3, 100);
    m_table->header()->setFixedHeight(40);
    m_table->setUniformRowHeights(true);
    m_table->setMouseTracking(true);
    m_table->setRootIsDecorated(false);
    connect(m_table, &QTreeWidget::itemEntered, this, &UiHistoryFeedback::setTextToolTips);

    m_pageWidget = new PaginationWid(m_tableWidget);
    connect(m_pageWidget, &PaginationWid::pageChanged, this, [=](int pageNum) {
        m_PageIndex = pageNum;
        FeedbackManager::getInstance()->getHistoryData(m_PageIndex);
    });
    QHBoxLayout *hl = new QHBoxLayout;
    hl->setMargin(0);
    hl->addStretch(9);
    hl->addWidget(m_pageWidget);

    QVBoxLayout *vl = new QVBoxLayout(m_tableWidget);
    vl->setContentsMargins(40, 0, 40, 24);
    vl->setSpacing(0);
    vl->addWidget(m_table);
    vl->addSpacing(24);
    vl->addStretch(9);
    vl->addLayout(hl);
}

void UiHistoryFeedback::initErrorUI()
{
    m_emptyText = tr("No record");
    m_errorText = tr("There is a network problem, please try again later");
    m_loadingText = tr("Loading, please wait");
    m_otherWidget = new QWidget(this);
    m_otherWidget->hide();
    m_labelText = new QLabel(m_otherWidget);
    QPalette pal = m_labelText->palette();
    pal.setColor(QPalette::Text, Qt::lightGray);
    m_labelText->setPalette(pal);
    m_labelIcon = new QPushButton(m_otherWidget);
    m_labelIcon->setFixedSize(96, 96);
    m_labelIcon->setIconSize(QSize(96, 96));
    m_labelIcon->setFlat(true);
    m_labelIcon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_retryBtn = new QPushButton(m_otherWidget);
    m_retryBtn->setText(tr("retry"));
    m_retryBtn->setFixedHeight(36);
    m_retryBtn->setMinimumWidth(96);
    m_labelText->setText(m_loadingText);
    connect(m_retryBtn, &QPushButton::clicked, this, [=] {
        m_retryBtn->hide();
        m_iconType = Loading;
        m_labelText->setText(m_loadingText);
        setIconPix();
        FeedbackManager::getInstance()->getHistoryData(m_PageIndex);
    });
    m_retryBtn->hide();
    QVBoxLayout *vlIconWidget = new QVBoxLayout(m_otherWidget);
    vlIconWidget->addStretch(9);
    vlIconWidget->addWidget(m_labelIcon);
    vlIconWidget->addWidget(m_labelText);
    vlIconWidget->addWidget(m_retryBtn);
    vlIconWidget->addStretch(9);
    vlIconWidget->addSpacing(80);
    vlIconWidget->setAlignment(m_labelIcon, Qt::AlignHCenter);
    vlIconWidget->setAlignment(m_labelText, Qt::AlignHCenter);
    vlIconWidget->setAlignment(m_retryBtn, Qt::AlignHCenter);
    setIconPix();
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this,
            &UiHistoryFeedback::setIconPix);
}

void UiHistoryFeedback::setIconPix()
{
    QString theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
    if (m_iconType == Loading) {
        if (theme == "ukui-default" || theme == "ukui-light") {
            m_labelIcon->setIcon(QIcon(":/res/loding-light.png"));
        } else {
            m_labelIcon->setIcon(QIcon(":/res/loding-dark.png"));
        }
    } else if (m_iconType == Error) {
        if (theme == "ukui-default" || theme == "ukui-light") {
            m_labelIcon->setIcon(QIcon(":/res/lodingerr-light.png"));
        } else {
            m_labelIcon->setIcon(QIcon(":/res/lodingerr-dark.png"));
        }
    } else if (m_iconType == Empty) {
        if (theme == "ukui-default" || theme == "ukui-light") {
            m_labelIcon->setIcon(QIcon(":/res/empty-light.png"));
        } else {
            m_labelIcon->setIcon(QIcon(":/res/empty-dark.png"));
        }
    }
}
