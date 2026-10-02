#include "nodevicepage.h"
#include "defaultgetset.h"
NoDevicePage::NoDevicePage(QWidget *parent) :
    QWidget(parent)
{
    gsettingItem = new DefaultGetSet;
    gsettingItem->setAccessibleName("kylin-camera_nodevicepage_DefaultGetSet_gsettingItem");
    iconContainer = new QWidget;
    iconContainer->setAccessibleName("kylin-camera_nodevicepage_QWidget_iconContainer");
    iconLabel = new QLabel(this);
    iconLabel->setAccessibleName("kylin-camera_QLable_iconLabel");
    iconHLayout = new QHBoxLayout;
    iconHLayout->addStretch(0);
    iconHLayout->addWidget(iconLabel);
    iconHLayout->addStretch(0);
    iconHLayout->setMargin(0);
    iconContainer->setLayout(iconHLayout);

    titleContainer = new QWidget;
    titleContainer->setAccessibleName("kylin-camera_nodevicepage_QWidget_titleContainer");
    titleLabel = new QLabel(this);
    titleLabel->setAccessibleName("kylin-camera_QLabel_titleLabel");
    titleHLayout = new QHBoxLayout;
    titleHLayout->addStretch(0);
    titleHLayout->addWidget(titleLabel);
    titleHLayout->addStretch(0);
    titleHLayout->setMargin(0);
    titleContainer->setLayout(titleHLayout);

    detailContainer = new QWidget;
    detailContainer->setAccessibleName("kylin-camera_nodevicepage_QWidget_detailContainer");
    detailLabel = new QLabel(this);
    detailLabel->setAccessibleName("kylin-camera_QLabel_detailLabel");
    detailHLayout = new QHBoxLayout;
    detailHLayout->addStretch(0);
    detailHLayout->addWidget(detailLabel);
    detailHLayout->addStretch(0);
    detailHLayout->setMargin(0);
    detailContainer->setLayout(detailHLayout);

    layout = new QVBoxLayout;
    layout->addStretch(10);
    layout->addWidget(iconContainer);
    layout->addWidget(titleContainer);
    layout->addWidget(detailContainer);
    layout->addStretch(10);
    this->setLayout(layout);
#ifdef __V10__
    iconLabel->setPixmap(QPixmap(":/image/camera-disconnected.png"));
#else

#endif
    iconLabel->setProperty("useIconHighlightEffect", 0x8);
    titleLabel->setText(tr("No device were found"));
    detailLabel->setText(tr("Please connect the camera first and app will continuously retrieve avaliable devices for you"));
    QString nowThemeStyle = gsettingItem->getStyleName();
    this->setGeometry(0,0,300,100);
    QPalette pal(this->palette());
    if (nowThemeStyle == "ukui-dark" || nowThemeStyle == "ukui-black"){
        //设置背景色黑色
        pal.setColor(QPalette::Background,Qt::black);
        this->setPalette(pal);
        iconLabel->setPixmap(QPixmap(":/image/illu-camera-dark.png"));
    }
    else{
        //设置背景色灰色
        pal.setColor(QPalette::Background,Qt::white);
        this->setPalette(pal);
        iconLabel->setPixmap(QPixmap(":/image/illu-camera-light.png"));
    }
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    connect(gsettingItem,&DefaultGetSet::sigThemeChange, this,&NoDevicePage::dealSystemGsettingChange);
}

void NoDevicePage::dealSystemGsettingChange(){
    QString themeNow = gsettingItem->getStyleName();
    QPalette pal(this->palette());
    if(themeNow == "ukui-dark" || themeNow == "ukui-black"){
        pal.setColor(QPalette::Background,Qt::black);
        this->setPalette(pal);
        iconLabel->setPixmap(QPixmap(":/image/illu-camera-dark"));
    }
    else{
        pal.setColor(QPalette::Background,Qt::white);
        this->setPalette(pal);
        iconLabel->setPixmap(QPixmap(":/image/illu-camera-light"));
    }
}
void NoDevicePage::setTips(QString str1,QString str2){
    titleLabel->setText(str1);
    detailLabel->setText(str2);
}
NoDevicePage::~NoDevicePage(){
    delete gsettingItem;
    delete iconContainer;
    delete titleContainer;
    delete detailContainer;
}
