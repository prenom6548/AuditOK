#include "addbutton.h"
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QPainter>
#include <QGSettings/QGSettings>
#include <QDebug>
#include <QApplication>

AddButton::AddButton(QWidget *parent, UkccFrame::BorderRadiusStyle style, bool heightAdaptive)
    : QPushButton(parent),
      m_radiusType(style),
      m_heightAdaptive(heightAdaptive)

{
    this->setObjectName("AddButton");
    this->setProperty("useButtonPalette", true);
    this->setProperty("needTranslucent", true);
    this->setFlat(true);

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel *iconLabel = new QLabel();
    QLabel *textLabel = new QLabel(tr("Add"));

    QIcon mAddIcon = QIcon::fromTheme("list-add-symbolic");
    iconLabel->setPixmap(mAddIcon.pixmap(mAddIcon.actualSize(QSize(16, 16))));
    iconLabel->setProperty("iconHighlightEffectMode", 1);

    const QByteArray idd(THEME_QT_SCHEMA);
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode){
        iconLabel->setProperty("useIconHighlightEffect", true);
    }
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            QString currentThemeMode = qtSettings->get(key).toString();
            if ("ukui-black" == currentThemeMode || "ukui-dark" == currentThemeMode) {
                iconLabel->setProperty("useIconHighlightEffect", true);
            } else if("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode) {
                iconLabel->setProperty("useIconHighlightEffect", false);
            }
        }
    });

    m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                             "/",
                                              "com.kylin.statusmanager.interface",
                                              QDBusConnection::sessionBus(), this);
    if (m_statusSessionDbus->isValid()) {
        QDBusReply<bool> is_tabletmode = m_statusSessionDbus->call("get_current_tabletmode");
        mode_change_signal_slots(is_tabletmode.isValid() ? is_tabletmode.value() : false);
        connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(mode_change_signal_slots(bool)));
    } else {
        mode_change_signal_slots(false);
        qWarning() << "Create com.kylin.statusmanager.interface Interface Failed When : " << QDBusConnection::systemBus().lastError();
    }

    addLyt->addStretch();
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    this->setLayout(addLyt);

}

AddButton::~AddButton()
{

}

void AddButton::mode_change_signal_slots(bool b)
{
    if (m_heightAdaptive) {
        if (b) {
            this->setMinimumSize(550, TABLET_HEIGHT);
            this->setMaximumSize(16777215, TABLET_HEIGHT);
        } else {
            this->setMinimumSize(550, PC_HEIGHT);
            this->setMaximumSize(16777215, PC_HEIGHT);
        }
    }
    m_isTabletMode = b;
    Q_EMIT tabletModeChanged(b);
}

void AddButton::setRadiusType(UkccFrame::BorderRadiusStyle style)
{
    m_radiusType = style;
    update();
}

void AddButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);
    painter.setBrush(qApp->palette().base());
    QPainterPath painterPath = getPaintrPath();
    painter.setClipPath(painterPath);
    painter.drawPath(painterPath);

    QPushButton::paintEvent(event);
}


QPainterPath AddButton::getPaintrPath()
{
    const QRect rect = this->rect();
    int radius = RADIUS_VALUE;
    int corner = 0;

    if (m_radiusType == UkccFrame::BorderRadiusStyle::None) {
        corner = 0;
    } else if (m_radiusType == UkccFrame::BorderRadiusStyle::Top) {
        corner = TOP_LEFT_CORNER + TOP_RIGHT_CORNER;
    } else if (m_radiusType == UkccFrame::BorderRadiusStyle::Bottom) {
        corner = BOTTOM_LEFT_CORNER + BOTTOM_RIGHT_CORNER;
    } else {
        corner = TOP_LEFT_CORNER + TOP_RIGHT_CORNER + BOTTOM_LEFT_CORNER + BOTTOM_RIGHT_CORNER;
    }

    QPainterPath path;
    if(corner & TOP_LEFT_CORNER) {
        path.moveTo(rect.x() + radius, rect.y());
        path.arcTo(rect.x(), rect.y(), radius * 2, radius * 2, 90.f, 90.f);//画左上圆角
    } else {
        path.moveTo(rect.x(), rect.y());
    }

    if(corner & BOTTOM_LEFT_CORNER) {
        path.lineTo(rect.x(), rect.y() + rect.height() -radius);
        path.arcTo(rect.x(), rect.y() + rect.height() - radius * 2, radius * 2, radius * 2, 180.0f, 90.f);
    } else {
        path.lineTo(rect.x(), rect.y() + rect.height());
    }

    if(corner & BOTTOM_RIGHT_CORNER) {
        path.lineTo(rect.x() + rect.width() - radius,rect.y() + rect.height());
        path.arcTo(rect.x() + rect.width() - 2 * radius, rect.y() + rect.height() - 2 * radius,radius*2, radius * 2, 270.0f, 90.f);
    } else {
        path.lineTo(rect.x()+ rect.width(), rect.y() + rect.height());
    }

    if(corner & TOP_RIGHT_CORNER) {
        path.lineTo(rect.x() + rect.width(), rect.y() + radius);
        path.arcTo(rect.x() + rect.width() - 2 * radius, rect.y(), radius * 2, radius * 2, 0.f, 90.f);
    } else {
        path.lineTo(rect.x()+ rect.width(), rect.y());
    }
    return path;
}
