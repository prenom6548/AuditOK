/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "backuplistwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMimeData>
#include <QFileInfo>
#include <QScrollBar>
#include <QDebug>

#include "../messageboxutils.h"
#include "../gsettingswrapper.h"
#include "../globalbackupinfo.h"
#include "component/imageutil.h"

#define WIDTH_ITEM 36
#define MERGE_IN 8
#define WIDTH_SPACING 5

MyItemWidget::MyItemWidget(QWidget* parent, QListWidgetItem *item) :
    QWidget(parent),
    m_item(item)
{
    if (parent && item)
        item->setSizeHint(QSize(parent->width() - 5, WIDTH_ITEM));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(5, 2, 2, 2);
    hlayout->setSpacing(WIDTH_SPACING);

    m_label = new MyLabel;
    m_label->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_m_label");
    // m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_label->setIsOriginal(true);

    m_buttonDelete = new QPushButton;
    m_buttonDelete->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_m_buttonDelete");
    m_buttonDelete->setFlat(true);
    m_buttonDelete->setFixedSize(20, 20);
    m_buttonDelete->setIcon(ImageUtil::loadTheme("window-close-symbolic", ":/symbos/window-close-symbolic", "white", 20));
    m_buttonDelete->setVisible(false);

    hlayout->addWidget(m_label);
    hlayout->addWidget(m_buttonDelete);
    // hlayout->setAlignment(Qt::AlignRight);

    connect(this, &MyItemWidget::setScrollWidth, this, [=](int width) {
        if (parent && this->m_item)
            item->setSizeHint(QSize(width - 5 - this->m_scrollWidth, WIDTH_ITEM));
        int labelWidth = this->m_label->width();
        this->m_extWidth = 0;
        // 隐藏滚动条时会增加右侧边距
        if (this->m_scrollWidth > 0 && width == 0)
            this->m_extWidth = -1 * MERGE_IN;
        // 显示滚动条时去掉右侧边距
        else if (this->m_scrollWidth == 0 && width > 0)
            this->m_extWidth = MERGE_IN;
        this->m_label->setFixedWidth(labelWidth + (this->m_scrollWidth - width) + this->m_extWidth);

        this->m_label->setElidedText(this->m_text, Qt::ElideLeft);
        // this->m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        this->m_scrollWidth = width;
    });

    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::fontChanged, this, [=](int fontSize) {
        QFont font = this->m_label->font();
        font.setPointSize(fontSize);
        this->m_label->setFont(font);
        // 字体家族、大小变化需重绘，并且字体大小变化也可能会造成显示不全问题
        this->m_label->setElidedText(this->m_text, Qt::ElideLeft);
    });

    connect(this, &MyItemWidget::selected, this, [=](bool checked) {
        // int labelWidth = this->m_label->width();
        int widgetWidth = this->width();
        if (checked) {
            this->m_buttonDelete->setVisible(true);
            // this->m_label->setFixedWidth(labelWidth - WIDTH_SPACING - m_buttonDelete->width());
            this->m_label->setFixedWidth(widgetWidth - WIDTH_SPACING - m_buttonDelete->width() - MERGE_IN);
            this->m_label->setElidedText(this->m_text, Qt::ElideLeft);
            // this->m_label->setFontColor(Qt::white);
            // this->m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            this->m_label->setStyleSheet("QLabel {color:black}"
                                         "QToolTip {color:palette(windowText)}");
            this->m_checked = true;
        } else {
            this->m_buttonDelete->setVisible(false);
            // this->m_label->setFixedWidth(labelWidth + WIDTH_SPACING + m_buttonDelete->width());
            this->m_label->setFixedWidth(widgetWidth - MERGE_IN);
            this->m_label->setElidedText(this->m_text, Qt::ElideLeft);
            // this->m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            this->m_label->setStyleSheet("QLabel {color:palette(windowText)}"
                                         "QToolTip {color:palette(windowText)}");
            this->m_checked = false;
        }
    });

    connect(this, &MyItemWidget::resetItemWidth, this, [=](int width) {
        if (parent && item) {
            QListWidget *parentWidget = qobject_cast<QListWidget *>(parent);
            QScrollBar *vertScroll = parentWidget->verticalScrollBar();
            int scrollWidth = 0;
            if (vertScroll->isVisible()) {
                scrollWidth = vertScroll->size().width();
            }
            this->m_scrollWidth = scrollWidth;
            if (vertScroll->isVisible())
                item->setSizeHint(QSize(width - MERGE_IN - this->m_scrollWidth, WIDTH_ITEM));
            else
                item->setSizeHint(QSize(width - 5*2 - MERGE_IN - MERGE_IN, WIDTH_ITEM));
        }

        if (this->m_buttonDelete->isVisible()) {
            this->m_label->setFixedWidth(width - WIDTH_SPACING - m_buttonDelete->width() - MERGE_IN - MERGE_IN - this->m_scrollWidth);
            this->m_label->setElidedText(this->m_text, Qt::ElideLeft);
            // this->m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        } else {
            this->m_label->setFixedWidth(width - 5*2 - MERGE_IN - MERGE_IN - this->m_scrollWidth);
            this->m_label->setElidedText(this->m_text, Qt::ElideLeft);
            // this->m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        }
    });

    connect(m_buttonDelete, &QPushButton::clicked, this, [=]() {
        this->m_item = nullptr;
        emit deleteItem();
    });

    this->setLayout(hlayout);
}

void MyItemWidget::setText(const QString& text)
{
    m_text = text;
    // m_label->setFixedWidth(this->width() - 7 - this->m_scrollWidth);
    m_label->setElidedText(m_text, Qt::ElideLeft);
    m_label->setToolTip(m_text);
}

MyItemWidget::~MyItemWidget()
{
}

BackupListWidget::BackupListWidget(QWidget *parent /*= nullptr*/, QHBoxLayout *parentLayout /*= nullptr*/) :
    QListWidget(parent),
    m_parentLayout(parentLayout)
{
    setSortingEnabled(false);
    setAcceptDrops(true);
    setAlternatingRowColors(true);
    // setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 设置为无边框，默认时为StyledPanel
    setFrameShape(QListWidget::NoFrame);
    if (parentLayout != nullptr) {
        parentLayout->setContentsMargins(MERGE_IN, MERGE_IN, MERGE_IN, MERGE_IN);
        setGridSize(QSize(this->width(), 40 + MERGE_IN / 2));
    } else
        setGridSize(QSize(this->width(), 40));

    // 列表为空时，展示一个“+”号图标和拖拽文件提示
    m_plusLogo = new PixmapLabel;
    m_plusLogo->setObjectName("yhkylin-backup-tools_kybackup_PixmapLabel_m_plusLogo");
    m_plusLogo->setThemeIconSchema("list-add-symbolic", ":/symbos/list-add-symbolic", QSize(16, 16));
    m_plusLogo->setEnabled(false);
    // 文件拖放区域
    m_plusText = new QLabel;
    m_plusText->setObjectName("yhkylin-backup-tools_kybackup_QLabel_m_plusText");
    m_plusText->setText(tr("File drag and drop area"));
    m_plusText->setEnabled(false);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(m_plusLogo);
    hlayout->addWidget(m_plusText);
    hlayout->addStretch();

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addStretch();
    vlayout->addLayout(hlayout);
    vlayout->addStretch();
    setLayout(vlayout);

    connect(this, &BackupListWidget::currentItemChanged, this, [=](QListWidgetItem *current, QListWidgetItem *previous) {
        /*
         * 从2303系统镜像看，QListWidget在第一次添加后-->选择这唯一的一行-->删除这一行-->再拖拽添加一行进来，这时会多发送一个currentItemChanged信号，currentItem变为第一行；
         * 早期镜像无此问题，应该是主题改造Qt控件引发的错误，造成应用使用currentItemChanged信号处理逻辑错误
         * 在此尝试在应用程序层面修改此缺陷，注释下面针对current的操作，改为使用itemClicked信号
         */
        Q_UNUSED(current)
        if (current) {
            MyItemWidget *widget = qobject_cast<MyItemWidget *>(this->itemWidget(current));
            qDebug() << "BackupListWidget::currentItemChanged" << widget->text();
            if (widget) {
                if (-1 == this->currentRow() || !m_hadSelected)
                    emit widget->selected(false);
                else {
                    m_hadSelected = true;
                    emit widget->selected(true);
                }
            }
        }

        if (previous) {
            MyItemWidget *widget = qobject_cast<MyItemWidget *>(this->itemWidget(previous));
            if (widget) {
                emit widget->selected(false);
            }
        }
    });

    connect(this, &BackupListWidget::itemClicked, this, [=](QListWidgetItem *current) {
        if (current) {
            MyItemWidget *widget = qobject_cast<MyItemWidget *>(this->itemWidget(current));
            if (widget) {
                qDebug() << "BackupListWidget::itemClicked" << widget->text();
                if (!m_hadSelected)
                    emit widget->selected(true);
                m_hadSelected = true;
            }
        }
    });

    // 大小改变
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::widthChanged, this, [=](){
        emit this->resetItemWidth(this->width());
    });
}

BackupListWidget::~BackupListWidget()
{}

bool BackupListWidget::contains(const QString& text)
{
    // 1、针对使用addItem等的正规使用场景（展示内容在原生item上）
    if (findItems(text, Qt::MatchCaseSensitive).size() > 0)
        return true;

    // 2、针对使用setItemWidget添加项（展示内容在widget上）的特殊使用场景
    return m_List.contains(text);
}

bool BackupListWidget::appendItem(const QString &text)
{
    if (!checkPathLimit(text))
        return false;

    if (0 == this->count())
        m_hadSelected = false;

    QListWidgetItem *item = new QListWidgetItem(this, m_type);
    MyItemWidget *widget = new MyItemWidget(this, item);
    widget->setObjectName("yhkylin-backup-tools_kybackup_MyItemWidget_widget");
    this->setItemWidget(item, widget);
    // 必须将scrollToBottom等这种滚动条操作放到判断滚动条是否显示isVisible之前
    this->scrollToBottom();

    connect(this, &BackupListWidget::setScrollWidth, widget, &MyItemWidget::setScrollWidth);
    // 滚动条是否展示
    QScrollBar *vertScroll = verticalScrollBar();
    int scrollWidth = 0;
    if (vertScroll->isVisible()) {
        if (m_parentLayout != nullptr)
            m_parentLayout->setContentsMargins(MERGE_IN, MERGE_IN, 0, MERGE_IN);
        scrollWidth = vertScroll->size().width();
        emit this->setScrollWidth(scrollWidth);
    }

    widget->setText(text);
    m_List << text;

    connect(widget, &MyItemWidget::deleteItem, this, [=]() {
        this->m_List.removeOne(widget->text());
        this->removeItemWidget(item);
        this->takeItem(this->row(item));
        delete item;
        this->scrollToBottom();
        if (this->count() == 0) {
            this->m_plusLogo->setVisible(true);
            this->m_plusText->setVisible(true);
            this->clear();
            this->setCurrentRow(-1);
            m_hadSelected = false;
            emit this->deleteEmpty();
        } else {
            QScrollBar *vertScrollBar = verticalScrollBar();
            if (!vertScrollBar->isVisible()) {
                if (m_parentLayout != nullptr)
                    m_parentLayout->setContentsMargins(MERGE_IN, MERGE_IN, MERGE_IN, MERGE_IN);
                emit this->setScrollWidth(0);
            }
        }
    });

    connect(this, &BackupListWidget::resetItemWidth, widget, &MyItemWidget::resetItemWidth);

    m_plusLogo->setVisible(false);
    m_plusText->setVisible(false);
    emit this->addedItem();

    return true;
}

void BackupListWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        for (QUrl url : event->mimeData()->urls()) {
            QString file = url.toString();
            if (file.startsWith("file://")) {
                file.replace("file://", "");
                appendItem(url.path());
            }
        }
    }
}

bool BackupListWidget::checkPathLimit(const QString &path)
{
    // 防命令注入
    // 1、形如：mkdir '`id&>id_bak_test.txt`'中的文件夹名称
    // 2、形如：$()的文件夹名称
    // 3、形如：${}的文件夹名称
    // 4、包含[;、&、|]等可以包含并执行系统命令或用于连续执行系统命令的符号
    if (    path.contains(QRegularExpression(".*`.*`.*"))
         || path.contains(QRegularExpression(".*\\$\\(.*\\).*"))
         || path.contains(QRegularExpression(".*\\$\\{.*\\}.*"))
         || path.contains(QRegularExpression("[;&|]+"))) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"), QObject::tr("Path can not include symbols that such as : ``,$(),${},;,&,|,etc."), QObject::tr("OK"));
        return false;
    }

    // 1、列表中是否已经存在
    if (contains(path)) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(GlobelBackupInfo::instance().getMainWidget(), QObject::tr("Warning"),
                                              QObject::tr("Path already exists : ") + path,
                                              QObject::tr("OK"));
        return false;
    }

    // 2、路径是否存在
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(GlobelBackupInfo::instance().getMainWidget(), QObject::tr("Warning"),
                                              QObject::tr("The file or directory does not exist : ") + path,
                                              QObject::tr("OK"));
        return false;
    }

    // 3、是否是限定路径及其子路径
    bool blimit = false;
    QString dirCanBeSelected;
    QString abstractPath = fileInfo.absoluteFilePath();
    for (const QString &item : m_pathLimit) {
        if (abstractPath.startsWith(item)) {
            blimit = true;
            break;
        }

        if (dirCanBeSelected.isEmpty())
            dirCanBeSelected = item;
        else {
            dirCanBeSelected += ",";
            dirCanBeSelected += item;
        }
    }
    if (m_pathLimit.size() > 0 && !blimit) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(GlobelBackupInfo::instance().getMainWidget(), QObject::tr("Warning"),
                                              QObject::tr("Only data that exists in the follow directorys can be selected: %1.\n Path:%2 is not in them.").arg(dirCanBeSelected).arg(path),
                                              QObject::tr("OK"));
        return false;
    }

    return true;
}


