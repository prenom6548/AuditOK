#include <QApplication>
#include "ukuitextfiled.h"
#include "qdebug.h"
using namespace UKUIQQC2Style;

UKUITextFiled::UKUITextFiled(QQuickItem *parent) : QQuickItem(parent)
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

UKUITextFiled::~UKUITextFiled()
{

}

UKUITextFiled* UKUITextFiled::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUITextFiled(p);
}

void UKUITextFiled::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setLeftRightPadding(8);
    setRadius(instance->kradiusNormal());
    setBorderWidth(instance->normalline());
    setFocusBorderWidth(instance->focusline());
    setNormalWidth(180);
    setNormalHeight(36);

    setInputNormalBC(instance->kComponentInput());
    setNormalBC(instance->kComponentNormal());
    setHoveredBC(instance->kComponentHover());
    setClickedBC(instance->kComponentClick());
    setDisableBC(instance->kComponentDisable());

    setNormalBorderColor(instance->kLineComponentNormal());
    setHoverBorderColor(instance->kLineComponentHover());
    setDisableBorderColor(instance->kLineComponentDisable());
    setFocusBorderColor(instance->kBrandNormal());

    setPlaceHolderNormalTextColor(instance->kFontPlaceholdertext());
    setPlaceHolderDisableTextColor(instance->kFontPlaceholdertextDisable());
    setNormalTextColor(instance->kFontPrimary());
    setDisableTextColor(instance->kFontPlaceholdertextDisable());

    setInputNormalTransparentBC(instance->kComponentInputAlpha());
    setNormalTransparentBC(instance->kComponentAlphaNormal());
    setHoveredTransparentBC(instance->kComponentAlphaHover());
    setClickedTransparentBC(instance->kComponentAlphaClick());
    setDisableTransparentBC(instance->kComponentAlphaDisable());

    emit parametryChanged();

}

double UKUITextFiled::radius() const
{
    return m_radius;
}

void UKUITextFiled::setRadius(double newRadius)
{
    if (qFuzzyCompare(m_radius, newRadius))
        return;
    m_radius = newRadius;
    emit radiusChanged();
}

double UKUITextFiled::leftRightPadding() const
{
    return m_leftRightPadding;
}

void UKUITextFiled::setLeftRightPadding(double newLeftRightPadding)
{
    if (qFuzzyCompare(m_leftRightPadding, newLeftRightPadding))
        return;
    m_leftRightPadding = newLeftRightPadding;
    emit leftRightPaddingChanged();
}

double UKUITextFiled::upDownMargin() const
{
    return m_upDownMargin;
}

void UKUITextFiled::setUpDownMargin(double newUpDownMargin)
{
    if (qFuzzyCompare(m_upDownMargin, newUpDownMargin))
        return;
    m_upDownMargin = newUpDownMargin;
    emit upDownMarginChanged();
}

double UKUITextFiled::space() const
{
    return m_space;
}

void UKUITextFiled::setSpace(double newSpace)
{
    if (qFuzzyCompare(m_space, newSpace))
        return;
    m_space = newSpace;
    emit spaceChange();
}

int UKUITextFiled::normalWidth() const
{
    return m_normalWidth;
}

void UKUITextFiled::setNormalWidth(int newNormalWidth)
{
    if (m_normalWidth == newNormalWidth)
        return;
    m_normalWidth = newNormalWidth;
    emit normalWidthChanged();
}

int UKUITextFiled::normalHeight() const
{
    return m_normalHeight;
}

void UKUITextFiled::setNormalHeight(int newNormalHeight)
{
    if (m_normalHeight == newNormalHeight)
        return;
    m_normalHeight = newNormalHeight;
    emit normalHeightChanged();
}

const QBrush &UKUITextFiled::normalBC() const
{
    return m_normalBC;
}

void UKUITextFiled::setNormalBC(const QBrush &newNormalBC)
{
    if (m_normalBC == newNormalBC)
        return;
    m_normalBC = newNormalBC;
    emit normalBCChanged();
}

const QBrush &UKUITextFiled::clickedBC() const
{
    return m_clickedBC;
}

void UKUITextFiled::setClickedBC(const QBrush &newClickedBC)
{
    if (m_clickedBC == newClickedBC)
        return;
    m_clickedBC = newClickedBC;
    emit clickedBCChanged();
}

const QBrush &UKUITextFiled::hoveredBC() const
{
    return m_hoveredBC;
}

void UKUITextFiled::setHoveredBC(const QBrush &newHoveredBC)
{
    if (m_hoveredBC == newHoveredBC)
        return;
    m_hoveredBC = newHoveredBC;
    emit hoveredBCChanged();
}

const QBrush &UKUITextFiled::disableBC() const
{
    return m_disableBC;
}

void UKUITextFiled::setDisableBC(const QBrush &newDisableBC)
{
    if (m_disableBC == newDisableBC)
        return;
    m_disableBC = newDisableBC;
    emit disableBCChanged();
}

const QBrush &UKUITextFiled::disableTextColor() const
{
    return m_disableTextColor;
}

void UKUITextFiled::setDisableTextColor(const QBrush &newDisableTextColor)
{
    if (m_disableTextColor == newDisableTextColor)
        return;
    m_disableTextColor = newDisableTextColor;
    emit disableTextColorChanged();
}

const QBrush &UKUITextFiled::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUITextFiled::setNormalTextColor(const QBrush &newNormalTextColor)
{
    if (m_normalTextColor == newNormalTextColor)
        return;
    m_normalTextColor = newNormalTextColor;
    emit normalTextColorChanged();
}

int UKUITextFiled::borderWidth() const
{
    return m_borderWidth;
}

void UKUITextFiled::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

int UKUITextFiled::focusBorderWidth() const
{
    return m_focusBorderWidth;
}

void UKUITextFiled::setFocusBorderWidth(int newFocusBorderWidth)
{
    if (m_focusBorderWidth == newFocusBorderWidth)
        return;
    m_focusBorderWidth = newFocusBorderWidth;
    emit focusBorderWidthChanged();
}

const QBrush &UKUITextFiled::normalBorderColor() const
{
    return m_normalBorderColor;
}

void UKUITextFiled::setNormalBorderColor(const QBrush &newNormalBorderColor)
{
    if (m_normalBorderColor == newNormalBorderColor)
        return;
    m_normalBorderColor = newNormalBorderColor;
    emit normalBorderColorChanged();
}

const QBrush &UKUITextFiled::clickBorderColor() const
{
    return m_clickBorderColor;
}

void UKUITextFiled::setClickBorderColor(const QBrush &newClickBorderColor)
{
    if (m_clickBorderColor == newClickBorderColor)
        return;
    m_clickBorderColor = newClickBorderColor;
    emit clickBorderColorChanged();
}

const QBrush &UKUITextFiled::disableBorderColor() const
{
    return m_disableBorderColor;
}

void UKUITextFiled::setDisableBorderColor(const QBrush &newDisableBorderColor)
{
    if (m_disableBorderColor == newDisableBorderColor)
        return;
    m_disableBorderColor = newDisableBorderColor;
    emit disableBorderColorChanged();
}

const QBrush &UKUITextFiled::focusBorderColor() const
{
    return m_focusBorderColor;
}

void UKUITextFiled::setFocusBorderColor(const QBrush &newFocusBorderColor)
{
    if (m_focusBorderColor == newFocusBorderColor)
        return;
    m_focusBorderColor = newFocusBorderColor;
    emit focusBorderColorChanged();
}

const QBrush &UKUITextFiled::hoverBorderColor() const
{
    return m_hoverBorderColor;
}

void UKUITextFiled::setHoverBorderColor(const QBrush &newHoverBorderColor)
{
    if (m_hoverBorderColor == newHoverBorderColor)
        return;
    m_hoverBorderColor = newHoverBorderColor;
    emit hoverBorderColorChanged();
}

const QBrush &UKUITextFiled::placeHolderNormalTextColor() const
{
    return m_placeHolderNormalTextColor;
}

void UKUITextFiled::setPlaceHolderNormalTextColor(const QBrush &newPlaceHolderNormalTextColor)
{
    if (m_placeHolderNormalTextColor == newPlaceHolderNormalTextColor)
        return;
    m_placeHolderNormalTextColor = newPlaceHolderNormalTextColor;
    emit placeHolderNormalTextColorChanged();
}

const QBrush &UKUITextFiled::placeHolderDisableTextColor() const
{
    return m_placeHolderDisableTextColor;
}

void UKUITextFiled::setPlaceHolderDisableTextColor(const QBrush &newPlaceHolderDisableTextColor)
{
    if (m_placeHolderDisableTextColor == newPlaceHolderDisableTextColor)
        return;
    m_placeHolderDisableTextColor = newPlaceHolderDisableTextColor;
    emit placeHolderDisableTextColorChanged();
}

const QBrush &UKUITextFiled::inputNormalBC() const
{
    return m_inputNormalBC;
}

void UKUITextFiled::setInputNormalBC(const QBrush &newInputNormalBC)
{
    if (m_inputNormalBC == newInputNormalBC)
        return;
    m_inputNormalBC = newInputNormalBC;
    emit inputNormalBCChanged();
}

const QBrush &UKUITextFiled::normalTransparentBC() const
{
    return m_normalTransparentBC;
}

void UKUITextFiled::setNormalTransparentBC(const QBrush &newNormalTransparentBC)
{
    if (m_normalTransparentBC == newNormalTransparentBC)
        return;
    m_normalTransparentBC = newNormalTransparentBC;
    emit normalTransparentBCChanged();
}

const QBrush &UKUITextFiled::inputNormalTransparentBC() const
{
    return m_inputNormalTransparentBC;
}

void UKUITextFiled::setInputNormalTransparentBC(const QBrush &newInputNormalTransparentBC)
{
    if (m_inputNormalTransparentBC == newInputNormalTransparentBC)
        return;
    m_inputNormalTransparentBC = newInputNormalTransparentBC;
    emit inputNormalTransparentBCChanged();
}

const QBrush &UKUITextFiled::clickedTransparentBC() const
{
    return m_clickedTransparentBC;
}

void UKUITextFiled::setClickedTransparentBC(const QBrush &newClickedTransparentBC)
{
    if (m_clickedTransparentBC == newClickedTransparentBC)
        return;
    m_clickedTransparentBC = newClickedTransparentBC;
    emit clickedTransparentBCChanged();
}

const QBrush &UKUITextFiled::hoveredTransparentBC() const
{
    return m_hoveredTransparentBC;
}

void UKUITextFiled::setHoveredTransparentBC(const QBrush &newHoveredTransparentBC)
{
    if (m_hoveredTransparentBC == newHoveredTransparentBC)
        return;
    m_hoveredTransparentBC = newHoveredTransparentBC;
    emit hoveredTransparentBCChanged();
}

const QBrush &UKUITextFiled::disableTransparentBC() const
{
    return m_disableTransparentBC;
}

void UKUITextFiled::setDisableTransparentBC(const QBrush &newDisableTransparentBC)
{
    if (m_disableTransparentBC == newDisableTransparentBC)
        return;
    m_disableTransparentBC = newDisableTransparentBC;
    emit disableTransparentBCChanged();
}
