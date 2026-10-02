#include "ukuiitemdelegate.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUIItemDelegate::UKUIItemDelegate(QQuickItem *parent) : QQuickItem(parent)
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

UKUIItemDelegate::~UKUIItemDelegate()
{

}

void UKUIItemDelegate::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setPadding(12);
    setRadius(instance->kradiusNormal());
    setNormalTextColor(instance->kFontPrimary());
    setDisableTextColor(instance->kFontPrimaryDisable());
    setNormalHTextColor(instance->kFontWhite());
    setDisableHTextColor(instance->kFontWhiteDisable());

    setNormalBC(QBrush(QColor(0,0,0,0)));
    setClickedBC(instance->kContainClick());
    setHoveredBC(instance->kContainHover());
    setDisableBC(QBrush(QColor(0,0,0,0)));

    setBorderWidth(1);
    setNormalBorderColor(QBrush(QColor(0,0,0,0)));
    setClickBorderColor(QBrush(QColor(0,0,0,0)));
    setHoverBorderColor(QBrush(QColor(0,0,0,0)));
    setDisableBorderColor(QBrush(QColor(0,0,0,0)));

    setNormalHBC(instance->kBrandNormal());
    setNormalBorderHColor(instance->kLineBrandNormal());
    setHoveredHBC(instance->kBrandHover());
    setHoverBorderHColor(instance->kLineBrandHover());
    setClickedHBC(instance->kBrandClick());
    setClickBorderHColor(instance->kLineBrandClick());
    setDisableHBC(instance->kBrandDisable());
    setDisableBorderHColor(QBrush(QColor(0,0,0,0)));

    setNormalCheckedBC(instance->kComponentSelectedNormal());
    setNormalBorderCheckedColor(QBrush(QColor(0,0,0,0)));
    setHoveredCheckedBC(instance->kComponentSelectedHover());
    setHoverBorderCheckedColor(QBrush(QColor(0,0,0,0)));
    setClickedCheckedBC(instance->kComponentSelectedClick());
    setClickBorderCheckedColor(QBrush(QColor(0,0,0,0)));
    setDisableCheckedBC(instance->kComponentSelectedDisable());
    setDisableBorderCheckedColor(QBrush(QColor(0,0,0,0)));

    setNormalTransparentBC(QBrush(QColor(0,0,0,0)));
    setHoveredTransparentBC(instance->kContainAlphaHover());
    setClickedTransparentBC(instance->kContainAlphaClick());
    setDisableTransparentBC(QBrush(QColor(0,0,0,0)));
    setNormalTransparentBorderColor(QBrush(QColor(0,0,0,0)));
    setHoveredTransparentBorderColor(QBrush(QColor(0,0,0,0)));
    setClickedTransparentBorderColor(QBrush(QColor(0,0,0,0)));
    setDisableTransparentBorderColor(QBrush(QColor(0,0,0,0)));

    setNormalCheckedTransparentBC(instance->kComponentSelectedAlphaNormal());
    setHoveredCheckedTransparentBC(instance->kComponentSelectedAlphaHover());
    setClickedCheckedTransparentBC(instance->kComponentSelectedAlphaClick());
    setDisableCheckedTransparentBC(instance->kComponentSelectedAlphaDisable());
    setNormalCheckedTransparentBorderColor(QBrush(QColor(0,0,0,0)));
    setHoveredCheckedTransparentBorderColor(QBrush(QColor(0,0,0,0)));
    setClickedCheckedTransparentBorderColor(QBrush(QColor(0,0,0,0)));
    setDisableCheckedTransparentBorderColor(QBrush(QColor(0,0,0,0)));

    setNormalAlternateBC(instance->kContainSecondaryNormal());
    setClickedAlternateBC(instance->kContainClick());
    setHoveredAlternateBC(instance->kContainHover());
    setDisableAlternateBC(instance->kContainSecondaryNormal());
     setNormalAlternateTransparentBC(instance->kContainSecondaryAlphaNormal());
    setClickedAlternateTransparentBC(instance->kContainAlphaClick());
    setHoveredAlternateTransparentBC(instance->kContainAlphaHover());
    setDisableAlternateTransparentBC(instance->kContainSecondaryAlphaNormal());

    emit parametryChanged();

}


UKUIItemDelegate* UKUIItemDelegate::qmlAttachedProperties(QObject *parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIItemDelegate(p);
}

QBrush UKUIItemDelegate::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUIItemDelegate::setNormalTextColor(QBrush newNormalTextColor)
{
    if (m_normalTextColor == newNormalTextColor)
        return;
    m_normalTextColor = newNormalTextColor;
    emit normalTextColorChanged();
}

QBrush UKUIItemDelegate::disableTextColor() const
{
    return m_disableTextColor;
}

void UKUIItemDelegate::setDisableTextColor(QBrush newDisableTextColor)
{
    if (m_disableTextColor == newDisableTextColor)
        return;
    m_disableTextColor = newDisableTextColor;
    emit disableTextColorChanged();
}

QBrush UKUIItemDelegate::normalBC() const
{
    return m_normalBC;
}

void UKUIItemDelegate::setNormalBC(QBrush newNormalBC)
{
    if (m_normalBC == newNormalBC)
        return;
    m_normalBC = newNormalBC;
    emit normalBCChanged();
}

QBrush UKUIItemDelegate::clickedBC() const
{
    return m_clickedBC;
}

void UKUIItemDelegate::setClickedBC(QBrush newClickedBC)
{
    if (m_clickedBC == newClickedBC)
        return;
    m_clickedBC = newClickedBC;
    emit clickedBCChanged();
}

QBrush UKUIItemDelegate::hoveredBC() const
{
    return m_hoveredBC;
}

void UKUIItemDelegate::setHoveredBC(QBrush newHoveredBC)
{
    if (m_hoveredBC == newHoveredBC)
        return;
    m_hoveredBC = newHoveredBC;
    emit hoveredBCChanged();
}

QBrush UKUIItemDelegate::disableBC() const
{
    return m_disableBC;
}

void UKUIItemDelegate::setDisableBC(QBrush newDisableBC)
{
    if (m_disableBC == newDisableBC)
        return;
    m_disableBC = newDisableBC;
    emit disableBCChanged();
}

int UKUIItemDelegate::radius() const
{
    return m_radius;
}

void UKUIItemDelegate::setRadius(int newRadius)
{
    if (m_radius == newRadius)
        return;
    m_radius = newRadius;
    emit radiusChanged();
}

int UKUIItemDelegate::borderWidth() const
{
    return m_borderWidth;
}

void UKUIItemDelegate::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

QBrush UKUIItemDelegate::normalBorderColor() const
{
    return m_normalBorderColor;
}

void UKUIItemDelegate::setNormalBorderColor(QBrush newNormalBorderColor)
{
    if (m_normalBorderColor == newNormalBorderColor)
        return;
    m_normalBorderColor = newNormalBorderColor;
    emit normalBorderColorChanged();
}

QBrush UKUIItemDelegate::hoverBorderColor() const
{
    return m_hoverBorderColor;
}

void UKUIItemDelegate::setHoverBorderColor(QBrush newHoverBorderColor)
{
    if (m_hoverBorderColor == newHoverBorderColor)
        return;
    m_hoverBorderColor = newHoverBorderColor;
    emit hoverBorderColorChanged();
}

QBrush UKUIItemDelegate::clickBorderColor() const
{
    return m_clickBorderColor;
}

void UKUIItemDelegate::setClickBorderColor(QBrush newClickBorderColor)
{
    if (m_clickBorderColor == newClickBorderColor)
        return;
    m_clickBorderColor = newClickBorderColor;
    emit clickBorderColorChanged();
}

QBrush UKUIItemDelegate::disableBorderColor() const
{
    return m_disableBorderColor;
}

void UKUIItemDelegate::setDisableBorderColor(QBrush newDisableBorderColor)
{
    if (m_disableBorderColor == newDisableBorderColor)
        return;
    m_disableBorderColor = newDisableBorderColor;
    emit disableBorderColorChanged();
}

const QBrush &UKUIItemDelegate::clickedHBC() const
{
    return m_clickedHBC;
}

void UKUIItemDelegate::setClickedHBC(const QBrush &newClickedHBC)
{
    if (m_clickedHBC == newClickedHBC)
        return;
    m_clickedHBC = newClickedHBC;
    emit clickedHBCChanged();
}

const QBrush &UKUIItemDelegate::hoveredHBC() const
{
    return m_hoveredHBC;
}

void UKUIItemDelegate::setHoveredHBC(const QBrush &newHoveredHBC)
{
    if (m_hoveredHBC == newHoveredHBC)
        return;
    m_hoveredHBC = newHoveredHBC;
    emit hoveredHBCChanged();
}

const QBrush &UKUIItemDelegate::hoverBorderHColor() const
{
    return m_hoverBorderHColor;
}

void UKUIItemDelegate::setHoverBorderHColor(const QBrush &newHoverBorderHColor)
{
    if (m_hoverBorderHColor == newHoverBorderHColor)
        return;
    m_hoverBorderHColor = newHoverBorderHColor;
    emit hoverBorderHColorChanged();
}

const QBrush &UKUIItemDelegate::clickBorderHColor() const
{
    return m_clickBorderHColor;
}

void UKUIItemDelegate::setClickBorderHColor(const QBrush &newClickBorderHColor)
{
    if (m_clickBorderHColor == newClickBorderHColor)
        return;
    m_clickBorderHColor = newClickBorderHColor;
    emit clickBorderHColorChanged();
}

const QBrush &UKUIItemDelegate::normalHBC() const
{
    return m_normalHBC;
}

void UKUIItemDelegate::setNormalHBC(const QBrush &newNormalHBC)
{
    if (m_normalHBC == newNormalHBC)
        return;
    m_normalHBC = newNormalHBC;
    emit normalHBCChanged();
}

const QBrush &UKUIItemDelegate::normalBorderHColor() const
{
    return m_normalBorderHColor;
}

void UKUIItemDelegate::setNormalBorderHColor(const QBrush &newNormalBorderHColor)
{
    if (m_normalBorderHColor == newNormalBorderHColor)
        return;
    m_normalBorderHColor = newNormalBorderHColor;
    emit normalBorderHColorChanged();
}

const QBrush &UKUIItemDelegate::normalCheckedBC() const
{
    return m_normalCheckedBC;
}

void UKUIItemDelegate::setNormalCheckedBC(const QBrush &newNormalCheckedBC)
{
    if (m_normalCheckedBC == newNormalCheckedBC)
        return;
    m_normalCheckedBC = newNormalCheckedBC;
    emit normalCheckedBCChanged();
}

const QBrush &UKUIItemDelegate::clickedCheckedBC() const
{
    return m_clickedCheckedBC;
}

void UKUIItemDelegate::setClickedCheckedBC(const QBrush &newClickedCheckedBC)
{
    if (m_clickedCheckedBC == newClickedCheckedBC)
        return;
    m_clickedCheckedBC = newClickedCheckedBC;
    emit clickedCheckedBCChanged();
}

const QBrush &UKUIItemDelegate::hoveredCheckedBC() const
{
    return m_hoveredCheckedBC;
}

void UKUIItemDelegate::setHoveredCheckedBC(const QBrush &newHoveredCheckedBC)
{
    if (m_hoveredCheckedBC == newHoveredCheckedBC)
        return;
    m_hoveredCheckedBC = newHoveredCheckedBC;
    emit hoveredCheckedBCChanged();
}

const QBrush &UKUIItemDelegate::normalBorderCheckedColor() const
{
    return m_normalBorderCheckedColor;
}

void UKUIItemDelegate::setNormalBorderCheckedColor(const QBrush &newNormalBorderCheckedColor)
{
    if (m_normalBorderCheckedColor == newNormalBorderCheckedColor)
        return;
    m_normalBorderCheckedColor = newNormalBorderCheckedColor;
    emit normalBorderCheckedColorChanged();
}

const QBrush &UKUIItemDelegate::hoverBorderCheckedColor() const
{
    return m_hoverBorderCheckedColor;
}

void UKUIItemDelegate::setHoverBorderCheckedColor(const QBrush &newHoverBorderCheckedColor)
{
    if (m_hoverBorderCheckedColor == newHoverBorderCheckedColor)
        return;
    m_hoverBorderCheckedColor = newHoverBorderCheckedColor;
    emit hoverBorderCheckedColorChanged();
}

const QBrush &UKUIItemDelegate::clickBorderCheckedColor() const
{
    return m_clickBorderCheckedColor;
}

void UKUIItemDelegate::setClickBorderCheckedColor(const QBrush &newClickBorderCheckedColor)
{
    if (m_clickBorderCheckedColor == newClickBorderCheckedColor)
        return;
    m_clickBorderCheckedColor = newClickBorderCheckedColor;
    emit clickBorderCheckedColorChanged();
}

int UKUIItemDelegate::implicitHeight() const
{
    return m_implicitHeight;
}

void UKUIItemDelegate::setImplicitHeight(int newImplicitHeight)
{
    if (m_implicitHeight == newImplicitHeight)
        return;
    m_implicitHeight = newImplicitHeight;
    emit implicitHeightChanged();
}

int UKUIItemDelegate::padding() const
{
    return m_padding;
}

void UKUIItemDelegate::setPadding(int newPadding)
{
    if (m_padding == newPadding)
        return;
    m_padding = newPadding;
    emit paddingChanged();
}

const QBrush &UKUIItemDelegate::normalHTextColor() const
{
    return m_normalHTextColor;
}

void UKUIItemDelegate::setNormalHTextColor(const QBrush &newNormalHTextColor)
{
    if (m_normalHTextColor == newNormalHTextColor)
        return;
    m_normalHTextColor = newNormalHTextColor;
    emit normalHTextColorChanged();
}

const QBrush &UKUIItemDelegate::disableHTextColor() const
{
    return m_disableHTextColor;
}

void UKUIItemDelegate::setDisableHTextColor(const QBrush &newDisableHTextColor)
{
    if (m_disableHTextColor == newDisableHTextColor)
        return;
    m_disableHTextColor = newDisableHTextColor;
    emit disableHTextColorChanged();
}

const QBrush &UKUIItemDelegate::normalTransparentBC() const
{
    return m_normalTransparentBC;
}

void UKUIItemDelegate::setNormalTransparentBC(const QBrush &newNormalTransparentBC)
{
    if (m_normalTransparentBC == newNormalTransparentBC)
        return;
    m_normalTransparentBC = newNormalTransparentBC;
    emit normalTransparentBCChanged();
}

const QBrush &UKUIItemDelegate::clickedTransparentBC() const
{
    return m_clickedTransparentBC;
}

void UKUIItemDelegate::setClickedTransparentBC(const QBrush &newClickedTransparentBC)
{
    if (m_clickedTransparentBC == newClickedTransparentBC)
        return;
    m_clickedTransparentBC = newClickedTransparentBC;
    emit clickedTransparentBCChanged();
}

const QBrush &UKUIItemDelegate::hoveredTransparentBC() const
{
    return m_hoveredTransparentBC;
}

void UKUIItemDelegate::setHoveredTransparentBC(const QBrush &newHoveredTransparentBC)
{
    if (m_hoveredTransparentBC == newHoveredTransparentBC)
        return;
    m_hoveredTransparentBC = newHoveredTransparentBC;
    emit hoveredTransparentBCChanged();
}

const QBrush &UKUIItemDelegate::disableTransparentBC() const
{
    return m_disableTransparentBC;
}

void UKUIItemDelegate::setDisableTransparentBC(const QBrush &newDisableTransparentBC)
{
    if (m_disableTransparentBC == newDisableTransparentBC)
        return;
    m_disableTransparentBC = newDisableTransparentBC;
    emit disableTransparentBCChanged();
}

const QBrush &UKUIItemDelegate::normalTransparentBorderColor() const
{
    return m_normalTransparentBorderColor;
}

void UKUIItemDelegate::setNormalTransparentBorderColor(const QBrush &newNormalTransparentBorderColor)
{
    if (m_normalTransparentBorderColor == newNormalTransparentBorderColor)
        return;
    m_normalTransparentBorderColor = newNormalTransparentBorderColor;
    emit normalTransparentBorderColorChanged();
}

const QBrush &UKUIItemDelegate::clickedTransparentBorderColor() const
{
    return m_clickedTransparentBorderColor;
}

void UKUIItemDelegate::setClickedTransparentBorderColor(const QBrush &newClickedTransparentBorderColor)
{
    if (m_clickedTransparentBorderColor == newClickedTransparentBorderColor)
        return;
    m_clickedTransparentBorderColor = newClickedTransparentBorderColor;
    emit clickedTransparentBorderColorChanged();
}

const QBrush &UKUIItemDelegate::hoveredTransparentBorderColor() const
{
    return m_hoveredTransparentBorderColor;
}

void UKUIItemDelegate::setHoveredTransparentBorderColor(const QBrush &newHoveredTransparentBorderColor)
{
    if (m_hoveredTransparentBorderColor == newHoveredTransparentBorderColor)
        return;
    m_hoveredTransparentBorderColor = newHoveredTransparentBorderColor;
    emit hoveredTransparentBorderColorChanged();
}

const QBrush &UKUIItemDelegate::disableTransparentBorderColor() const
{
    return m_disableTransparentBorderColor;
}

void UKUIItemDelegate::setDisableTransparentBorderColor(const QBrush &newDisableTransparentBorderColor)
{
    if (m_disableTransparentBorderColor == newDisableTransparentBorderColor)
        return;
    m_disableTransparentBorderColor = newDisableTransparentBorderColor;
    emit disableTransparentBorderColorChanged();
}

const QBrush &UKUIItemDelegate::disableHBC() const
{
    return m_disableHBC;
}

void UKUIItemDelegate::setDisableHBC(const QBrush &newDisableHBC)
{
    if (m_disableHBC == newDisableHBC)
        return;
    m_disableHBC = newDisableHBC;
    emit disabledHBCChanged();
}

const QBrush &UKUIItemDelegate::disableBorderHColor() const
{
    return m_disableBorderHColor;
}

void UKUIItemDelegate::setDisableBorderHColor(const QBrush &newDisableBorderHColor)
{
    if (m_disableBorderHColor == newDisableBorderHColor)
        return;
    m_disableBorderHColor = newDisableBorderHColor;
    emit disabledBorderHColorChanged();
}

const QBrush &UKUIItemDelegate::disableCheckedBC() const
{
    return m_disableCheckedBC;
}

void UKUIItemDelegate::setDisableCheckedBC(const QBrush &newDisableCheckedBC)
{
    if (m_disableCheckedBC == newDisableCheckedBC)
        return;
    m_disableCheckedBC = newDisableCheckedBC;
    emit disableCheckedBCChanged();
}

const QBrush &UKUIItemDelegate::disableBorderCheckedColor() const
{
    return m_disableBorderCheckedColor;
}

void UKUIItemDelegate::setDisableBorderCheckedColor(const QBrush &newDisableBorderCheckedColor)
{
    if (m_disableBorderCheckedColor == newDisableBorderCheckedColor)
        return;
    m_disableBorderCheckedColor = newDisableBorderCheckedColor;
    emit disableBorderCheckedColorChanged();
}

const QBrush &UKUIItemDelegate::normalCheckedTransparentBC() const
{
    return m_normalCheckedTransparentBC;
}

void UKUIItemDelegate::setNormalCheckedTransparentBC(const QBrush &newNormalCheckedTransparentBC)
{
    if (m_normalCheckedTransparentBC == newNormalCheckedTransparentBC)
        return;
    m_normalCheckedTransparentBC = newNormalCheckedTransparentBC;
    emit normalCheckedTransparentBCChanged();
}

const QBrush &UKUIItemDelegate::clickedCheckedTransparentBC() const
{
    return m_clickedCheckedTransparentBC;
}

void UKUIItemDelegate::setClickedCheckedTransparentBC(const QBrush &newClickedCheckedTransparentBC)
{
    if (m_clickedCheckedTransparentBC == newClickedCheckedTransparentBC)
        return;
    m_clickedCheckedTransparentBC = newClickedCheckedTransparentBC;
    emit clickedCheckedTransparentBCChanged();
}

const QBrush &UKUIItemDelegate::hoveredCheckedTransparentBC() const
{
    return m_hoveredCheckedTransparentBC;
}

void UKUIItemDelegate::setHoveredCheckedTransparentBC(const QBrush &newHoveredCheckedTransparentBC)
{
    if (m_hoveredCheckedTransparentBC == newHoveredCheckedTransparentBC)
        return;
    m_hoveredCheckedTransparentBC = newHoveredCheckedTransparentBC;
    emit hoveredCheckedTransparentBCChanged();
}

const QBrush &UKUIItemDelegate::disableCheckedTransparentBC() const
{
    return m_disableCheckedTransparentBC;
}

void UKUIItemDelegate::setDisableCheckedTransparentBC(const QBrush &newDisableCheckedTransparentBC)
{
    if (m_disableCheckedTransparentBC == newDisableCheckedTransparentBC)
        return;
    m_disableCheckedTransparentBC = newDisableCheckedTransparentBC;
    emit disableCheckedTransparentBCChanged();
}

const QBrush &UKUIItemDelegate::normalCheckedTransparentBorderColor() const
{
    return m_normalCheckedTransparentBorderColor;
}

void UKUIItemDelegate::setNormalCheckedTransparentBorderColor(const QBrush &newNormalCheckedTransparentBorderColor)
{
    if (m_normalCheckedTransparentBorderColor == newNormalCheckedTransparentBorderColor)
        return;
    m_normalCheckedTransparentBorderColor = newNormalCheckedTransparentBorderColor;
    emit normalCheckedTransparentBorderColorChanged();
}

const QBrush &UKUIItemDelegate::clickedCheckedTransparentBorderColor() const
{
    return m_clickedCheckedTransparentBorderColor;
}

void UKUIItemDelegate::setClickedCheckedTransparentBorderColor(const QBrush &newClickedCheckedTransparentBorderColor)
{
    if (m_clickedCheckedTransparentBorderColor == newClickedCheckedTransparentBorderColor)
        return;
    m_clickedCheckedTransparentBorderColor = newClickedCheckedTransparentBorderColor;
    emit clickedCheckedTransparentBorderColorChanged();
}

const QBrush &UKUIItemDelegate::hoveredCheckedTransparentBorderColor() const
{
    return m_hoveredCheckedTransparentBorderColor;
}

void UKUIItemDelegate::setHoveredCheckedTransparentBorderColor(const QBrush &newHoveredCheckedTransparentBorderColor)
{
    if (m_hoveredCheckedTransparentBorderColor == newHoveredCheckedTransparentBorderColor)
        return;
    m_hoveredCheckedTransparentBorderColor = newHoveredCheckedTransparentBorderColor;
    emit hoveredCheckedTransparentBorderColorChanged();
}

const QBrush &UKUIItemDelegate::disableCheckedTransparentBorderColor() const
{
    return m_disableCheckedTransparentBorderColor;
}

void UKUIItemDelegate::setDisableCheckedTransparentBorderColor(const QBrush &newDisableCheckedTransparentBorderColor)
{
    if (m_disableCheckedTransparentBorderColor == newDisableCheckedTransparentBorderColor)
        return;
    m_disableCheckedTransparentBorderColor = newDisableCheckedTransparentBorderColor;
    emit disableCheckedTransparentBorderColorChanged();
}

QBrush UKUIItemDelegate::normalAlternateBC() const
{
    return m_normalAlternateBC;
}

void UKUIItemDelegate::setNormalAlternateBC(const QBrush &newNormalAlternateBC)
{
    if (m_normalAlternateBC == newNormalAlternateBC)
        return;
    m_normalAlternateBC = newNormalAlternateBC;
    emit normalAlternateBCChanged();
}

QBrush UKUIItemDelegate::clickedAlternateBC() const
{
    return m_clickedAlternatedBC;
}

void UKUIItemDelegate::setClickedAlternateBC(const QBrush &newClickedAlternatedBC)
{
    if (m_clickedAlternatedBC == newClickedAlternatedBC)
        return;
    m_clickedAlternatedBC = newClickedAlternatedBC;
    emit clickedAlternateBCChanged();
}

QBrush UKUIItemDelegate::hoveredAlternateBC() const
{
    return m_hoveredAlternatedBC;
}

void UKUIItemDelegate::setHoveredAlternateBC(const QBrush &newHoveredAlternatedBC)
{
    if (m_hoveredAlternatedBC == newHoveredAlternatedBC)
        return;
    m_hoveredAlternatedBC = newHoveredAlternatedBC;
    emit hoveredAlternateBCChanged();
}

QBrush UKUIItemDelegate::disableAlternateBC() const
{
    return m_disableAlternateBC;
}

void UKUIItemDelegate::setDisableAlternateBC(const QBrush &newDisableAlternateBC)
{
    if (m_disableAlternateBC == newDisableAlternateBC)
        return;
    m_disableAlternateBC = newDisableAlternateBC;
    emit disableAlternateBCChanged();
}

QBrush UKUIItemDelegate::normalAlternateTransparentBC() const
{
    return m_normalAlternateTransparentBC;
}

void UKUIItemDelegate::setNormalAlternateTransparentBC(const QBrush &newNormalAlternateTransparentBC)
{
    if (m_normalAlternateTransparentBC == newNormalAlternateTransparentBC)
        return;
    m_normalAlternateTransparentBC = newNormalAlternateTransparentBC;
    emit normalAlternateTransparentBCChanged();
}

QBrush UKUIItemDelegate::clickedAlternateTransparentBC() const
{
    return m_clickedAlternateTransparentBC;
}

void UKUIItemDelegate::setClickedAlternateTransparentBC(const QBrush &newClickedAlternateTransparentBC)
{
    if (m_clickedAlternateTransparentBC == newClickedAlternateTransparentBC)
        return;
    m_clickedAlternateTransparentBC = newClickedAlternateTransparentBC;
    emit clickedAlternateTransparentBCChanged();
}

QBrush UKUIItemDelegate::hoveredAlternateTransparentBC() const
{
    return m_hoveredAlternateTransparentBC;
}

void UKUIItemDelegate::setHoveredAlternateTransparentBC(const QBrush &newHoveredAlternateTransparentBC)
{
    if (m_hoveredAlternateTransparentBC == newHoveredAlternateTransparentBC)
        return;
    m_hoveredAlternateTransparentBC = newHoveredAlternateTransparentBC;
    emit hoveredAlternateTransparentBCChanged();
}

QBrush UKUIItemDelegate::disableAlternateTransparentBC() const
{
    return m_disableAlternateTransparentBC;
}

void UKUIItemDelegate::setDisableAlternateTransparentBC(const QBrush &newDisableAlternateTransparentBC)
{
    if (m_disableAlternateTransparentBC == newDisableAlternateTransparentBC)
        return;
    m_disableAlternateTransparentBC = newDisableAlternateTransparentBC;
    emit disableAlternateTransparentBCChanged();
}
