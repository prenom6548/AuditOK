#include "ukuitooltip.h"
#include "qdebug.h"
#include "settings/ukui-style-settings.h"

using namespace UKUIQQC2Style;

UKUIToolTip::UKUIToolTip(QQuickItem *parent) : QQuickItem(parent)
{
    if(!qApp || !qApp->property("qqc2-globaltoken").isValid())
        return;
    TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
    if(token && token->getInstance()){
        m_instance = token->getInstance();
        initParam(m_instance);
        connect(m_instance, &UKUIGlobalDTConfig::GlobalDTConfig::tokenChanged, this, [=](){
            initParam(m_instance);
        }, Qt::UniqueConnection);
        if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
            auto settings = UKUIStyleSettings::globalInstance();
            connect(settings, &UKUIStyleSettings::changed, this, [=](const QString &key) {
                if (key == "menuTransparency" || key == "menu-transparency") {
                    initParam(m_instance);
                    return;
                }
            }, Qt::UniqueConnection
            );
        }
    }
}

UKUIToolTip::~UKUIToolTip()
{

}

void UKUIToolTip::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setPadding(8);
    setBackBorderColor(instance->kLineWindowInactive());
    setBackColor(instance->kMenu());
    //setShadowColor(instance->kShadowMin());
    setTextColor(instance->windowTextActive());
    setRadius(instance->kradiusMenu());
    QColor c = instance->kMenu().color();

    if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
        auto opacity = UKUIStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
        c.setAlphaF(opacity);
    }
    setBackTransparentColor(c);
    setBackTransparentBorderColor(instance->kLineWindowInactive());
    emit parametryChanged();

}

UKUIToolTip* UKUIToolTip::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIToolTip(p);
}

int UKUIToolTip::padding() const
{
    return m_padding;
}

void UKUIToolTip::setPadding(int newPadding)
{
    if (m_padding == newPadding)
        return;
    m_padding = newPadding;
    emit paddingChanged();
}

const QBrush &UKUIToolTip::backColor() const
{
    return m_backColor;
}

void UKUIToolTip::setBackColor(const QBrush &newBackColor)
{
    if (m_backColor == newBackColor)
        return;
    m_backColor = newBackColor;
    emit backColorChanged();
}

const QBrush &UKUIToolTip::backBorderColor() const
{
    return m_backBorderColor;
}

void UKUIToolTip::setBackBorderColor(const QBrush &newBackBorderColor)
{
    if (m_backBorderColor == newBackBorderColor)
        return;
    m_backBorderColor = newBackBorderColor;
    emit backBorderColorChanged();
}

const QBrush &UKUIToolTip::shadowColor() const
{
    return m_shadowColor;
}

void UKUIToolTip::setShadowColor(const QBrush &newShadowColor)
{
    if (m_shadowColor == newShadowColor)
        return;
    m_shadowColor = newShadowColor;
    emit shadowColorChanged();
}

const QBrush &UKUIToolTip::textColor() const
{
    return m_textColor;
}

void UKUIToolTip::setTextColor(const QBrush &newTextColor)
{
    if (m_textColor == newTextColor)
        return;
    m_textColor = newTextColor;
    emit textColorChanged();
}

int UKUIToolTip::radius() const
{
    return m_radius;
}

void UKUIToolTip::setRadius(int newRadius)
{
    if (m_radius == newRadius)
        return;
    m_radius = newRadius;
    emit radiusChanged();
}

int UKUIToolTip::margins() const
{
    return m_margins;
}

void UKUIToolTip::setMargins(int newMargins)
{
    if (m_margins == newMargins)
        return;
    m_margins = newMargins;
    emit marginsChanged();
}

const QBrush &UKUIToolTip::backTransparentColor() const
{
    return m_backTransparentColor;
}

void UKUIToolTip::setBackTransparentColor(const QBrush &newBackTransparentColor)
{
    if (m_backTransparentColor == newBackTransparentColor)
        return;
    m_backTransparentColor = newBackTransparentColor;
    emit backTransparentColorChanged();
}

const QBrush &UKUIToolTip::backTransparentBorderColor() const
{
    return m_backBorderTransparentColor;
}

void UKUIToolTip::setBackTransparentBorderColor(const QBrush &newBackBorderTransparentColor)
{
    if (m_backBorderTransparentColor == newBackBorderTransparentColor)
        return;
    m_backBorderTransparentColor = newBackBorderTransparentColor;
    emit backTransparentBorderColorChanged();
}
