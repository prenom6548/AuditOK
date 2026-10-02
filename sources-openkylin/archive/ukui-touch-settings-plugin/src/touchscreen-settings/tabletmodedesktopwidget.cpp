#include "tabletmodedesktopwidget.h"
#include "picturebutton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QGSettings>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

const QByteArray tabletModeGsettings = "org.ukui.tablet-desktop";
const QString tabletMode = "desktop-mode";

TabletModeDesktopWidget::TabletModeDesktopWidget(QWidget* parent)
    : QFrame(parent)
{
    QByteArray schema(tabletModeGsettings);
    if (QGSettings::isSchemaInstalled(schema)) {
        m_tabletModeGsettings = new QGSettings(tabletModeGsettings, QByteArray(), this);
    } else {
        qCritical() << "GSetting org.ukui-tablet-desktop don't install";
        m_tabletModeGsettings = nullptr;
    }

    setContentsMargins(16, 10, 16, 10);
    setFrameShape(QFrame::Shape::Box);

    initLayout();

    initTabletModeDesktopWidget();
    initConnect();
}

void TabletModeDesktopWidget::initLayout()
{
    QVBoxLayout* tabletModeVLayout = new QVBoxLayout;
    tabletModeVLayout->setMargin(0);
    setLayout(tabletModeVLayout);
}

void TabletModeDesktopWidget::initTabletModeDesktopWidget()
{
    QWidget* tabletTextWidget = createTabletTextWidget();
    tabletTextWidget->setMinimumHeight(40);
    layout()->addWidget(tabletTextWidget);

    initButtonGroup();
    QWidget* tabletPictureWidget = createTabletPictureWidget();
    layout()->addWidget(tabletPictureWidget);
}

void TabletModeDesktopWidget::initConnect()
{
    connect(m_buttonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [this](QAbstractButton* button) {
        PictureButton* pictureButton = qobject_cast<PictureButton*> (button);
        this->setButtonChecked(pictureButton);
        this->setTabletModeGsettings(pictureButton->getThemeName());
    });
}

void TabletModeDesktopWidget::initButtonGroup()
{
    PictureButton* tabletButton = new PictureButton(tr("Tablet"),
                                               QPixmap(":/gif/resources/Tablet.png"));

    PictureButton* maxTabletButton = new PictureButton(tr("MaxTablet"),
                                               QPixmap(":/gif/resources/MaxTablet.png"));

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    m_buttonGroup->addButton(tabletButton);
    m_buttonGroup->addButton(maxTabletButton);

    m_buttonGroup->buttonClicked(tabletButton);
    tabletButton->setStatus(true);
    setTabletModeGsettings("Tablet");
}

QWidget* TabletModeDesktopWidget::createTabletTextWidget() const
{
    QWidget* tabletTextWidget = new QWidget;
    //~ contents_path /TouchScreen/Tablet mode desktop ui
    QLabel* tabletText = new QLabel(tr("Tablet mode desktop ui"));

    QHBoxLayout* tabletModeLayout = new QHBoxLayout(tabletTextWidget);
    tabletModeLayout->setContentsMargins(0, 0, 0, 0);
    tabletModeLayout->addWidget(tabletText);
    tabletModeLayout->addStretch();
    tabletTextWidget->setLayout(tabletModeLayout);

    return tabletTextWidget;
}

QWidget* TabletModeDesktopWidget::createTabletPictureWidget() const
{
    QWidget* tabletPictureWidget = new QWidget;

    QHBoxLayout* tabletPictureLayout = new QHBoxLayout(tabletPictureWidget);
    tabletPictureLayout->setContentsMargins(0, 0, 0, 0);
    tabletPictureLayout->addWidget(m_buttonGroup->buttons().at(0));
    tabletPictureLayout->addSpacing(16);
    tabletPictureLayout->addWidget(m_buttonGroup->buttons().at(1));
    tabletPictureLayout->addStretch();
    tabletPictureWidget->setLayout(tabletPictureLayout);

    return tabletPictureWidget;
}

void TabletModeDesktopWidget::setButtonChecked(PictureButton* button)
{
    button->setStatus(true);
    for (int i = 0; i < m_buttonGroup->buttons().count(); i++) {
        if (button != m_buttonGroup->buttons().at(i)) {
            (qobject_cast<PictureButton*> (m_buttonGroup->buttons().at(i)))->setStatus(false);
        }
    }
}

void TabletModeDesktopWidget::setTabletModeGsettings(const QString& type)
{
    if (m_tabletModeGsettings == nullptr) {
        return;
    }

    m_tabletModeGsettings->set(tabletMode, type);
}

bool TabletModeDesktopWidget::getCurrentTabletMode() const
{
    QDBusInterface statusManager("com.kylin.statusmanager.interface",
                                 "/",
                                 "com.kylin.statusmanager.interface");
    QDBusReply<bool> reply = statusManager.call("getCurrentTabletMode");
    if (!reply.isValid()) {
        qWarning() << reply.error();
        return false;
    }

    return reply.value();
}

