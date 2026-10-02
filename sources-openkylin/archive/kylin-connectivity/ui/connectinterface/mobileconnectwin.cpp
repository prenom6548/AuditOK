#include "mobileconnectwin.h"

#include <QDebug>
#include <QComboBox>
#include "windowmanage.hpp"
#include "generatetools.h"
#include "highlight-effect.h"

const int TABBAR_W = 314;
const QString LIGHT = "light";
const QString DARK = "dark";
QStringList VVLIST;
QStringList HWLIST;
QStringList XMLIST;
QStringList SXLIST;
QStringList OPPOLIST;
QStringList OTHERLIST;

MobileConnectWin::MobileConnectWin(QWidget *parent) : QWidget(parent)
{
    m_tabBar = new KTabBar(KTabBarStyle::Sliding, this);
    m_tabBar->setFixedHeight(50);
    m_tabBar->setMinimumWidth(TABBAR_W);
    m_tabBar->addTab(tr("ScanCode"));
    m_tabBar->addTab(tr("USBConnect"));
    connect(m_tabBar, &KTabBar::tabBarClicked, this, &MobileConnectWin::slotTabBarClicked);

    m_mobileQRcode = new MobileQRcode();
    m_usbConnWin = new UsbConnectWin();
    connect(m_usbConnWin, &UsbConnectWin::sigUSBconnectBtnClicked, this, &MobileConnectWin::sigUSBconnectBtnClicked);
    m_stackedWin = new QStackedWidget(this);
    // 设置堆栈窗口
    m_stackedWin->addWidget(m_mobileQRcode);  // 索引：0
    m_stackedWin->addWidget(m_usbConnWin);    // 索引：1
    m_stackedWin->setCurrentIndex(TabBarIndex::MobileQRcodeWin);

    m_comboBoxWin = new QWidget(this);
    m_comboBoxWin->setFixedWidth(300);
    m_spaceLab = new QLabel(this);
    m_spaceLab->setFixedWidth(300);

    m_choiceDescLab = new QLabel(m_comboBoxWin);
    m_choiceDescLab->setText(tr("Your phone model: "));

    QLabel *phoneIconLab = new QLabel(m_comboBoxWin);
    phoneIconLab->setPixmap(QIcon(":/connectinterface/phoneicon.svg").pixmap(16, 16));

    QComboBox *modelBox = new QComboBox(m_comboBoxWin);
    modelBox->setFixedHeight(40);
    modelBox->setMinimumWidth(100);
    QStringList modelist;
    modelist << m_vvstr << m_hwstr << m_xmstr << m_oppostr;  // << m_sxstr << m_otherstr;
    m_currentFlage = m_vvstr;
    modelBox->addItems(modelist);
    connect(modelBox, &QComboBox::currentTextChanged, this, &MobileConnectWin::valueChanged);

    QHBoxLayout *comboBoLayout = new QHBoxLayout;
    comboBoLayout->setMargin(0);
    comboBoLayout->setSpacing(0);
    comboBoLayout->addStretch();
    comboBoLayout->addWidget(m_choiceDescLab);
    comboBoLayout->addSpacing(4);
    comboBoLayout->addWidget(phoneIconLab);
    comboBoLayout->addSpacing(4);
    comboBoLayout->addWidget(modelBox);
    comboBoLayout->addSpacing(8);
    m_comboBoxWin->setLayout(comboBoLayout);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    hLayout->addWidget(m_spaceLab);
    hLayout->addStretch();
    hLayout->addWidget(m_tabBar);
    hLayout->addStretch();
    hLayout->addWidget(m_comboBoxWin);
    m_comboBoxWin->hide();
    m_spaceLab->hide();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_stackedWin);

    this->setLayout(vLayout);
}

void MobileConnectWin::setInterFaceCodeInfo(QString codeInfo)
{
    if (codeInfo.isEmpty()) {
        m_mobileQRcode->setQRCode();
    } else {
        m_mobileQRcode->setQRCode(codeInfo);
    }
}

void MobileConnectWin::changeFontSize(double fontSize)
{
    QFont font;
    font.setPointSizeF(fontSize);
    QFontMetrics fm(font);
    QString str = tr("Your phone model: ");
    int width = m_choiceDescLab->width();
    QString text = fm.elidedText(str, Qt::ElideRight, width);
    m_choiceDescLab->setText(text);
}

void MobileConnectWin::setTheme(PublicAttributes::Theme theme)
{
    switch (theme) {
        case PublicAttributes::Theme::Light: {
            setUsbUI(LIGHT);
        } break;
        case PublicAttributes::Theme::Dark: {
            setUsbUI(DARK);
        } break;
    }
    m_usbConnWin->setTheme(theme);
    m_mobileQRcode->setTheme(theme);
}

void MobileConnectWin::setUsbUI(QString theme)
{
    VVLIST.clear();
    HWLIST.clear();
    XMLIST.clear();
    OPPOLIST.clear();
    SXLIST.clear();
    OTHERLIST.clear();
    QString locale = QLocale::system().name();
    if (locale == "en") {
        locale = "en";
    } else {
        locale = "zh";
    }
    VVLIST << ":/connectinterface/vivo/" + theme + locale + "1.png"
           << ":/connectinterface/vivo/" + theme + locale + "2.png";
    HWLIST << ":/connectinterface/hw/" + theme + locale + "1.png"
           << ":/connectinterface/hw/" + theme + locale + "2.png";
    XMLIST << ":/connectinterface/xm/" + theme + locale + "1.png"
           << ":/connectinterface/xm/" + theme + locale + "2.png";
    OPPOLIST << ":/connectinterface/oppo/" + theme + locale + "1.png"
             << ":/connectinterface/oppo/" + theme + locale + "2.png";
    valueChanged(m_currentFlage);
}

void MobileConnectWin::slotTabBarClicked(int index)
{
    m_stackedWin->setCurrentIndex(index);
    switch (index) {
        case TabBarIndex::MobileQRcodeWin:
            m_comboBoxWin->hide();
            m_spaceLab->hide();
            break;
        case TabBarIndex::UsbConnWin:
            m_comboBoxWin->show();
            m_spaceLab->show();
            break;
        default:
            break;
    }
}

void MobileConnectWin::valueChanged(QString text)
{
    m_currentFlage = text;
    m_usbConnWin->clearImageList();
    if (m_vvstr == text) {
        m_usbConnWin->setImageList(VVLIST);
    } else if (m_hwstr == text) {
        m_usbConnWin->setImageList(HWLIST);
    } else if (m_xmstr == text) {
        m_usbConnWin->setImageList(XMLIST);
    } else if (m_oppostr == text) {
        m_usbConnWin->setImageList(OPPOLIST);
    }
    // else if (m_sxstr == text) {
    // m_usbConnWin->setImageList(SXLIST);
    //} else if (m_otherstr == text) {
    // m_usbConnWin->setImageList(OTHERLIST);
    //}
}
