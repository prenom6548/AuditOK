#include "repairingwidget.h"

repairingWidget::repairingWidget(QWidget *parent) : QWidget(parent)
{
    initUI();

    m_pTime = new QTimer();
    m_pTime->setInterval(100);
    connect(m_pTime, &QTimer::timeout, this, &repairingWidget::finishLoadDisk);
    m_pTime->start();
}

repairingWidget::~repairingWidget()
{

}

void repairingWidget::initUI()
{
    m_pRepairIconWidget = new QWidget();
    m_pRepairIconWidget->setContentsMargins(0, 0, 0, 0);
    m_pRepairIconLayout = new QHBoxLayout();
    m_pRepairIconLayout->setContentsMargins(0, 0, 0, 0);
    m_pRepairIconLayout->setSpacing(0);

    m_pRepairIconLabel = new QLabel();
    m_pRepairIconLabel->setPixmap(QIcon::fromTheme("ukui-loading-0", \
                                                   QIcon(":/data/svg/ukui-loading-0.svg")).pixmap(48, 48));
    m_pRepairIconLayout->addItem(new QSpacerItem(255, 10, QSizePolicy::Expanding));
    m_pRepairIconLayout->addWidget(m_pRepairIconLabel);
    m_pRepairIconLayout->addItem(new QSpacerItem(255, 10, QSizePolicy::Expanding));
    m_pRepairIconWidget->setLayout(m_pRepairIconLayout);

    m_pRepairTextWidget = new QWidget();
    m_pRepairTextWidget->setContentsMargins(0, 0, 0, 0);

    m_pRepairTextLayout = new QHBoxLayout();
    m_pRepairTextLayout->setContentsMargins(0, 0, 0, 0);
    m_pRepairTextLayout->setSpacing(0);

    m_pRepairTextLabel = new QLabel();
    QFont font = m_pRepairTextLabel->font();
    font.setPixelSize(14);
    font.setFamily("Noto Sans CJK SC-Regular");
    QPalette palette;
    palette = m_pRepairTextLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_pRepairTextLabel->setPalette(palette);
    m_pRepairTextLabel->setFont(font);
    m_pRepairTextLabel->setText(QObject::tr("Repairing Grub, please wait..."));
    m_pRepairTextLayout->addItem(new QSpacerItem(161, 10, QSizePolicy::Expanding));
    m_pRepairTextLayout->addWidget(m_pRepairTextLabel);
    m_pRepairTextLayout->addItem(new QSpacerItem(161, 10, QSizePolicy::Expanding));
    m_pRepairTextWidget->setLayout(m_pRepairTextLayout);

    this->setContentsMargins(0, 0, 0, 0);
    m_pVboxRepairLayout = new QVBoxLayout();
    m_pVboxRepairLayout->setSpacing(0);
    m_pVboxRepairLayout->setContentsMargins(0, 0, 0, 0);
    m_pVboxRepairLayout->addItem(new QSpacerItem(10, 197, QSizePolicy::Expanding));
    m_pVboxRepairLayout->addWidget(m_pRepairIconWidget);
    m_pVboxRepairLayout->addItem(new QSpacerItem(10, 24, QSizePolicy::Fixed));
    m_pVboxRepairLayout->addWidget(m_pRepairTextWidget);
    m_pVboxRepairLayout->addItem(new QSpacerItem(10, 249, QSizePolicy::Expanding));
    this->setLayout(m_pVboxRepairLayout);
}

void repairingWidget::finishLoadDisk()
{
    m_iconNum++;
    if(m_iconNum > 7) {
        m_iconNum = 0;
    }
    QString s = QString(":/data/svg/ukui-loading-%1.svg").arg(m_iconNum);
    m_pRepairIconLabel->setPixmap(QPixmap(s).scaled(QSize(48, 48), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_pRepairIconLabel->setScaledContents(true);
    m_pRepairIconLabel->update();
}
