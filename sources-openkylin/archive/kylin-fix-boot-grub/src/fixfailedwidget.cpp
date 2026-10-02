#include "fixfailedwidget.h"
#include "ConfirmButtonStyle.h"
fixFailedWidget::fixFailedWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
}

fixFailedWidget::~fixFailedWidget()
{

}

void fixFailedWidget::initUI()
{
    m_pFailedIconWidget = new QWidget();
    m_pFailedIconWidget->setContentsMargins(0, 0, 0, 0);

    m_pHboxIconLayout = new QHBoxLayout();
    m_pHboxIconLayout->setSpacing(0);
    m_pHboxIconLayout->setContentsMargins(0, 0, 0, 0);

    m_pFailedIconLabel = new QLabel();
    m_pFailedIconLabel->setPixmap(QIcon::fromTheme("dialog-error", \
                                                   QIcon(":/data/svg/dialog-error.png")).pixmap(48, 48));
    m_pHboxIconLayout->addItem(new QSpacerItem(216, 10, QSizePolicy::Fixed));
    m_pHboxIconLayout->addWidget(m_pFailedIconLabel);
    m_pHboxIconLayout->addItem(new QSpacerItem(216, 10, QSizePolicy::Fixed));
    m_pFailedIconWidget->setLayout(m_pHboxIconLayout);

    m_pFailedTextWidget = new QWidget();
    m_pFailedTextWidget->setContentsMargins(0, 0, 0, 0);

    m_pFailedTextLayout = new QHBoxLayout();
    m_pFailedTextLayout->setSpacing(0);
    m_pFailedTextLayout->setContentsMargins(0, 0, 0, 0);

    m_pFailedTextLabel = new QLabel();
    QFont font = m_pFailedTextLabel->font();
    font.setPixelSize(14);
    font.setFamily("Noto Sans CJK SC-Regular");
    QPalette paletteLabel;
    paletteLabel = m_pFailedTextLabel->palette();
    paletteLabel.setColor(QPalette::WindowText, Qt::white);
    m_pFailedTextLabel->setPalette(paletteLabel);
    m_pFailedTextLabel->setFont(font);
    m_pFailedTextLabel->setText(QObject::tr("Repair failed!"));

    m_pFailedTextLayout->addItem(new QSpacerItem(205, 10, QSizePolicy::Fixed));
    m_pFailedTextLayout->addWidget(m_pFailedTextLabel);
    m_pFailedTextLayout->addItem(new QSpacerItem(205, 10, QSizePolicy::Expanding));
    m_pFailedTextWidget->setLayout(m_pFailedTextLayout);

    m_pFrameLine = new QFrame();
    m_pFrameLine->setFrameShape(QFrame::HLine);
    QPalette palette = m_pFrameLine->palette();
    QColor color = palette.color(palette.Button);
    color.setAlphaF(0.5);
    palette.setColor(QPalette::WindowText, color);
    m_pFrameLine->setPalette(palette);
    m_pFrameLine->setFixedSize(478, 1);

    m_pFailedButtonWidget = new QWidget();
    m_pFailedButtonWidget->setContentsMargins(0, 0, 0, 0);

    m_pFailedButtonLayout = new QHBoxLayout();
    m_pFailedButtonLayout->setContentsMargins(0, 0, 0, 0);
    m_pFailedButtonLayout->setSpacing(0);

    ConfirmButtonStyle *p_buttonStyle = new ConfirmButtonStyle();
    m_pFailedConfirmButton = new QPushButton();
    m_pFailedConfirmButton->setStyle(p_buttonStyle);
    QPalette paletteButton;
    paletteButton = m_pFailedConfirmButton->palette();
    paletteButton.setColor(QPalette::ButtonText, Qt::white);
    m_pFailedConfirmButton->setPalette(paletteButton);
    m_pFailedConfirmButton->setFixedSize(120, 40);
    m_pFailedConfirmButton->setFocusPolicy(Qt::NoFocus);
    connect(m_pFailedConfirmButton, &QPushButton::clicked, this, &fixFailedWidget::confirmButtonFaildSlots);
    m_pFailedConfirmButton->setText(QObject::tr("confirm"));
    m_pFailedConfirmButton->setFixedSize(120, 40);

    m_pFailedButtonLayout->addItem(new QSpacerItem(180, 10, QSizePolicy::Fixed));
    m_pFailedButtonLayout->addWidget(m_pFailedConfirmButton);
    m_pFailedButtonLayout->addItem(new QSpacerItem(180, 10, QSizePolicy::Fixed));
    m_pFailedButtonWidget->setLayout(m_pFailedButtonLayout);

    m_pVboxLayout = new QVBoxLayout();
    m_pVboxLayout->setContentsMargins(0, 0, 0, 0);
    m_pVboxLayout->setSpacing(0);

    this->setContentsMargins(0, 0, 0, 0);
    m_pVboxLayout->addItem(new QSpacerItem(10, 174, QSizePolicy::Fixed));
    m_pVboxLayout->addWidget(m_pFailedIconWidget);
    m_pVboxLayout->addItem(new QSpacerItem(10, 24, QSizePolicy::Fixed));
    m_pVboxLayout->addWidget(m_pFailedTextWidget);
    m_pVboxLayout->addItem(new QSpacerItem(10, 173, QSizePolicy::Fixed));
    m_pVboxLayout->addWidget(m_pFrameLine);
    m_pVboxLayout->addItem(new QSpacerItem(10, 16, QSizePolicy::Fixed));
    m_pVboxLayout->addWidget(m_pFailedButtonWidget);
    m_pVboxLayout->addItem(new QSpacerItem(10, 16, QSizePolicy::Fixed));
    this->setLayout(m_pVboxLayout);
}

void fixFailedWidget::confirmButtonFaildSlots()
{
    exit(0);
    return;
}
