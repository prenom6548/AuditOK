#include "successwidget.h"

successWidget::successWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    m_pTime = new QTimer();
    m_pTime->setInterval(3000);
    connect(m_pTime, &QTimer::timeout, this, &successWidget::timerRebootSlots);
}

successWidget::~successWidget()
{

}

void successWidget::initUI()
{
    m_pIconLabelWidget = new QWidget();
    m_pIconLabelWidget->setContentsMargins(0, 0, 0, 0);

    m_pHboxIconLabelLayout = new QHBoxLayout();
    m_pHboxIconLabelLayout->setContentsMargins(0, 0, 0, 0);
    m_pHboxIconLabelLayout->setSpacing(0);


    m_pSuccessIconLabel = new QLabel();
    m_pSuccessIconLabel->setPixmap(QIcon::fromTheme("ukui-dialog-success", \
                                                    QIcon(":/data/svg/ukui-dialog-success.png")).pixmap(48, 48));

    m_pHboxIconLabelLayout->addItem(new QSpacerItem(221, 10, QSizePolicy::Expanding));
    m_pHboxIconLabelLayout->addWidget(m_pSuccessIconLabel);
    m_pHboxIconLabelLayout->addItem(new QSpacerItem(221, 10, QSizePolicy::Expanding));
    m_pIconLabelWidget->setLayout(m_pHboxIconLabelLayout);

    m_pTextLabelWidget = new QWidget();
    m_pTextLabelWidget->setContentsMargins(0, 0, 0, 0);

    m_pHboxTextLabelWidget = new QHBoxLayout();
    m_pHboxTextLabelWidget->setContentsMargins(0, 0, 0, 0);
    m_pHboxTextLabelWidget->setSpacing(0);

    m_pSuccessTextLabel = new QLabel();
    QFont font = m_pSuccessTextLabel->font();
    font.setPixelSize(14);
    font.setFamily("Noto Sans CJK SC-Regular");
    QPalette palette;
    palette = m_pSuccessTextLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_pSuccessTextLabel->setPalette(palette);
    m_pSuccessTextLabel->setFont(font);
    m_pSuccessTextLabel->setMidLineWidth(224);
    m_pSuccessTextLabel->setWordWrap(true);
    m_pSuccessTextLabel->setText(QObject::tr("Repair succeeded! The system will restart automatically in 3 seconds"));

    m_pHboxTextLabelWidget->addItem(new QSpacerItem(128, 10, QSizePolicy::Fixed));
    m_pHboxTextLabelWidget->addWidget(m_pSuccessTextLabel);
    m_pHboxTextLabelWidget->addItem(new QSpacerItem(128, 10, QSizePolicy::Fixed));
    m_pTextLabelWidget->setLayout(m_pHboxTextLabelWidget);

    m_pVboxLayout = new QVBoxLayout();
    m_pVboxLayout->setSpacing(0);
    m_pVboxLayout->setContentsMargins(0, 0, 0, 0);

    this->setContentsMargins(0, 0, 0, 0);
    m_pVboxLayout->addItem(new QSpacerItem(10, 192, QSizePolicy::Fixed));
    m_pVboxLayout->addWidget(m_pIconLabelWidget);
    m_pVboxLayout->addItem(new QSpacerItem(10, 21, QSizePolicy::Fixed));
    m_pVboxLayout->addWidget(m_pTextLabelWidget);
    m_pVboxLayout->addItem(new QSpacerItem(10, 239, QSizePolicy::Expanding));
    this->setLayout(m_pVboxLayout);
}

void successWidget::timerRebootSlots()
{
    QProcess::execute("systemctl reboot");
    qDebug() << "--------->>>>>>>>>>>>>.执行成功";
}
