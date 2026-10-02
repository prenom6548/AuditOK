#include <QApplication>
#include "ukuiheaderview.h"
#include "qdebug.h"
using namespace UKUIQQC2Style;

UKUIHeaderView::UKUIHeaderView(QQuickItem *parent) : QQuickItem(parent)
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

UKUIHeaderView::~UKUIHeaderView()
{

}

UKUIHeaderView* UKUIHeaderView::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIHeaderView(p);
}

void UKUIHeaderView::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalTextColor(m_instance->kFontPrimary());
    setLineColor(m_instance->kLineInputNormal());
    setLineWidth(1);
    setnormalHeight(36);
    emit parametryChanged();
}


const QBrush &UKUIHeaderView::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUIHeaderView::setNormalTextColor(const QBrush &newNormalTextColor)
{
    if (m_normalTextColor == newNormalTextColor)
        return;
    m_normalTextColor = newNormalTextColor;
    emit normalTextColorChanged();
}

const QBrush &UKUIHeaderView::lineColor() const
{
    return m_lineColor;
}

void UKUIHeaderView::setLineColor(const QBrush &newLineColor)
{
    if (m_lineColor == newLineColor)
        return;
    m_lineColor = newLineColor;
    emit lineColorChanged();
}


int UKUIHeaderView::lineWidth() const
{
    return m_lineWidth;
}

void UKUIHeaderView::setLineWidth(int newLineWidth)
{
    if (m_lineWidth == newLineWidth)
        return;
    m_lineWidth = newLineWidth;
    emit lineWidthChanged();
}

int UKUIHeaderView::normalHeight() const
{
    return m_normalHeight;
}

void UKUIHeaderView::setnormalHeight(int newNormalHeight)
{
    if (m_normalHeight == newNormalHeight)
        return;
    m_normalHeight = newNormalHeight;
    emit lineWidthChanged();
}
