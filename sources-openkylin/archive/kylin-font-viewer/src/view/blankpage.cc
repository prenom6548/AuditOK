#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>
#include <QPainter>

#include "blankpage.h"
#include "globalsizedata.h"

/* 适配组内SDK */
#include <gsettingmonitor.h>
#include <usermanual.h>
#include "windowmanage.hpp"
#include <QPainterPath>

const QSize IMAGESIZE = QSize(96, 96);

BlankPage::BlankPage(QWidget *parent)
    : QWidget(parent)
{
    setWidgetUi();
    initLayout();
    initGsetting();
    changeTheme();
}

BlankPage::~BlankPage()
{
}

void BlankPage::setWidgetUi()
{
    m_blankIconLabel = new QLabel(this);
    m_blankIconLabel->setAccessibleName("kylin-font-viewer_BlankPage_QLabel_blankIconLabel");
    m_blankIconLabel->setAccessibleDescription("this is the blank icon label for the blank page");

    m_blankTextLabel = new QLabel(this);
    m_blankTextLabel->setAccessibleName("kylin-font-viewer_BlankPage_QLabel_blankTextLabel");
    m_blankTextLabel->setAccessibleDescription("this is the blank text label for the blank page");

    m_blankTextLabel->setText(tr("No Font"));
    QString labGrayStyle = "color:#8C8C8C;";
    m_blankTextLabel->setStyleSheet(labGrayStyle);
    m_blankTextLabel->setAlignment(Qt::AlignHCenter);

    QFont font14;
    font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
    m_blankTextLabel->setFont(font14);

    m_vLayout = new QVBoxLayout();
    m_hLayout = new QHBoxLayout();
    return;
}

void BlankPage::initLayout()
{
    this->setMinimumSize(588, 440);
    this->setBackgroundRole(QPalette::Base);

    m_hLayout->addStretch();
    m_hLayout->addWidget(m_blankIconLabel);
    m_hLayout->addStretch();

    m_vLayout->addStretch();
    m_vLayout->addLayout(m_hLayout);
    m_vLayout->setSpacing(8);
    m_vLayout->addWidget(m_blankTextLabel);
    m_vLayout->addStretch();
    m_vLayout->setSpacing(0);
    m_vLayout->setMargin(0);

    this->setLayout(m_vLayout);


    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontChange, [=](){
        QFont font14;
        font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
        m_blankTextLabel->setFont(font14);
    });
    return;
}

void BlankPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    QPainterPath rectPath;
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    return;
}

void BlankPage::initGsetting()
{
    // 主题颜色
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        GlobalSizeData::getInstance()->getSystemTheme();
        changeTheme();
    });
}

void BlankPage::changeTheme()
{
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
        QIcon blankIcon = QIcon(":/data/image/light.svg");
        m_blankIconLabel->setPixmap(blankIcon.pixmap(blankIcon.actualSize(IMAGESIZE)));
    } else {
        QIcon blankIcon = QIcon(":/data/image/dark.svg");
        m_blankIconLabel->setPixmap(blankIcon.pixmap(blankIcon.actualSize(IMAGESIZE)));
    }
}

void BlankPage::changeSearchText()
{
    m_blankTextLabel->setText(tr("No Search Results"));
}

void BlankPage::changeBlankText()
{
    m_blankTextLabel->setText(tr("No Font"));
}
