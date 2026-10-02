#include "ukuilabel.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUILabel::UKUILabel(QQuickItem *parent) : QQuickItem(parent)
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
    }
}

UKUILabel::~UKUILabel()
{

}

void UKUILabel::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalColor(instance->textActive());
    setDisableColor(instance->textDisable());

    emit parametryChanged();

}

UKUILabel* UKUILabel::qmlAttachedProperties(QObject *parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUILabel(p);
}

const QBrush  &UKUILabel::normalColor() const
{
    return m_normalColor;
}

void UKUILabel::setNormalColor(const QBrush  &newNormalColor)
{
    if (m_normalColor == newNormalColor)
        return;
    m_normalColor = newNormalColor;
    emit normalColorChanged();
}

const QBrush  &UKUILabel::linkColor() const
{
    return m_linkColor;
}

void UKUILabel::setLinkColor(const QBrush  &newLinkColor)
{
    if (m_linkColor == newLinkColor)
        return;
    m_linkColor = newLinkColor;
    emit linkColorChanged();
}


const QBrush  &UKUILabel::disableColor() const
{
    return m_disableColor;
}

void UKUILabel::setDisableColor(const QBrush  &newDisableColor)
{
    if (m_disableColor == newDisableColor)
        return;
    m_disableColor = newDisableColor;
    emit disableColorChanged();
}
