#include "pictureviewpage.h"
#include <QDebug>

PictureViewPage::PictureViewPage(QWidget *parent)
    : QWidget(parent)
{
    grid = new QGridLayout;
    deleteBtn = new QPushButton;
    deleteBtn->setAccessibleName("kylin-camera_QPushButton_deleteBtn");
    text = new QLabel(this);
    text->setAccessibleName("kylin-camera_QLabel_text");
    listWidget = new QListWidget(this);
    listWidget->setAccessibleName("kylin-camera_QListWidget_listWidget");
    vlayout = new QVBoxLayout;
    hlayout = new QHBoxLayout;
    btnlayout = new QHBoxLayout;
    title = new QWidget;
    title->setAccessibleName("kylin-camera_pictureviewpage_QWidget_title");
    btnpage = new QWidget;
    btnpage->setAccessibleName("kylin-camera_pictureviewpage_QWidget_btnpage");
    
    text->setText(tr("Album"));
    text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
#ifdef __V10__
    text->setStyleSheet("color:#FFFFFF;font-size:16px;font-family:SourceHanSansCN-Regular;");
    deleteBtn->setIcon(QIcon(":/image/delete.svg"));
#else
    deleteBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    deleteBtn->setProperty("useIconHighlightEffect", 0x8);
#endif
    deleteBtn->setFixedSize(42,32);
    deleteBtn->setFlat(true);
    hlayout->setContentsMargins(QMargins(0,6,0,0));
    hlayout->addStretch(10);
    hlayout->addWidget(text);
    hlayout->addStretch(10);

    btnlayout->setContentsMargins(QMargins(0,0,6,6));
    btnlayout->addStretch(50);
    btnlayout->addWidget(deleteBtn);
    btnpage->setLayout(btnlayout);
    btnpage->setFixedHeight(40);

    title->setLayout(hlayout);


    deleteBtn->setFlat(true);
    deleteBtn->setFocusPolicy(Qt::NoFocus);

    vlayout->addWidget(title);
    vlayout->addWidget(listWidget);
    vlayout->addWidget(btnpage);
    vlayout->setMargin(0);
    title->setFixedHeight(30);
    this->setLayout(vlayout);
    installEventFilter(this);
    initListWidget();
}

void PictureViewPage::initListWidget()
{
    listWidget->setIconSize(QSize(120,90));
    listWidget->setResizeMode(QListView::Adjust);
    listWidget->setViewMode(QListView::IconMode);
    listWidget->setMovement(QListView::Static);
    listWidget->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
#ifdef __V10__
    listWidget->setStyleSheet("background-color:#000000");
#endif
    listWidget->horizontalScrollBar()->setContextMenuPolicy(Qt::ActionsContextMenu);
    listWidget->setContextMenuPolicy(Qt::NoContextMenu);
    listWidget->setFrameShape(QListWidget::NoFrame);
    listWidget->setSpacing(0);
    listWidget->horizontalScrollBar()->setDisabled(true);
    imageLabel = new QLabel;
    imageLabel->setAccessibleName("kylin-camera_QLabel_imageLabel");
    imageLabel->setWindowTitle("TITLE");
}



void PictureViewPage::initListView(){
    listview->setIconSize(QSize(120,90));
    listview->setViewMode(QListView::IconMode);
    listview->setResizeMode(QListView::Adjust);
    listview->setMovement(QListView::Static);
}

