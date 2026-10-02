#include "shredfailinfodialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFileInfo>
#include <QDebug>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <QUrl>
#include <QDir>
#include <QToolTip>
ShredFailInfoDialog::ShredFailInfoDialog(QWidget *parent) : KDialog(parent)
{
    this->setFixedSize(800, 500);
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    // m_divider = new Divider(QSize(759, 367), false, this);
    m_frame = new QFrame(this);
    m_frame->setFixedSize(759, 367);
    m_frame->setAcceptDrops(true);

    QPushButton *cancelBtn = new QPushButton(this);
    cancelBtn->setText(tr("cancel"));

    m_failTable = new QTreeWidget(m_frame);
    m_failTable->setFixedSize(QSize(m_frame->width() - 7, m_frame->height() - 7));
    m_failTable->move(m_frame->width() / 2 - m_failTable->width() / 2,
                      m_frame->height() / 2 - m_failTable->height() / 2);
    // m_failTable = new QTreeWidget(m_divider);
    // m_failTable->setFixedSize(QSize(m_divider->width() - 7, m_divider->height() - 7));
    // m_failTable->move(m_divider->width() / 2 - m_failTable->width() / 2,
    //                   m_divider->height() / 2 - m_failTable->height() / 2);
    initFailTable();

    QHBoxLayout *btnlayout = new QHBoxLayout;
    btnlayout->setContentsMargins(0, 0, 0, 0);
    btnlayout->addStretch();
    btnlayout->addWidget(cancelBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(24, 16, 24, 16);
    mainLayout->setSpacing(0);
    // mainLayout->addWidget(m_divider);
    mainLayout->addWidget(m_frame);
    mainLayout->addSpacing(32);
    mainLayout->addLayout(btnlayout);
    mainWidget()->setLayout(mainLayout);

    connect(m_failTable, &QTreeWidget::itemEntered, this, &ShredFailInfoDialog::setTextToolTips);
    connect(cancelBtn, &QPushButton::clicked, this, &ShredFailInfoDialog::close);
}
void ShredFailInfoDialog::setTextToolTips(QTreeWidgetItem *item, int indexCol)
{
    QString tempText = item->text(indexCol);

    QFontMetrics fontWidth = QFontMetrics(tempText);

    if (fontWidth.width(tempText) > m_failTable->columnWidth(indexCol) - 10) {
        QToolTip ::showText(QCursor::pos(), tempText);
    } else {
        QToolTip ::showText(QCursor::pos(), "");
    }
}
void ShredFailInfoDialog::insertFailContent(QStringList failList)
{
    if (failList.length() < 0) {
        qDebug() << "失败列表为空";
        return;
    }
    for (int i = 0; i < failList.length(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        QFileInfo fileInfo(failList.at(i));
        if (fileInfo.exists()) {
            refreshInfoContents(failList.at(i));
            item->setIcon(0, QIcon::fromTheme(m_iconName));
        } else {
            item->setIcon(0, QIcon::fromTheme("text-x-generic"));
        }
        item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
        item->setText(0, fileInfo.fileName());
        item->setText(1, fileInfo.absolutePath());
        item->setSizeHint(0, QSize(m_failTable->columnWidth(0), 36));
        m_failTable->addTopLevelItem(item);
        //    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        //        item->setCheckState(0, Qt::Checked); //也可以是其他状态
        item->setWhatsThis(0, fileInfo.absoluteFilePath());
    }
}

void ShredFailInfoDialog::initFailTable()
{
    m_failTable->setFrameStyle(QFrame::NoFrame);
    m_failTable->setColumnCount(2);
    m_failTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // 将表格变为禁止编辑,取消焦点
    m_failTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_failTable->setFocusPolicy(Qt::NoFocus);
    // 设置表格不可选中
    m_failTable->setSelectionMode(QAbstractItemView::NoSelection);
    // 设置水平方向的表头标签与垂直方向上的表头标签，注意必须在初始化行列之后进行，否则，没有效果
    QList<QString> header = {tr("File Name"), tr("File Path")};
    m_failTable->setHeaderLabels(header);
    m_failTable->header()->setFixedHeight(36);

    //设置每列最小宽度
    m_failTable->header()->setMinimumSectionSize(100);
    //设置交替背景色
    m_failTable->setAlternatingRowColors(true);
    int column_width = 364;
    m_failTable->setColumnWidth(0, column_width);
    m_failTable->setUniformRowHeights(true);

    m_failTable->setMouseTracking(true);
    m_failTable->setRootIsDecorated(false);
}
bool ShredFailInfoDialog::refreshInfoContents(QString path)
{
    // GFIle
    QUrl url = QUrl::fromUserInput(path, QDir::currentPath(), QUrl::AssumeLocalFile);
    GError *error;
    GFile *file = g_file_new_for_path(url.path().toLocal8Bit());
    GFileInfo *file_info = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, &error);
    //判空-返回false
    if (!file_info) {
        qDebug() << "file path is null!";
        return false;
    }

    //获取文件类型
    const char *content_type = g_file_info_get_content_type(file_info);

    //普通文件
    GIcon *icon = g_file_info_get_icon(file_info);
    if (G_IS_THEMED_ICON(icon)) {
        const gchar *const *icon_names = g_themed_icon_get_names(G_THEMED_ICON(icon));
        if (icon_names) {
            auto p = icon_names;
            while (*p) {
                QIcon icon = QIcon::fromTheme(*p);
                if (!icon.isNull()) {
                    m_iconName = QString(*p);
                    break;
                } else {
                    p++;
                }
            }
        }
    }

    // desktop
    if (m_iconName == "application-x-desktop" || content_type == "application/x-desktop") {
        auto _desktop_file = g_desktop_app_info_new_from_filename(url.path().toUtf8().constData());
        if (_desktop_file) {
            m_iconName = g_desktop_app_info_get_string(_desktop_file, "Icon");
        }
    }
    return true;
}
