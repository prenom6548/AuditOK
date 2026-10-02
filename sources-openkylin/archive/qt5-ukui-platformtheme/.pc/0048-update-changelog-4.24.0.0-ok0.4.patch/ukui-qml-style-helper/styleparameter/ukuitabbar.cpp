#include "ukuitabbar.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUITabBar::UKUITabBar(QQuickItem *parent) : QQuickItem(parent)
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

UKUITabBar::~UKUITabBar()
{

}

void UKUITabBar::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalColor(instance->kContainSecondaryNormal());

    emit parametryChanged();

}

UKUITabBar* UKUITabBar::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUITabBar(p);
}

const QBrush &UKUITabBar::normalColor() const
{
    return m_normalColor;
}

void UKUITabBar::setNormalColor(const QBrush &newNormalColor)
{
    if (m_normalColor == newNormalColor)
        return;
    m_normalColor = newNormalColor;
    emit normalColorChanged();
}
