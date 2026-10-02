#include <QDebug>

#include "homepagewidget.h"
#include "../globaldata.h"

HomePageWidget::HomePageWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initStyle();
    initLayout();
    initConn();
}

HomePageWidget::~HomePageWidget()
{

}

HomePageWidget *HomePageWidget::getInstance()
{
    static HomePageWidget *instance = nullptr;
    if (nullptr == instance) {
        instance = new HomePageWidget();
    }
    return instance;
}

void HomePageWidget::initUI()
{
    m_labelAppIcon = new IconLabel(this);
    m_labelAppType = new TextLabel(this);
    m_labelHomeIcon = new IconLabel(this);
    m_labelHomeType = new TextLabel(this);
    m_labelSysIcon = new IconLabel(this);
    m_labelSysType = new TextLabel(this);
    m_labelInfo = new TextLabel(this);
    m_labelWelcome = new TextLabel(this);
    m_labelIconBig = new QLabel(this);
    m_btnHowUse = new KBorderlessButton(this);
    m_btnStart = new QPushButton(this);

    m_layoutF = new QFormLayout();
    m_layoutHBtnStart = new QHBoxLayout();
    m_layoutHBtnHowToUse = new QHBoxLayout();
    m_layoutHMain = new QHBoxLayout();
    m_layoutVInfo = new QVBoxLayout();
    m_layoutVMain = new QVBoxLayout();
    m_layoutHApp = new QHBoxLayout();
    m_layoutHHome = new QHBoxLayout();
    m_layoutHSys = new QHBoxLayout();

    GlobalData::getInstance();
    return;
}

void HomePageWidget::initStyle()
{
    this->setFixedSize(GlobalData::g_sizeWidget);
    // this->setIcon(GlobalData::g_appNameIconText); // 设置图标
    // this->setWidgetName(GlobalData::g_appNameText);
    this->setBackgroundRole(QPalette::Base);
    this->setAttribute(Qt::WA_TranslucentBackground, true);  //透明

    // iconBar()->setAutoFillBackground(true);
    // iconBar()->setAttribute(Qt::WA_TranslucentBackground, true);  //透明
    // iconBar()->setProperty("useSystemStyleBlur", true);
    // baseBar()->setAutoFillBackground(true);
    // baseBar()->setAttribute(Qt::WA_TranslucentBackground, true);  //透明
    // baseBar()->setProperty("useSystemStyleBlur", true);
    // windowButtonBar()->setAutoFillBackground(true);
    // windowButtonBar()->setAttribute(Qt::WA_TranslucentBackground, true);  //透明
    // windowButtonBar()->setProperty("useSystemStyleBlur", true);

    m_labelWelcome->SetText(GlobalData::g_textMainWidgetWelcome);
    m_labelWelcome->SetPixelSize(30, true);

    m_labelInfo->SetText(GlobalData::g_textMainWidgetInfo);
    m_labelInfo->SetPixelSize(16);

    m_labelAppType->SetText(GlobalData::g_textMigrationTypeApp);
    m_labelAppType->SetFontColor(QColor(140, 140, 140));
    m_labelAppIcon->SetIcon(GlobalData::g_iconMigrationTypeApp);
    m_labelAppIcon->setFixedSize(32, 32);

    m_labelHomeType->SetText(GlobalData::g_textMigrationTypeUser);
    m_labelHomeType->SetFontColor(QColor(140, 140, 140));
    m_labelHomeIcon->SetIcon(GlobalData::g_iconMigrationTypeHome);
    m_labelHomeIcon->setFixedSize(32, 32);

    m_labelSysType->SetText(GlobalData::g_textMigrationTypeSystem);
    m_labelSysType->SetFontColor(QColor(140, 140, 140));
    m_labelSysIcon->SetIcon(GlobalData::g_iconMigrationTypeSys);
    m_labelSysIcon->setFixedSize(32, 32);

    m_btnStart->setText(GlobalData::g_textStartBtn);
    GlobalData::getInstance()->setHighColorBtn(m_btnStart);

    m_btnHowUse->setText(GlobalData::g_textTipBtn);
    m_btnHowUse->setIcon(QIcon::fromTheme("help-app-symbolic"));

    QIcon blankIcon = QIcon(":/res/homePageIconLight.png");
    m_labelIconBig->setPixmap(blankIcon.pixmap(blankIcon.actualSize(QSize(334, 284))));


    if (QGSettings::isSchemaInstalled("org.ukui.control-center.personalise")) {
        m_pTransparency = new QGSettings("org.ukui.control-center.personalise");
        tranSparency = m_pTransparency->get("transparency").toDouble();
        connect(m_pTransparency, &QGSettings::changed, this, [=](QString value) {
            if (value == "transparency") {
                tranSparency = m_pTransparency->get("transparency").toDouble();
                adjustBackground();
            }
        });
        adjustBackground();
    }
}

void HomePageWidget::initLayout()
{
    m_layoutHApp->addWidget(m_labelAppIcon);
    m_layoutHApp->addSpacing(10);
    m_layoutHApp->addWidget(m_labelAppType);
    m_layoutHApp->addStretch();

    m_layoutHHome->addWidget(m_labelHomeIcon);
    m_layoutHHome->addSpacing(10);
    m_layoutHHome->addWidget(m_labelHomeType);
    m_layoutHHome->addStretch();

    m_layoutHSys->addWidget(m_labelSysIcon);
    m_layoutHSys->addSpacing(10);
    m_layoutHSys->addWidget(m_labelSysType);
    m_layoutHSys->addStretch();

    m_layoutF->setLayout(0,QFormLayout::LabelRole, m_layoutHApp);
    m_layoutF->setLayout(0, QFormLayout::FieldRole, m_layoutHHome);
    m_layoutF->setLayout(1, QFormLayout::LabelRole, m_layoutHSys);
    m_layoutF->setHorizontalSpacing(26);
    m_layoutF->setVerticalSpacing(12);

    m_layoutHBtnStart->addWidget(m_btnStart);
    m_layoutHBtnStart->addStretch();

    m_layoutHBtnHowToUse->addStretch();
    m_layoutHBtnHowToUse->addWidget(m_btnHowUse);
    m_layoutHBtnHowToUse->addStretch();

    m_layoutVInfo->addStretch();
    m_layoutVInfo->addWidget(m_labelWelcome);
    m_layoutVInfo->addWidget(m_labelInfo);
    m_layoutVInfo->addSpacing(12);
    m_layoutVInfo->addLayout(m_layoutF);
    m_layoutVInfo->addSpacing(32);
    m_layoutVInfo->addLayout(m_layoutHBtnStart);
    m_layoutVInfo->addStretch();

    m_layoutHMain->addSpacing(96);
    m_layoutHMain->addLayout(m_layoutVInfo);
    m_layoutHMain->addStretch();
    m_layoutHMain->addWidget(m_labelIconBig);
    m_layoutHMain->addSpacing(80);

    // m_layoutVMain->setContentsMargins(96, 100, 80, 24);
    // m_layoutVMain->addStretch();
    m_layoutVMain->addSpacing(100);
    m_layoutVMain->addLayout(m_layoutHMain);
    m_layoutVMain->addStretch();
    m_layoutVMain->addLayout(m_layoutHBtnHowToUse);
    m_layoutVMain->addSpacing(24);
    m_layoutVMain->setMargin(0);
    m_layoutVMain->setSpacing(0);

    // baseBar()->setLayout(m_layoutVMain);
    this->setLayout(m_layoutVMain);
}

void HomePageWidget::initConn()
{
    connect(m_btnStart, &QPushButton::clicked, this, &HomePageWidget::sinStatClick);
}

void HomePageWidget::setBackgroundColor(QWidget *w)
{

}

void HomePageWidget::adjustBackground()
{

    QColor sideColor = this->palette().color(QPalette::Base);
    sideColor.setAlphaF(tranSparency);
    QPalette sidePalette = this->palette();
    sidePalette.setColor(QPalette::Base, sideColor);

    // iconBar()->setAutoFillBackground(true);
    // iconBar()->setBackgroundRole(QPalette::Base);
    // iconBar()->setPalette(sidePalette);

    // baseBar()->setAutoFillBackground(true);
    // baseBar()->setBackgroundRole(QPalette::Base);
    // baseBar()->setPalette(sidePalette);

    // windowButtonBar()->setAutoFillBackground(true);
    // windowButtonBar()->setBackgroundRole(QPalette::Base);
    // windowButtonBar()->setPalette(sidePalette);
    // m_w->setAutoFillBackground(true);
    // m_w->setProperty("useSystemStyleBlur", true);
    // m_w->setBackgroundRole(QPalette::Window);
    // m_w->setPalette(sidePalette);
}
