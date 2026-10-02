#include "selectionwidget.h"
#include "include/common.h"
#include "include/globalsignal.h"
#include <QScrollBar>

SelectionWidget::SelectionWidget(QWidget *parent) :
        QWidget(parent),
        m_driverInstallBtn(new CustomPushButton()),
        m_hardwareInfoBtn(new CustomPushButton()),
        m_driverUninstallBtn(new CustomPushButton()),
        m_btnGroup(new QButtonGroup()),
        m_stackWidget(new QStackedWidget()),
        m_driverInstallWidget(new DriverInstallWidget()),
        m_hardwareInfoWidget(new HardWareInfoWidget()),
        m_driverUninstallWidget(new DriverUninstallMainWidget()),
        m_driverInstallSrollArea(new QScrollArea()),
        m_driverUninstallSrollArea(new QScrollArea()),
        m_hardwareInfoSrollArea(new QScrollArea()),
        m_mainLayout(new QVBoxLayout()),
        m_buttonLayout(new QHBoxLayout()) {
    initGui();
    initConnect();
}

void SelectionWidget::initGui() {
    this->setProperty("useSystemStyleBlur", true);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    m_btnGroup->setExclusive(true);
    m_btnGroup->addButton(m_driverInstallBtn);
    m_btnGroup->addButton(m_driverUninstallBtn);
    m_btnGroup->addButton(m_hardwareInfoBtn);

    m_driverInstallBtn->setCheckable(true);
    m_driverInstallBtn->setText(tr("Driver Install"));
    m_driverInstallBtn->setFixedSize(FUNCTIONBUTTONSIZE);

    m_driverUninstallBtn->setCheckable(true);
    m_driverUninstallBtn->setText(tr("Driver Uninstall"));
    m_driverUninstallBtn->setFixedSize(FUNCTIONBUTTONSIZE);

    m_hardwareInfoBtn->setCheckable(true);
    m_hardwareInfoBtn->setText(tr("Hardware Info"));
    m_hardwareInfoBtn->setFixedSize(FUNCTIONBUTTONSIZE);

    m_driverInstallSrollArea->setWidget(m_driverInstallWidget);
    m_driverInstallSrollArea->setWidgetResizable(true);
    m_driverInstallSrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_driverInstallSrollArea->setFrameShape(QFrame::NoFrame);
    m_driverInstallSrollArea->setContentsMargins(0, 0, 0, 0);
    m_driverInstallWidget->adjustSize();

    m_driverUninstallSrollArea->setWidget(m_driverUninstallWidget);
    m_driverUninstallSrollArea->setWidgetResizable(true);
    m_driverUninstallSrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_driverUninstallSrollArea->setFrameShape(QFrame::NoFrame);
    m_driverUninstallSrollArea->setContentsMargins(0, 0, 0, 0);
    m_driverUninstallWidget->adjustSize();

    m_hardwareInfoSrollArea->setWidget(m_hardwareInfoWidget);
    m_hardwareInfoSrollArea->setWidgetResizable(true);
    m_hardwareInfoSrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_hardwareInfoSrollArea->setFrameShape(QFrame::NoFrame);
    m_hardwareInfoSrollArea->setContentsMargins(0, 0, 0, 0);
    m_hardwareInfoWidget->adjustSize();


    m_stackWidget->addWidget(m_driverInstallSrollArea);
    m_stackWidget->addWidget(m_driverUninstallSrollArea);
    m_stackWidget->addWidget(m_hardwareInfoSrollArea);

    m_driverInstallBtn->setChecked(true);
    m_stackWidget->setCurrentWidget(m_driverInstallWidget);

    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->setSpacing(0);
    m_buttonLayout->addWidget(m_driverInstallBtn);
    m_buttonLayout->addWidget(m_driverUninstallBtn);
    m_buttonLayout->addWidget(m_hardwareInfoBtn);
    m_buttonLayout->setAlignment(Qt::AlignCenter);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addSpacing(24);
    m_mainLayout->addWidget(m_stackWidget);
    m_mainLayout->setAlignment(Qt::AlignCenter);

    this->setLayout(m_mainLayout);
}

void SelectionWidget::initConnect() {
    connect(m_driverInstallBtn, &QAbstractButton::clicked, [=]() {
        m_driverInstallBtn->setChecked(true);
        m_stackWidget->setCurrentIndex(0);
    });
    connect(m_driverUninstallBtn, &QAbstractButton::clicked, [=]() {
        m_driverUninstallBtn->setChecked(true);
        m_stackWidget->setCurrentIndex(1);
    });
    connect(m_hardwareInfoBtn, &QAbstractButton::clicked, [=]() {
        m_hardwareInfoBtn->setChecked(true);
        m_stackWidget->setCurrentIndex(2);
    });

}
