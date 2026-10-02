#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QPen>
#include <QApplication>
#include <QMouseEvent>

#include "dbus_args.h"
#include "data_checkbox.h"

DataCheckBox::DataCheckBox(bool isAppData, QWidget *parent) : QWidget(parent), m_isAppData(isAppData)
{
    initUI();
    initConn();
}

DataCheckBox::~DataCheckBox()
{

}

void DataCheckBox::setLabelText(DataType type)
{
    switch (type)
    {
    case DataType::AppData:
        m_label->setText(GlobalData::g_textReceMigrateApp);
        m_labelTip->setToolTip(GlobalData::g_textRectTipApp);
        break;
    case DataType::UserData:
        m_label->setText(GlobalData::g_textReceMigrateUser);
        m_labelTip->setToolTip(GlobalData::g_textRectTipUser);
        break;
    case DataType::SysData:
        m_label->setText(GlobalData::g_textRectMigrateSys);
        m_labelTip->setToolTip(GlobalData::g_textRectTipSys);
        break;
    default:
        break;
    }
}

bool DataCheckBox::getCilckStyle()
{
    return m_isClicked;
}

void DataCheckBox::setClickStyle()
{
    clickWidget();
}

void DataCheckBox::initAppWidget()
{
    if (m_appData == nullptr) {
        m_appData = new AcceptMigrationAppData();
        connect(m_appData, &AcceptMigrationAppData::sigAppData, this, [=](QStringList list){
            qDebug() << list;
        }, Qt::UniqueConnection);
    }
    m_appData->initTreewidgetStyle();
}

void DataCheckBox::initUI()
{    
    this->setFixedSize(600, 56);
    m_btnApp = new QPushButton(this);
    m_checkBox = new QCheckBox(this);
    m_labelTip = new QLabel(this);
    m_label = new QLabel(this);
    m_layoutH = new QHBoxLayout(this);

    m_btnApp->setIcon(QIcon::fromTheme("document-open-symbolic")); /*传输文件*/
    m_btnApp->setIconSize(QSize(16, 16));
    m_btnApp->setFixedSize(QSize(36, 36));
    m_btnApp->setProperty("isWindowButton", 0x1);
    m_btnApp->setProperty("useIconHighlightEffect", 0x2);
    m_btnApp->setFlat(true);
    m_btnApp->setFocusPolicy(Qt::NoFocus);
    m_btnApp->setToolTip(tr("Open Directory"));

    QIcon blankIcon = QIcon::fromTheme("preferences-system-details-symbolic");
    m_labelTip->setPixmap(blankIcon.pixmap(blankIcon.actualSize(QSize(14, 14))));

    m_layoutH->addSpacing(24);
    m_layoutH->addWidget(m_checkBox);
    m_layoutH->addSpacing(16);
    m_layoutH->addWidget(m_label);
    m_layoutH->addSpacing(6);
    m_layoutH->addWidget(m_labelTip);
    m_layoutH->addStretch();
    m_layoutH->addWidget(m_btnApp, 0, Qt::AlignVCenter);
    m_layoutH->addSpacing(20);
    m_layoutH->setSpacing(0);
    m_layoutH->setMargin(0);
    // m_layoutH->setContentsMargins(0, 15, 0, 15);
    
    if (!m_isAppData) {
        m_btnApp->hide();    
    }

    return;
}

void DataCheckBox::initConn()
{
    connect(m_checkBox, &QCheckBox::clicked, this, &DataCheckBox::clickWidget);
    connect(m_btnApp, &QPushButton::clicked, this, &DataCheckBox::cilckAppBtn);
}

void DataCheckBox::clickWidget()
{
    if (m_isClicked) {
        m_checkBox->setChecked(false);
        m_isClicked = m_checkBox->isChecked();
        emit sigCannel();
    } else {
        m_checkBox->setChecked(true);
        m_isClicked = m_checkBox->isChecked();
        if (m_isAppData) {
            cilckAppBtn();
        }
        emit sigClicked();
    }
    this->update();
}

void DataCheckBox::cilckAppBtn()
{
    if (m_appData == nullptr) {
        m_appData = new AcceptMigrationAppData();
        dbus = new QDBusInterface(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_DATA_MIGRATION, dbus::DBUS_INTERFACE_DATA_MIGRATION,
                                QDBusConnection::systemBus());
        connect(m_appData, &AcceptMigrationAppData::sigAppData, this, [=](QStringList list){
            qDebug() << list;

            QDBusMessage msg = dbus->call("receiverAppDataList", list);
            if (msg.errorMessage() != "") {
                qDebug() << "dbus interface error : receiverAppDataList" << msg.errorMessage();
            }
        }, Qt::UniqueConnection);
    }
    m_appData->show();
}

void DataCheckBox::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton) {
        clickWidget();
    }
}

void DataCheckBox::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = QApplication::palette().window().color();
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setPen(Qt::transparent);
    QStyleOption opt;
    QColor highLightColor = opt.palette.highlight().color();
    opt.init(this);
    painter.drawRoundedRect(opt.rect, 16, 16); // 绘制圆角
    // 点击后设置高亮
    if(m_isClicked) {
        QPen pen(highLightColor, 2);
        painter.setPen(pen);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12); // 绘制圆角
    } else {
        painter.setPen(Qt::transparent);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12); // 绘制圆角
    }
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}
