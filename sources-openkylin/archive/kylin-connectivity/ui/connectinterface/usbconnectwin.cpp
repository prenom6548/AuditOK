#include "usbconnectwin.h"

#include <QVBoxLayout>
#include <QScrollBar>
#include <QDebug>

int UsbConnectWin::ROLLING_WIN_W = 772;
int UsbConnectWin::ROLLING_WIN_H = 400;

UsbConnectWin::UsbConnectWin(QWidget *parent) : QWidget(parent)
{
    m_backImageBtn = new QPushButton(this);
    connect(m_backImageBtn, &QPushButton::clicked, this, &UsbConnectWin::onClicked);
    m_backImageBtn->setObjectName("back");
    m_backImageBtn->setIcon(QIcon(":/connectinterface/backIcon.png"));
    m_backImageBtn->setFixedSize(14, 27);
    m_backImageBtn->setStyleSheet("QPushButton{border:none;background-color:transparent;border-radius:4px;}");

    m_nextImageBtn = new QPushButton(this);
    connect(m_nextImageBtn, &QPushButton::clicked, this, &UsbConnectWin::onClicked);
    m_nextImageBtn->setObjectName("next");
    m_nextImageBtn->setIcon(QIcon(":/connectinterface/nextIcon.png"));
    m_nextImageBtn->setFixedSize(14, 27);
    m_nextImageBtn->setStyleSheet("QPushButton{border:none;background-color:transparent;border-radius:4px;}");

    m_imageWidget = new QWidget(this);
    m_imageLayout = new QHBoxLayout;
    m_imageLayout->setSpacing(0);
    m_imageLayout->setMargin(0);
    m_imageWidget->setLayout(m_imageLayout);
    m_imagePlayWidget = new QScrollArea(this);
    m_imagePlayWidget->setProperty("barPos", 0);
    m_imagePlayWidget->setFixedSize(ROLLING_WIN_W, ROLLING_WIN_H);
    m_imagePlayWidget->setFrameShape(QFrame::NoFrame);
    m_imagePlayWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //设置水平滚动条
    m_imagePlayWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);   //设置垂直滚动条
    m_imagePlayWidget->setWindowFlags(Qt::FramelessWindowHint);
    m_imagePlayWidget->setWidget(m_imageWidget);

    m_backImageLab = new QLabel(this);
    QVBoxLayout *vLayout1 = new QVBoxLayout;
    vLayout1->addStretch();
    vLayout1->addWidget(m_backImageBtn);
    vLayout1->addStretch();
    vLayout1->setSpacing(0);
    vLayout1->setMargin(0);
    m_backImageLab->setLayout(vLayout1);

    m_nextImageLab = new QLabel(this);
    QVBoxLayout *vLayout2 = new QVBoxLayout;
    vLayout2->addStretch();
    vLayout2->addWidget(m_nextImageBtn);
    vLayout2->addStretch();
    vLayout2->setSpacing(0);
    vLayout2->setMargin(0);
    m_nextImageLab->setLayout(vLayout2);

    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addStretch();
    hLayout1->addWidget(m_backImageLab);
    hLayout1->addSpacing(30);
    hLayout1->addWidget(m_imagePlayWidget);
    hLayout1->addSpacing(30);
    hLayout1->addWidget(m_nextImageLab);
    hLayout1->addStretch();
    hLayout1->setSpacing(0);
    hLayout1->setMargin(0);

    m_connectBtn = new QPushButton(this);
    connect(m_connectBtn, &QPushButton::clicked, this, &UsbConnectWin::sigUSBconnectBtnClicked);
    m_connectBtn->setText(tr("Connect now"));
    m_connectBtn->setProperty("isImportant", true);

    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->addStretch();
    hLayout2->addWidget(m_connectBtn);
    hLayout2->addStretch();
    hLayout2->setSpacing(0);
    hLayout2->setMargin(0);

    QVBoxLayout *vLayout3 = new QVBoxLayout;
    vLayout3->addSpacing(23);
    vLayout3->addLayout(hLayout1);
    vLayout3->addSpacing(24);
    vLayout3->addLayout(hLayout2);
    vLayout3->addSpacing(13);
    vLayout3->setSpacing(0);
    vLayout3->setMargin(0);
    setLayout(vLayout3);

    // 动画切换类;
    m_opacityAnimation = new QPropertyAnimation(m_imagePlayWidget, "barPos");
    // 这里要设置的动画时间
    m_opacityAnimation->setDuration(400);

    // 动画变化
    connect(m_opacityAnimation, SIGNAL(valueChanged(const QVariant &)), this,
            SLOT(slotImageIndexChange(const QVariant &)));
}

UsbConnectWin::~UsbConnectWin()
{
    if (m_opacityAnimation != nullptr) {
        delete m_opacityAnimation;
        m_opacityAnimation = nullptr;
    }
}

void UsbConnectWin::setImageList(const QStringList &imageFileNameList)
{
    m_imageFileNameList = imageFileNameList;
    if (m_imageFileNameList.isEmpty()) {
        // 没有图片默认显示
        m_imageWidget->setFixedSize(ROLLING_WIN_W, ROLLING_WIN_H);
        QLabel *defaultLab = new QLabel(m_imageWidget);
        defaultLab->setText(tr("No tutorial"));

        m_imageLayout->addStretch();
        m_imageLayout->addWidget(defaultLab);
        m_imageLayout->addStretch();

        // 隐藏左右按钮
        m_backImageBtn->hide();
        m_nextImageBtn->hide();
    } else if (m_imageFileNameList.count() == 1) {
        // 设置放置图片的界面大小 w = 图片数量*滚动窗口大小
        m_imageWidget->setFixedSize(ROLLING_WIN_W, ROLLING_WIN_H);
        // 若是只有一张图片
        QLabel *oneLab = new QLabel(m_imageWidget);
        // 默认显示第一张图片
        QPixmap pixmap = QPixmap(m_imageFileNameList.first());
        // Lab自适应图片大小
        pixmap.scaled(oneLab->size(), Qt::KeepAspectRatio);
        oneLab->setScaledContents(true);
        oneLab->setPixmap(pixmap);

        m_imageLayout->addWidget(oneLab);
        m_imageWidget->setLayout(m_imageLayout);

        // 隐藏左右按钮
        m_backImageBtn->hide();
        m_nextImageBtn->hide();
    } else if (m_imageFileNameList.count() > 1) {
        // 设置放置图片的界面大小 w = 图片数量*滚动窗口大小
        m_imageWidget->setFixedSize(m_imageFileNameList.count() * ROLLING_WIN_W, ROLLING_WIN_H);
        // 多张图片;
        QHBoxLayout *hLayout = new QHBoxLayout;
        // 循环放置多张图片
        for (int i = 0; i < m_imageFileNameList.count(); i++) {
            QLabel *oneLab = new QLabel(m_imageWidget);
            QPixmap pixmap = QPixmap(m_imageFileNameList.value(i));
            pixmap.scaled(oneLab->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            oneLab->setScaledContents(true);
            oneLab->setPixmap(pixmap);
            m_imageLayout->addWidget(oneLab);
        }
        m_imageWidget->setLayout(m_imageLayout);
        // 显示第一张图片滑块处于初始位置
        m_currentDrawImageIndex = 0;
        slotImageIndexChange(0);

        m_backImageBtn->hide();
        m_nextImageBtn->show();
    }
}

void UsbConnectWin::clearImageList()
{
    m_imageFileNameList.clear();
    if (m_imageWidget != nullptr) {
        QLayoutItem *child;
        while ((child = m_imageLayout->takeAt(0)) != 0) {
            // setParent为NULL，防止删除之后界面不消失
            if (child->widget()) {
                child->widget()->setParent(nullptr);
            }
            delete child;
        }
    }
}

void UsbConnectWin::slotImageIndexChange(const QVariant &value)
{
    // 响应动画变化，移动滑块位置
    QScrollBar *bar = m_imagePlayWidget->horizontalScrollBar();
    bar->setSliderPosition(value.toInt());
}

void UsbConnectWin::onClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    QString str = btn->objectName();

    if ("back" == str) {
        // 设置ImageOpacity属性值的变化范围;
        m_opacityAnimation->setStartValue(m_currentDrawImageIndex);
        m_opacityAnimation->setEndValue(m_currentDrawImageIndex - ROLLING_WIN_W);
        // 设置当前滑块位置
        m_currentDrawImageIndex -= ROLLING_WIN_W;
    } else {
        // 设置ImageOpacity属性值的变化范围;
        m_opacityAnimation->setStartValue(m_currentDrawImageIndex);
        m_opacityAnimation->setEndValue(m_currentDrawImageIndex + ROLLING_WIN_W);
        // 设置当前滑块位置
        m_currentDrawImageIndex += ROLLING_WIN_W;
    }
    // 开始动画
    m_opacityAnimation->start();

    if (0 == m_currentDrawImageIndex) {
        // 第一张图片
        m_backImageBtn->hide();
        m_nextImageBtn->show();
    } else if (ROLLING_WIN_W == m_currentDrawImageIndex) {
        // 第一张图片
        m_backImageBtn->show();
        m_nextImageBtn->hide();
    } else {
        m_backImageBtn->show();
        m_backImageBtn->show();
    }
}

void UsbConnectWin::setTheme(PublicAttributes::Theme theme)
{
    switch (theme) {
    case PublicAttributes::Theme::Light: {
        m_imageWidget->setStyleSheet("QWidget{background: #F6F6F6;}");
        m_backImageBtn->setIcon(QIcon(":/connectinterface/backIcon.png"));
        m_nextImageBtn->setIcon(QIcon(":/connectinterface/nextIcon.png"));
    } break;
    case PublicAttributes::Theme::Dark: {
        m_imageWidget->setStyleSheet("QWidget{background: #232426;}");
        m_backImageBtn->setIcon(QIcon(":/connectinterface/backDarkIcon.png"));
        m_nextImageBtn->setIcon(QIcon(":/connectinterface/nextDarkIcon.png"));
    } break;
    }
}
