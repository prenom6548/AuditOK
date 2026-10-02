#include "centerwidget.h"
#include "ConfirmButtonStyle.h"

centerWidget::centerWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setFixedSize(480, 520);
    getDiskInfo();
    initListWidget();
    initTitleLable();
    initButton();
    initMenberVar();
    initListWidgetEntry();
}

centerWidget::~centerWidget()
{

}

void centerWidget::initMenberVar()
{
    this->setContentsMargins(0, 0, 0, 0);
    m_pMainVboxLayout = new QVBoxLayout();
    m_pMainVboxLayout->setContentsMargins(0, 0, 0, 0);
    m_pMainVboxLayout->setSpacing(0);

    m_pFrameLine = new QFrame();
    m_pFrameLine->setFrameShape(QFrame::HLine);
    QPalette paletteLine = m_pFrameLine->palette();
    QColor color = paletteLine.color(paletteLine.Button);
    color.setAlphaF(0.08);
    paletteLine.setColor(QPalette::WindowText, color);
    m_pFrameLine->setPalette(paletteLine);
    m_pFrameLine->setFixedSize(478, 1);

    m_pMainVboxLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Fixed));
    m_pMainVboxLayout->addWidget(m_pTitleWidget);
    m_pMainVboxLayout->addItem(new QSpacerItem(10, 24, QSizePolicy::Fixed));
    m_pMainVboxLayout->addWidget(m_pListContentWidget);
    m_pMainVboxLayout->addItem(new QSpacerItem(10, 400, QSizePolicy::Expanding));
    m_pMainVboxLayout->addWidget(m_pFrameLine);
    m_pMainVboxLayout->addItem(new QSpacerItem(10, 16, QSizePolicy::Fixed));
    m_pMainVboxLayout->addWidget(m_pButtonWidget);
    m_pMainVboxLayout->addItem(new QSpacerItem(10, 24, QSizePolicy::Fixed));
    this->setLayout(m_pMainVboxLayout);
}

void centerWidget::initListWidget()
{
    m_pListContentWidget = new QWidget();
    m_pListContentWidget->setContentsMargins(0, 0, 0, 0);
    m_pListContentLayout = new QHBoxLayout();
    m_pListContentLayout->setContentsMargins(22, 0, 26, 0);

    m_pListWidget = new QListWidget();
    m_pListWidget->setContentsMargins(22, 0, 26, 0);
    QPalette listWidgetPalette;
    listWidgetPalette = m_pListWidget->palette();
    listWidgetPalette.setBrush(QPalette::Base, QColor(0, 0, 0, 0));
    m_pListWidget->setPalette(listWidgetPalette);
    m_pListWidget->setProperty("needTranslucent", true);

    m_pListWidget->setFrameShape(QListWidget::NoFrame);
    m_pListWidget->setFixedSize(480, 432);

    m_pListContentLayout->addWidget(m_pListWidget);
    m_pListContentWidget->setLayout(m_pListContentLayout);
}
void centerWidget::initTitleLable()
{
    m_pTitleWidget = new QWidget();
    m_pTitleWidget->setContentsMargins(0, 0, 0, 0);

    m_pHboxTitleLayout = new QHBoxLayout();
    m_pHboxTitleLayout->setContentsMargins(0, 0, 0, 0);
    m_pHboxTitleLayout->setSpacing(0);

    m_pWidgetTitleLabel = new QLabel();
    QFont font = m_pWidgetTitleLabel->font();
    font.setPixelSize(18);
    font.setFamily("Noto Sans CJK SC-Regular");
    QPalette palette;
    palette = m_pWidgetTitleLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_pWidgetTitleLabel->setPalette(palette);
    m_pWidgetTitleLabel->setFont(font);
    m_pWidgetTitleLabel->setText(QObject::tr("Select Need to repair grub"));
    m_pHboxTitleLayout->addItem(new QSpacerItem(130, 10, QSizePolicy::Expanding));
    m_pHboxTitleLayout->addWidget(m_pWidgetTitleLabel, Qt::AlignCenter);
    m_pTitleWidget->setLayout(m_pHboxTitleLayout);
}
void centerWidget::initButton()
{
    m_pButtonWidget = new QWidget();
    m_pButtonWidget->setContentsMargins(0, 0, 0, 0);

    m_pHboxButtonLayout = new QHBoxLayout();
    m_pHboxButtonLayout->setContentsMargins(0, 0, 0, 0);
    m_pHboxButtonLayout->setSpacing(0);

    ConfirmButtonStyle *p_buttonStyle = new ConfirmButtonStyle();
    m_pconfirmButton = new QPushButton();
    m_pconfirmButton->setStyle(p_buttonStyle);

    QPalette paletteButton;
    paletteButton = m_pWidgetTitleLabel->palette();
    paletteButton.setColor(QPalette::ButtonText, Qt::white);
    m_pconfirmButton->setPalette(paletteButton);
    m_pconfirmButton->setFixedSize(120, 40);
    m_pconfirmButton->setProperty("isWindowsButton", 0x2);
    m_pconfirmButton->setFocusPolicy(Qt::NoFocus);
    m_pconfirmButton->setText(QObject::tr("confirm"));
    connect(m_pconfirmButton, &QPushButton::clicked, this, &centerWidget::confirmButtonSlots);
    m_pconfirmButton->setDisabled(true);
    m_pHboxButtonLayout->addItem(new QSpacerItem(180, 10, QSizePolicy::Expanding));
    m_pHboxButtonLayout->addWidget(m_pconfirmButton);
    m_pHboxButtonLayout->addItem(new QSpacerItem(180, 10, QSizePolicy::Expanding));
    m_pButtonWidget->setLayout(m_pHboxButtonLayout);
    return;
}

void centerWidget::initListWidgetEntry()
{
    for (int i = 0; i < m_DiskInfoList.length(); i++) {
        QListWidgetItem *pListWidgetItem = new QListWidgetItem;
        pListWidgetItem->setFlags(Qt::ItemIsAutoTristate);
        pListWidgetItem->setSizeHint(QSize(432, 40));
        selectWidgetEntry *p_dislInfoEntryWidget = new selectWidgetEntry;

        connect(p_dislInfoEntryWidget, &selectWidgetEntry::updateDiskSelectEntry, \
                this, &centerWidget::updateDiskInfoEntry);
        p_dislInfoEntryWidget->setDiskName(m_DiskInfoList.at(i));
        m_pListWidget->addItem(pListWidgetItem);
        m_pListWidget->setItemWidget(pListWidgetItem, p_dislInfoEntryWidget);
    }
}

void centerWidget::getDiskInfo()
{
    QProcess process(0);
    QString cmd;
    cmd.clear();
    cmd = "os-prober";
    QStringList cmdArgvList;
    cmdArgvList.clear();
    cmdArgvList.append("/");
    process.start(cmd, cmdArgvList);
    process.waitForFinished(-1);
    QString outputString = process.readAllStandardOutput();

    // 处理读取的输出信息
    QStringList outputStringList = outputString.split("\n");
    outputStringList.removeLast();
    for (int i = 0; i < outputStringList.length(); i++) {
        QStringList stringlist1 = outputStringList.at(i).split(":");
        QStringList stringlist2 = stringlist1.at(1).split("(");
        QString diskName = QString("%1(%2)").arg(stringlist2.at(0)).arg(stringlist1.at(0));
        m_DiskInfoList.append(diskName);
    }
    qDebug() << "os-proble 分区信息: " << m_DiskInfoList;
    process.deleteLater();
    return;
}

void centerWidget::updateDiskInfoEntry(QWidget *widget)
{
    if (nullptr == widget) {
        return;
    }
    for (int i = 0; i < m_pListWidget->count(); i++) {
        QListWidgetItem *item = m_pListWidget->item(i);
        selectWidgetEntry *p_Entry = (selectWidgetEntry *)m_pListWidget->itemWidget(item);
        p_Entry->m_pIconLabel->setVisible(false);
    }
    selectWidgetEntry *p_selectWidgetEntry = (selectWidgetEntry *)widget;
    p_selectWidgetEntry->m_pIconLabel->setVisible(true);
    m_diskNameString = p_selectWidgetEntry->m_distNameSelectEntry;
    m_pconfirmButton->setDisabled(false);
    return;
}

void centerWidget::confirmButtonSlots()
{
    if (m_diskNameString.length() == 0) {
        return;
    }
    emit execSwitchWidgetFixing(this->m_diskNameString);
    return;
}
