#include <QApplication>
#include "ukuibutton.h"
#include "qdebug.h"
using namespace UKUIQQC2Style;

UKUIButton::UKUIButton(QQuickItem *parent) : QQuickItem(parent)
{
    {
        m_normalBC         = QBrush(QColor("#E6E6E6"));
        m_clickedBC        = QBrush(QColor("#B9B9B9"));
        m_hoveredBC        = QBrush(QColor("#DCDCDC"));
        m_disableBC        = QBrush(QColor("#EEEEEE"));
        m_disableTextColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.35));
        m_normalTextColor  = QBrush(QColor::fromRgbF(0, 0, 0, 0.85));
        m_normalHBC        = QBrush(QColor("#3790FA"));
        m_clickedHBC       = QBrush(QColor("#3790FA"));
        m_hoveredHBC       = QBrush(QColor("#3790FA"));
        m_disableHBC       = QBrush(QColor("#EEEEEE"));
        m_highlightTextColor = QBrush(QColor("#FFFFFF"));
        m_highlightTextDisableColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.35));
    }
    if(!qApp || !qApp->property("qqc2-globaltoken").isValid())
        return;
    TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
    if(token && token->getInstance()){
        m_instance = token->getInstance();
        initParam(m_instance);
        connect(m_instance, &UKUIGlobalDTConfig::GlobalDTConfig::tokenChanged, this, [=](){
            initParam(m_instance);
        }, Qt::UniqueConnection);

        connect(qApp, &QApplication::paletteChanged, this, [=](const QPalette &pal){
            initParam(m_instance);
        }, Qt::UniqueConnection);
    }
}

UKUIButton::~UKUIButton()
{

}

UKUIButton* UKUIButton::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIButton(p);
}

void UKUIButton::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalBC(instance->kComponentNormal());
    setClickedBC(instance->kComponentClick());
    setHoveredBC(instance->kComponentHover());
    setDisableBC(instance->kComponentDisable());

//    qDebug() << "kComponentHover...." << instance->kComponentHover().gradient();
//    if(instance->kComponentHover().gradient() && instance->kComponentHover().gradient()->stops().length() >= 2){
//        qDebug() << "second0" << instance->kComponentHover().gradient()->stops().at(0).second;
//        qDebug() << "second1" << instance->kComponentHover().gradient()->stops().at(1).second;

//    }
    setNormalHBC(instance->kBrandNormal());
    setHoveredHBC(instance->kBrandHover());
    setClickedHBC(instance->kBrandClick());
    setDisableHBC(instance->kBrandDisable());

    setNormalTextColor(instance->kFontPrimary());
    setDisableTextColor(instance->kFontPrimaryDisable());
    setHighlightTextColor(instance->kFontWhite());
    setHighlightTextDisableColor(instance->kFontWhiteDisable());

    setBorderWidth(1);
    setFocusBorderWidth(instance->focusline());
    setNormalBorderColor(instance->kLineComponentNormal());
    setClickBorderColor(instance->kLineComponentClick());
    setHoverBorderColor(instance->kLineComponentHover());
    setDisableBorderColor(instance->kLineComponentDisable());

    setNormalBorderHColor(instance->kLineComponentNormal());
    setClickBorderHColor(instance->kLineComponentClick());
    setHoverBorderHColor(instance->kLineComponentHover());
    setDisableBorderHColor(instance->kLineComponentDisable());
    setFocusBorderColor(instance->kBrandFocus());
    setRadius(instance->kradiusNormal());


    setNormalCloseBC(instance->kGrayAlpha0());
    setClickedCloseBC(instance->kErrorClick());
    setHoveredClosedBC(instance->kErrorHover());
    setDisableCloseBC(instance->kGrayAlpha0());
    setNormalBorderCloseColor(QBrush(QColor(0,0,0,0)));
    setHoverBorderCloseColor(QBrush(QColor(0,0,0,0)));
    setClickBorderCloseColor(QBrush(QColor(0,0,0,0)));
    setDisableBorderCloseColor(QBrush(QColor(0,0,0,0)));

    setNormalWindowBC(instance->kGrayAlpha0());
    setClickedWindowBC(instance->kComponentClick());
    setHoveredWindowBC(instance->kComponentHover());
    setDisableWindowBC(instance->kGrayAlpha0());
    setNormalBorderWindowColor(QBrush(QColor(0,0,0,0)));
    setHoverBorderWindowColor(QBrush(QColor(0,0,0,0)));
    setClickBorderWindowColor(QBrush(QColor(0,0,0,0)));
    setDisableBorderWindowColor(QBrush(QColor(0,0,0,0)));

    setNormalTransparentBC(instance->kComponentAlphaNormal());
    setHoveredTransparentBC(instance->kComponentAlphaHover());
    setClickedTransparentBC(instance->kComponentAlphaClick());
    setDisableTransparentBC(instance->kComponentAlphaDisable());
    setNormalBorderTransparentBC(instance->kLineComponentNormal());
    setHoveredBorderTransparentBC(instance->kLineComponentHover());
    setClickedBorderTransparentBC(instance->kLineComponentClick());
    setDisableBorderTransparentBC(instance->kLineComponentDisable());

    setNormalWindowTransparentBC(instance->kGrayAlpha0());
   setClickedWindowTransparentBC(instance->kComponentAlphaClick());
   setHoveredWindowTransparentBC(instance->kComponentAlphaHover());
   setDisableWindowTransparentBC(instance->kGrayAlpha0());
    setNormalBorderWindowTransparentColor(QBrush(QColor(0,0,0,0)));
   setHoveredBorderWindowTransparentColor(QBrush(QColor(0,0,0,0)));
   setClickedBorderWindowTransparentColor(QBrush(QColor(0,0,0,0)));
   setDisableBorderWindowTransparentColor(QBrush(QColor(0,0,0,0)));
    emit parametryChanged();
//    qDebug() << "instance->highLightActive()..." << m_normalHBC.red() << m_normalHBC.green() << m_normalHBC.blue();
//    qDebug() << "instance->setHoveredHBC()..." << m_hoveredHBC.red() << m_hoveredHBC.green() << m_hoveredHBC.blue();
}


double UKUIButton::radius() const
{
    return m_radius;
}

void UKUIButton::setRadius(double newRadius)
{
    if (m_radius == newRadius)
        return;
    m_radius = newRadius;
    emit radiusChanged();
}

double UKUIButton::leftRightMargin() const
{
    return m_leftRightMargin;
}

void UKUIButton::setLeftRightMargin(double newMargin)
{
    if (m_leftRightMargin == newMargin)
        return;
    m_leftRightMargin = newMargin;
    emit leftRightMarginChanged();
}

double UKUIButton::upDownMargin() const
{
    return m_upDownMargin;
}

void UKUIButton::setUpDownMargin(double newUpDownMargin)
{
    if (m_upDownMargin == newUpDownMargin)
        return;
    m_upDownMargin = newUpDownMargin;
    emit upDownMarginChanged();
}

double UKUIButton::space() const
{
    return m_space;
}

void UKUIButton::setSpace(double newSpace)
{
    if (qFuzzyCompare(m_space, newSpace))
        return;
    m_space = newSpace;
    emit spaceChanged();
}

int UKUIButton::normalWidth() const
{
    return m_normalWidth;
}

void UKUIButton::setNormalWidth(int newNormalWidth)
{
    if (m_normalWidth == newNormalWidth)
        return;
    m_normalWidth = newNormalWidth;
    emit normalWidthChanged();
}

int UKUIButton::normalHeight() const
{
    return m_normalHeight;
}

void UKUIButton::setNormalHeight(int newNormalHeight)
{
    if (m_normalHeight == newNormalHeight)
        return;
    m_normalHeight = newNormalHeight;
    emit normalHeightChanged();
}

QBrush UKUIButton::normalBC() const
{
    return m_normalBC;
}

void UKUIButton::setNormalBC(QBrush newNormalBC)
{
    if (m_normalBC != newNormalBC)
    {
        m_normalBC = newNormalBC;
        emit normalBCChanged();
    }
}

QBrush UKUIButton::clickedBC() const
{
    return m_clickedBC;
}

void UKUIButton::setClickedBC(QBrush newClickedBC)
{
    if (m_clickedBC != newClickedBC){
    m_clickedBC = newClickedBC;
    emit clickedBCChanged();
    }
}

QBrush UKUIButton::hoveredBC() const
{
    return m_hoveredBC;
}

void UKUIButton::setHoveredBC(QBrush newHoveredBC)
{
    if (m_hoveredBC != newHoveredBC){
        m_hoveredBC = newHoveredBC;
        emit hoveredBCChanged();
    }
}

QBrush UKUIButton::disableBC() const
{
    return m_disableBC;
}

void UKUIButton::setDisableBC(QBrush newDisableBC)
{
    if (m_disableBC != newDisableBC){
        m_disableBC = newDisableBC;
        emit disableBCChanged();
    }
}

QBrush UKUIButton::disableTextColor() const
{
    return m_disableTextColor;
}

void UKUIButton::setDisableTextColor(QBrush newDisableTextColor)
{
    if (m_disableTextColor != newDisableTextColor){
    m_disableTextColor = newDisableTextColor;
    emit disableTextColorChanged();
    }
}

QBrush UKUIButton::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUIButton::setNormalTextColor(QBrush newNormalTextColor)
{
    if (m_normalTextColor != newNormalTextColor){
    m_normalTextColor = newNormalTextColor;
    emit normalTextColorChanged();
    }
}

QBrush UKUIButton::normalHBC() const
{
    return m_normalHBC;
}

void UKUIButton::setNormalHBC(QBrush newNormalHBC)
{
    if (m_normalHBC != newNormalHBC){
    m_normalHBC = newNormalHBC;
    emit normalHBCChanged();
    }
}

QBrush UKUIButton::clickedHBC() const
{
    return m_clickedHBC;
}

void UKUIButton::setClickedHBC(QBrush newClickedHBC)
{
    if (m_clickedHBC != newClickedHBC){
    m_clickedHBC = newClickedHBC;
    emit clickedHBCChanged();
    }
}

QBrush UKUIButton::hoveredHBC()
{
    return m_hoveredHBC;
}

void UKUIButton::setHoveredHBC(QBrush newHoveredHBC)
{
    if (m_hoveredHBC != newHoveredHBC){
    m_hoveredHBC = newHoveredHBC;
    emit hoveredHBCChanged();
    }
}

QBrush UKUIButton::disableHBC() const
{
    return m_disableHBC;
}

void UKUIButton::setDisableHBC(QBrush newDisableHBC)
{
    if (m_disableHBC != newDisableHBC){
    m_disableHBC = newDisableHBC;
    emit disableHBCChanged();
    }
}

int UKUIButton::borderWidth() const
{
    return m_borderWidth;
}

void UKUIButton::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

QBrush UKUIButton::normalBorderColor() const
{
    return m_normalBorderColor;
}

void UKUIButton::setNormalBorderColor(QBrush newNormalBorderColor)
{
    if (m_normalBorderColor != newNormalBorderColor){
    m_normalBorderColor = newNormalBorderColor;
    emit normalBorderColorChanged();
    }
}

QBrush UKUIButton::hoverBorderColor() const
{
    return m_hoverBorderColor;
}

void UKUIButton::setHoverBorderColor(QBrush newHoverBorderColor)
{
    if (m_hoverBorderColor != newHoverBorderColor){
    m_hoverBorderColor = newHoverBorderColor;
    emit hoverBorderColorChanged();
    }
}

QBrush UKUIButton::clickBorderColor() const
{
    return m_clickBorderColor;
}

void UKUIButton::setClickBorderColor(QBrush newClickBorderColor)
{
    if (m_clickBorderColor!= newClickBorderColor){
    m_clickBorderColor = newClickBorderColor;
    emit clickBorderColorChanged();
    }
}

QBrush UKUIButton::disableBorderColor() const
{
    return m_disableBorderColor;
}

void UKUIButton::setDisableBorderColor(QBrush newDisableBorderColor)
{
    if (m_disableBorderColor != newDisableBorderColor){
    m_disableBorderColor = newDisableBorderColor;
    emit disableBorderColorChanged();
    }
}

QBrush UKUIButton::normalBorderHColor() const
{
    return m_normalBorderHColor;
}

void UKUIButton::setNormalBorderHColor(QBrush newNormalBorderHColor)
{
    if (m_normalBorderHColor != newNormalBorderHColor){
    m_normalBorderHColor = newNormalBorderHColor;
    emit normalBorderHColorChanged();
    }
}

QBrush UKUIButton::hoverBorderHColor() const
{
    return m_hoverBorderHColor;
}

void UKUIButton::setHoverBorderHColor(QBrush newHoverBorderHColor)
{
    if (m_hoverBorderHColor != newHoverBorderHColor){
    m_hoverBorderHColor = newHoverBorderHColor;
    emit hoverBorderHColorChanged();
    }
}

QBrush UKUIButton::clickBorderHColor() const
{
    return m_clickBorderHColor;
}

void UKUIButton::setClickBorderHColor(QBrush newClickBorderHColor)
{
    if (m_clickBorderHColor != newClickBorderHColor){
    m_clickBorderHColor = newClickBorderHColor;
    emit clickBorderHColorChanged();
    }
}

QBrush UKUIButton::disableBorderHColor() const
{
    return m_disableBorderHColor;
}

void UKUIButton::setDisableBorderHColor(QBrush newDisableBorderHColor)
{
    if (m_disableBorderHColor != newDisableBorderHColor){
    m_disableBorderHColor = newDisableBorderHColor;
    emit disableBorderHColorChanged();
    }
}

QBrush UKUIButton::focusBorderColor() const
{
    return m_focusBorderColor;
}

void UKUIButton::setFocusBorderColor(QBrush newFocusBorderColor)
{
    if (m_focusBorderColor != newFocusBorderColor){
    m_focusBorderColor = newFocusBorderColor;
    emit focusBorderColorChanged();
    }
}

int UKUIButton::focusBorderWidth() const
{
    return m_focusBorderWidth;
}

void UKUIButton::setFocusBorderWidth(int newFocusBorderWidth)
{
    if (m_focusBorderWidth == newFocusBorderWidth)
        return;
    m_focusBorderWidth = newFocusBorderWidth;
    emit focusBorderWidthChanged();
}

QBrush UKUIButton::highlightTextColor() const
{
    return m_highlightTextColor;
}

void UKUIButton::setHighlightTextColor(QBrush newHighlightTextColor)
{
    if (m_highlightTextColor == newHighlightTextColor)
        return;
    m_highlightTextColor = newHighlightTextColor;
    emit highlightTextColorChanged();
}

QBrush UKUIButton::highlightTextDisableColor() const
{
    return m_highlightTextDisableColor;
}

void UKUIButton::setHighlightTextDisableColor(QBrush newHighlightTextDisableColor)
{
    if (m_highlightTextDisableColor == newHighlightTextDisableColor)
        return;
    m_highlightTextDisableColor = newHighlightTextDisableColor;
    emit highlightTextDisableColorChanged();
}

const QBrush &UKUIButton::normalCloseBC() const
{
    return m_normalCloseBC;
}

void UKUIButton::setNormalCloseBC(const QBrush &newNormalCloseBC)
{
    if (m_normalCloseBC == newNormalCloseBC)
        return;
    m_normalCloseBC = newNormalCloseBC;
    emit normalCloseBCChanged();
}

const QBrush &UKUIButton::clickedCloseBC() const
{
    return m_clickedCloseBC;
}

void UKUIButton::setClickedCloseBC(const QBrush &newClickedCloseBC)
{
    if (m_clickedCloseBC == newClickedCloseBC)
        return;
    m_clickedCloseBC = newClickedCloseBC;
    emit clickedCloseBCChanged();
}

const QBrush &UKUIButton::hoveredCloseBC() const
{
    return m_hoveredCloseBC;
}

void UKUIButton::setHoveredClosedBC(const QBrush &newHoveredCloseBC)
{
    if (m_hoveredCloseBC == newHoveredCloseBC)
        return;
    m_hoveredCloseBC = newHoveredCloseBC;
    emit hoveredCloseBCChanged();
}

const QBrush &UKUIButton::disableCloseBC() const
{
    return m_disableCloseBC;
}

void UKUIButton::setDisableCloseBC(const QBrush &newDisableCloseBC)
{
    if (m_disableCloseBC == newDisableCloseBC)
        return;
    m_disableCloseBC = newDisableCloseBC;
    emit disableCloseBCChanged();
}

const QBrush &UKUIButton::normalBorderCloseColor() const
{
    return m_normalBorderCloseColor;
}

void UKUIButton::setNormalBorderCloseColor(const QBrush &newNormalBorderCloseColor)
{
    if (m_normalBorderCloseColor == newNormalBorderCloseColor)
        return;
    m_normalBorderCloseColor = newNormalBorderCloseColor;
    emit normalBorderCloseColorChanged();
}

const QBrush &UKUIButton::hoverBorderCloseColor() const
{
    return m_hoverBorderCloseColor;
}

void UKUIButton::setHoverBorderCloseColor(const QBrush &newHoverBorderCloseColor)
{
    if (m_hoverBorderCloseColor == newHoverBorderCloseColor)
        return;
    m_hoverBorderCloseColor = newHoverBorderCloseColor;
    emit hoverBorderCloseColorChanged();
}

const QBrush &UKUIButton::clickBorderCloseColor() const
{
    return m_clickBorderCloseColor;
}

void UKUIButton::setClickBorderCloseColor(const QBrush &newClickBorderCloseColor)
{
    if (m_clickBorderCloseColor == newClickBorderCloseColor)
        return;
    m_clickBorderCloseColor = newClickBorderCloseColor;
    emit clickBorderCloseColorChanged();
}

const QBrush &UKUIButton::disableBorderCloseColor() const
{
    return m_disableBorderCloseColor;
}

void UKUIButton::setDisableBorderCloseColor(const QBrush &newDisableBorderCloseColor)
{
    if (m_disableBorderCloseColor == newDisableBorderCloseColor)
        return;
    m_disableBorderCloseColor = newDisableBorderCloseColor;
    emit disableBorderCloseColorChanged();
}

const QBrush &UKUIButton::normalWindowBC() const
{
    return m_normalWindowBC;
}

void UKUIButton::setNormalWindowBC(const QBrush &newNormalWindowBC)
{
    if (m_normalWindowBC == newNormalWindowBC)
        return;
    m_normalWindowBC = newNormalWindowBC;
    emit normalWindowBCChanged();
}

const QBrush &UKUIButton::clickedWindowBC() const
{
    return m_clickedWindowBC;
}

void UKUIButton::setClickedWindowBC(const QBrush &newClickedWindowBC)
{
    if (m_clickedWindowBC == newClickedWindowBC)
        return;
    m_clickedWindowBC = newClickedWindowBC;
    emit clickedWindowBCChanged();
}

const QBrush &UKUIButton::hoveredWindowBC() const
{
    return m_hoveredWindowBC;
}

void UKUIButton::setHoveredWindowBC(const QBrush &newHoveredWindowBC)
{
    if (m_hoveredWindowBC == newHoveredWindowBC)
        return;
    m_hoveredWindowBC = newHoveredWindowBC;
    emit hoveredWindowBCChanged();
}

const QBrush &UKUIButton::disableWindowBC() const
{
    return m_disableWindowBC;
}

void UKUIButton::setDisableWindowBC(const QBrush &newDisableWindowBC)
{
    if (m_disableWindowBC == newDisableWindowBC)
        return;
    m_disableWindowBC = newDisableWindowBC;
    emit disableWindowBCChanged();
}

const QBrush &UKUIButton::normalBorderWindowColor() const
{
    return m_normalBorderWindowColor;
}

void UKUIButton::setNormalBorderWindowColor(const QBrush &newNormalBorderWindowColor)
{
    if (m_normalBorderWindowColor == newNormalBorderWindowColor)
        return;
    m_normalBorderWindowColor = newNormalBorderWindowColor;
    emit normalBorderWindowColorChanged();
}

const QBrush &UKUIButton::hoverBorderWindowColor() const
{
    return m_hoverBorderWindowColor;
}

void UKUIButton::setHoverBorderWindowColor(const QBrush &newHoverBorderWindowColor)
{
    if (m_hoverBorderWindowColor == newHoverBorderWindowColor)
        return;
    m_hoverBorderWindowColor = newHoverBorderWindowColor;
    emit hoverBorderWindowColorChanged();
}

const QBrush &UKUIButton::clickBorderWindowColor() const
{
    return m_clickBorderWindowColor;
}

void UKUIButton::setClickBorderWindowColor(const QBrush &newClickBorderWindowColor)
{
    if (m_clickBorderWindowColor == newClickBorderWindowColor)
        return;
    m_clickBorderWindowColor = newClickBorderWindowColor;
    emit clickBorderWindowColorChanged();
}

const QBrush &UKUIButton::disableBorderWindowColor() const
{
    return m_disableBorderWindowColor;
}

void UKUIButton::setDisableBorderWindowColor(const QBrush &newDisableBorderWindowColor)
{
    if (m_disableBorderWindowColor == newDisableBorderWindowColor)
        return;
    m_disableBorderWindowColor = newDisableBorderWindowColor;
    emit disableBorderWindowColorChanged();
}

const QBrush &UKUIButton::normalTransparentBC() const
{
    return m_normalTransparentBC;
}

void UKUIButton::setNormalTransparentBC(const QBrush &newNormalTransparentBC)
{
    if (m_normalTransparentBC == newNormalTransparentBC)
        return;
    m_normalTransparentBC = newNormalTransparentBC;
    emit normalTransparentBCChanged();
}

const QBrush &UKUIButton::clickedTransparentBC() const
{
    return m_clickedTransparentBC;
}

void UKUIButton::setClickedTransparentBC(const QBrush &newClickedTransparentBC)
{
    if (m_clickedTransparentBC == newClickedTransparentBC)
        return;
    m_clickedTransparentBC = newClickedTransparentBC;
    emit clickedTransparentBCChanged();
}

const QBrush &UKUIButton::hoveredTransparentBC() const
{
    return m_hoveredTransparentBC;
}

void UKUIButton::setHoveredTransparentBC(const QBrush &newHoveredTransparentBC)
{
    if (m_hoveredTransparentBC == newHoveredTransparentBC)
        return;
    m_hoveredTransparentBC = newHoveredTransparentBC;
    emit hoveredTransparentBCChanged();
}

const QBrush &UKUIButton::disableTransparentBC() const
{
    return m_disableTransparentBC;
}

void UKUIButton::setDisableTransparentBC(const QBrush &newDisableTransparentBC)
{
    if (m_disableTransparentBC == newDisableTransparentBC)
        return;
    m_disableTransparentBC = newDisableTransparentBC;
    emit disableTransparentBCChanged();
}

const QBrush &UKUIButton::normalBorderTransparentBC() const
{
    return m_normalBorderTransparentBC;
}

void UKUIButton::setNormalBorderTransparentBC(const QBrush &newNormalBorderTransparentBC)
{
    if (m_normalBorderTransparentBC == newNormalBorderTransparentBC)
        return;
    m_normalBorderTransparentBC = newNormalBorderTransparentBC;
    emit normalBorderTransparentBCChanged();
}

const QBrush &UKUIButton::clickedBorderTransparentBC() const
{
    return m_clickedBorderTransparentBC;
}

void UKUIButton::setClickedBorderTransparentBC(const QBrush &newClickedBorderTransparentBC)
{
    if (m_clickedBorderTransparentBC == newClickedBorderTransparentBC)
        return;
    m_clickedBorderTransparentBC = newClickedBorderTransparentBC;
    emit clickedBorderTransparentBCChanged();
}

const QBrush &UKUIButton::hoveredBorderTransparentBC() const
{
    return m_hoveredBorderTransparentBC;
}

void UKUIButton::setHoveredBorderTransparentBC(const QBrush &newHoveredBorderTransparentBC)
{
    if (m_hoveredBorderTransparentBC == newHoveredBorderTransparentBC)
        return;
    m_hoveredBorderTransparentBC = newHoveredBorderTransparentBC;
    emit hoveredBorderTransparentBCChanged();
}

const QBrush &UKUIButton::disableBorderTransparentBC() const
{
    return m_disableBorderTransparentBC;
}

void UKUIButton::setDisableBorderTransparentBC(const QBrush &newDisableBorderTransparentBC)
{
    if (m_disableBorderTransparentBC == newDisableBorderTransparentBC)
        return;
    m_disableBorderTransparentBC = newDisableBorderTransparentBC;
    emit disableBorderTransparentBCChanged();
}

const QBrush &UKUIButton::normalWindowTransparentBC() const
{
    return m_normalWindowTransparentBC;
}

void UKUIButton::setNormalWindowTransparentBC(const QBrush &newNormalWindowTransparentBC)
{
    if (m_normalWindowTransparentBC == newNormalWindowTransparentBC)
        return;
    m_normalWindowTransparentBC = newNormalWindowTransparentBC;
    emit normalWindowTransparentBCChanged();
}

const QBrush &UKUIButton::clickedWindowTransparentBC() const
{
    return m_clickedWindowTransparentBC;
}

void UKUIButton::setClickedWindowTransparentBC(const QBrush &newClickedWindowTransparentBC)
{
    if (m_clickedWindowTransparentBC == newClickedWindowTransparentBC)
        return;
    m_clickedWindowTransparentBC = newClickedWindowTransparentBC;
    emit clickedWindowTransparentBCChanged();
}

const QBrush &UKUIButton::hoveredWindowTransparentBC() const
{
    return m_hoveredWindowTransparentBC;
}

void UKUIButton::setHoveredWindowTransparentBC(const QBrush &newHoveredWindowTransparentBC)
{
    if (m_hoveredWindowTransparentBC == newHoveredWindowTransparentBC)
        return;
    m_hoveredWindowTransparentBC = newHoveredWindowTransparentBC;
    emit hoveredWindowTransparentBCChanged();
}

const QBrush &UKUIButton::disableWindowTransparentBC() const
{
    return m_disableWindowTransparentBC;
}

void UKUIButton::setDisableWindowTransparentBC(const QBrush &newDisableWindowTransparentBC)
{
    if (m_disableWindowTransparentBC == newDisableWindowTransparentBC)
        return;
    m_disableWindowTransparentBC = newDisableWindowTransparentBC;
    emit disableWindowTransparentBCChanged();
}

const QBrush &UKUIButton::normalBorderWindowTransparentColor() const
{
    return m_normalBorderWindowTransparentColor;
}

void UKUIButton::setNormalBorderWindowTransparentColor(const QBrush &newNormalBorderWindowTransparentColor)
{
    if (m_normalBorderWindowTransparentColor == newNormalBorderWindowTransparentColor)
        return;
    m_normalBorderWindowTransparentColor = newNormalBorderWindowTransparentColor;
    emit normalBorderWindowTransparentColorChanged();
}

const QBrush &UKUIButton::hoveredBorderWindowTransparentColor() const
{
    return m_hoveredBorderWindowTransparentColor;
}

void UKUIButton::setHoveredBorderWindowTransparentColor(const QBrush &newHoveredBorderWindowTransparentColor)
{
    if (m_hoveredBorderWindowTransparentColor == newHoveredBorderWindowTransparentColor)
        return;
    m_hoveredBorderWindowTransparentColor = newHoveredBorderWindowTransparentColor;
    emit hoveredBorderWindowTransparentColorChanged();
}

const QBrush &UKUIButton::clickedBorderWindowTransparentColor() const
{
    return m_clickedBorderWindowTransparentColor;
}

void UKUIButton::setClickedBorderWindowTransparentColor(const QBrush &newClickedBorderWindowTransparentColor)
{
    if (m_clickedBorderWindowTransparentColor == newClickedBorderWindowTransparentColor)
        return;
    m_clickedBorderWindowTransparentColor = newClickedBorderWindowTransparentColor;
    emit clickedBorderWindowTransparentColorChanged();
}

const QBrush &UKUIButton::disableBorderWindowTransparentColor() const
{
    return m_disableBorderWindowTransparentColor;
}

void UKUIButton::setDisableBorderWindowTransparentColor(const QBrush &newDisableBorderWindowTransparentColor)
{
    if (m_disableBorderWindowTransparentColor == newDisableBorderWindowTransparentColor)
        return;
    m_disableBorderWindowTransparentColor = newDisableBorderWindowTransparentColor;
    emit disableBorderWindowTransparentColorChanged();
}
