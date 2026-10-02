#include "ukuitoken.h"
#include "qdebug.h"
#include "settings/ukui-style-settings.h"

using namespace UKUIQQC2Style;

UKUIToken::UKUIToken(QQuickItem *parent) : QQuickItem(parent)
{
    if(!qApp || !qApp->property("qqc2-globaltoken").isValid())
        return;
    TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
    if(token && token->getInstance()){
        m_instance = token->getInstance();
        initDTBrush(m_instance);
        connect(m_instance, &UKUIGlobalDTConfig::GlobalDTConfig::tokenChanged, this, [=](){
            initDTBrush(m_instance);
        }, Qt::UniqueConnection);
    }
}

UKUIToken::~UKUIToken()
{

}

UKUIToken *UKUIToken::qmlAttachedProperties(QObject *parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIToken(p);
}

// ========== 从GlobalDTConfig初始化所有属性 ==========
void UKUIToken::initDTBrush(UKUIGlobalDTConfig::GlobalDTConfig *instance)
{
    if (!instance) return;

    // 基础调色板属性
    m_alternateBaseActive = instance->alternateBaseActive();
    m_alternateBaseDisable = instance->alternateBaseDisable();
    m_alternateBaseInactive = instance->alternateBaseInactive();
    m_baseActive = instance->baseActive();
    m_baseDisable = instance->baseDisable();
    m_baseInactive = instance->baseInactive();
    m_brightTextActive = instance->brightTextActive();
    m_brightTextDisable = instance->brightTextDisable();
    m_brightTextInactive = instance->brightTextInactive();
    m_buttonActive = instance->buttonActive();
    m_buttonDisable = instance->buttonDisable();
    m_buttonInactive = instance->buttonInactive();
    m_buttonTextActive = instance->buttonTextActive();
    m_buttonTextDisable = instance->buttonTextDisable();
    m_buttonTextInactive = instance->buttonTextInactive();
    m_darkActive = instance->darkActive();
    m_darkDisable = instance->darkDisable();
    m_darkInactive = instance->darkInactive();
    m_focusline = instance->focusline();
    m_highLightActive = instance->highLightActive();
    m_highLightDisable = instance->highLightDisable();
    m_highLightInactive = instance->highLightInactive();
    m_highLightedTextActive = instance->highLightedTextActive();
    m_highLightedTextDisable = instance->highLightedTextDisable();
    m_highLightedTextInactive = instance->highLightedTextInactive();

    // 品牌色/组件色属性
    m_kBlack = instance->kBlack();
    m_kBrand1 = instance->kBrand1();
    m_kBrand2 = instance->kBrand2();
    m_kBrand3 = instance->kBrand3();
    m_kBrand4 = instance->kBrand4();
    m_kBrand5 = instance->kBrand5();
    m_kBrand6 = instance->kBrand6();
    m_kBrand7 = instance->kBrand7();
    m_kBrand8 = instance->kBrand8();
    m_kBrand9 = instance->kBrand9();
    m_kBrandClick = instance->kBrandClick();
    m_kBrandDisable = instance->kBrandDisable();
    m_kBrandFocus = instance->kBrandFocus();
    m_kBrandHover = instance->kBrandHover();
    m_kBrandNormal = instance->kBrandNormal();
    m_kComponentAlphaClick = instance->kComponentAlphaClick();
    m_kComponentAlphaDisable = instance->kComponentAlphaDisable();
    m_kComponentAlphaHover = instance->kComponentAlphaHover();
    m_kComponentAlphaNormal = instance->kComponentAlphaNormal();
    m_kComponentClick = instance->kComponentClick();
    m_kComponentDisable = instance->kComponentDisable();
    m_kComponentHover = instance->kComponentHover();
    m_kComponentNormal = instance->kComponentNormal();
    m_kComponentSelectedAlphaDisable = instance->kComponentSelectedAlphaDisable();
    m_kComponentSelectedAlphaNormal = instance->kComponentSelectedAlphaNormal();
    m_kComponentSelectedAlphaHover = instance->kComponentSelectedAlphaHover();
    m_kComponentSelectedAlphaClick = instance->kComponentSelectedAlphaClick();
    m_kComponentInput = instance->kComponentInput();
    m_kComponentInputAlpha = instance->kComponentInputAlpha();
    m_kComponentSelectedDisable = instance->kComponentSelectedDisable();
    m_kComponentSelectedNormal = instance->kComponentSelectedNormal();
    m_kComponentSelectedHover = instance->kComponentSelectedHover();
    m_kComponentSelectedClick = instance->kComponentSelectedClick();

    // 容器/分割线属性
    m_kContainAlphaClick = instance->kContainAlphaClick();
    m_kContainAlphaHover = instance->kContainAlphaHover();
    m_kContainClick = instance->kContainClick();
    m_kContainGeneralAlphaNormal = instance->kContainGeneralAlphaNormal();
    m_kContainGeneralInactive = instance->kContainGeneralInactive();
    m_kContainGeneralNormal = instance->kContainGeneralNormal();
    m_kContainHover = instance->kContainHover();
    m_kContainSecondaryAlphaNormal = instance->kContainSecondaryAlphaNormal();
    m_kContainSecondaryInactive = instance->kContainSecondaryInactive();
    m_kContainSecondaryNormal = instance->kContainSecondaryNormal();
    m_kDivider = instance->kDivider();
    m_kDividerWhite = instance->kDividerWhite();

    // 状态色属性
    m_kErrorClick = instance->kErrorClick();
    m_kErrorDisable = instance->kErrorDisable();
    m_kErrorHover = instance->kErrorHover();
    m_kErrorNormal = instance->kErrorNormal();
    m_kSuccessClick = instance->kSuccessClick();
    m_kSuccessDisable = instance->kSuccessDisable();
    m_kSuccessHover = instance->kSuccessHover();
    m_kSuccessNormal = instance->kSuccessNormal();
    m_kWarningClick = instance->kWarningClick();
    m_kWarningDisable = instance->kWarningDisable();
    m_kWarningHover = instance->kWarningHover();
    m_kWarningNormal = instance->kWarningNormal();

    // 字体色属性
    m_kFontPlaceholdertextDisable = instance->kFontPlaceholdertextDisable();
    m_kFontPlaceholdertext = instance->kFontPlaceholdertext();
    m_kFontPrimaryDisable = instance->kFontPrimaryDisable();
    m_kFontPrimary = instance->kFontPrimary();
    m_kFontSecondaryDisable = instance->kFontSecondaryDisable();
    m_kFontSecondary = instance->kFontSecondary();
    m_kFontStrong = instance->kFontStrong();
    m_kFontWhiteDisable = instance->kFontWhiteDisable();
    m_kFontWhitePlaceholdertextDisable = instance->kFontWhitePlaceholdertextDisable();
    m_kFontWhiteplaceholdertext = instance->kFontWhiteplaceholdertext();
    m_kFontWhite = instance->kFontWhite();
    m_kFontWhiteSecondaryDisable = instance->kFontWhiteSecondaryDisable();
    m_kFontWhiteSecondary = instance->kFontWhiteSecondary();

    // 灰色系属性
    m_kGray0 = instance->kGray0();
    m_kGray1 = instance->kGray1();
    m_kGray2 = instance->kGray2();
    m_kGray3 = instance->kGray3();
    m_kGray4 = instance->kGray4();
    m_kGray5 = instance->kGray5();
    m_kGray6 = instance->kGray6();
    m_kGray7 = instance->kGray7();
    m_kGray8 = instance->kGray8();
    m_kGray9 = instance->kGray9();
    m_kGray10 = instance->kGray10();
    m_kGray11 = instance->kGray11();
    m_kGray12 = instance->kGray12();
    m_kGray13 = instance->kGray13();
    m_kGray14 = instance->kGray14();
    m_kGray15 = instance->kGray15();
    m_kGray16 = instance->kGray16();
    m_kGray17 = instance->kGray17();
    m_kGrayAlpha0 = instance->kGrayAlpha0();
    m_kGrayAlpha1 = instance->kGrayAlpha1();
    m_kGrayAlpha2 = instance->kGrayAlpha2();
    m_kGrayAlpha3 = instance->kGrayAlpha3();
    m_kGrayAlpha4 = instance->kGrayAlpha4();
    m_kGrayAlpha5 = instance->kGrayAlpha5();
    m_kGrayAlpha6 = instance->kGrayAlpha6();
    m_kGrayAlpha7 = instance->kGrayAlpha7();
    m_kGrayAlpha8 = instance->kGrayAlpha8();
    m_kGrayAlpha9 = instance->kGrayAlpha9();
    m_kGrayAlpha10 = instance->kGrayAlpha10();
    m_kGrayAlpha11 = instance->kGrayAlpha11();
    m_kGrayAlpha12 = instance->kGrayAlpha12();
    m_kGrayAlpha13 = instance->kGrayAlpha13();

    // 特殊色/线条属性
    m_kGreen1 = instance->kGreen1();
    m_kOrange1 = instance->kOrange1();
    m_kRed1 = instance->kRed1();
    m_kLineBrandClick = instance->kLineBrandClick();
    m_kLineBrandDisable = instance->kLineBrandDisable();
    m_kLineBrandHover = instance->kLineBrandHover();
    m_kLineBrandNormal = instance->kLineBrandNormal();
    m_kLineComponentClick = instance->kLineComponentClick();
    m_kLineComponentDisable = instance->kLineComponentDisable();
    m_kLineComponentHover = instance->kLineComponentHover();
    m_kLineComponentNormal = instance->kLineComponentNormal();
    m_kLineDisable = instance->kLineDisable();
    m_kLineDock = instance->kLineDock();
    m_kLineErrorClick = instance->kLineErrorClick();
    m_kLineErrorDisable = instance->kLineErrorDisable();
    m_kLineErrorHover = instance->kLineErrorHover();
    m_kLineErrorNormal = instance->kLineErrorNormal();
    m_kLineInputClick = instance->kLineInputClick();
    m_kLineInputDisable = instance->kLineInputDisable();
    m_kLineInputHover = instance->kLineInputHover();
    m_kLineInputNormal = instance->kLineInputNormal();
    m_kLineMenu = instance->kLineMenu();
    m_kLineNormal = instance->kLineNormal();
    m_kLineSelectboxClick = instance->kLineSelectboxClick();
    m_kLineSelectboxDisable = instance->kLineSelectboxDisable();
    m_kLineSelectboxHover = instance->kLineSelectboxHover();
    m_kLineSelectboxNormal = instance->kLineSelectboxNormal();
    m_kLineSelectboxSelected = instance->kLineSelectboxSelected();
    m_kLineSuccessClick = instance->kLineSuccessClick();
    m_kLineSuccessDisable = instance->kLineSuccessDisable();
    m_kLineSuccessHover = instance->kLineSuccessHover();
    m_kLineSuccessNormal = instance->kLineSuccessNormal();
    m_kLineTable = instance->kLineTable();
    m_kLineWarningClick = instance->kLineWarningClick();
    m_kLineWarningDisable = instance->kLineWarningDisable();
    m_kLineWarningHover = instance->kLineWarningHover();
    m_kLineWarningNormal = instance->kLineWarningNormal();
    m_kLineWindowActive = instance->kLineWindowActive();
    m_kLineWindowInactive = instance->kLineWindowInactive();

    // 菜单/阴影属性
    m_kMenu = instance->kMenu();
    m_kModalmask = instance->kModalmask();
    m_kSelectAlphaWhiteClick = instance->kSelectAlphaWhiteClick();
    m_kSelectAlphaWhiteDisable = instance->kSelectAlphaWhiteDisable();
    m_kSelectAlphaWhiteHover = instance->kSelectAlphaWhiteHover();
    m_kSelectAlphaWhite = instance->kSelectAlphaWhite();
    m_kshadowComponent = instance->kshadowComponent();
    m_kshadowMenu = instance->kshadowMenu();
    m_kshadowMin = instance->kshadowMin();
    m_kshadowPrimaryActive = instance->kshadowPrimaryActive();
    m_kshadowPrimaryInactive = instance->kshadowPrimaryInactive();
    m_kshadowSecondaryActive = instance->kshadowSecondaryActive();
    m_kshadowSecondaryInactive = instance->kshadowSecondaryInactive();
    m_kWhite = instance->kWhite();

    // 通用状态属性
    m_lightActive = instance->lightActive();
    m_lightDisable = instance->lightDisable();
    m_lightInactive = instance->lightInactive();
    m_linkActive = instance->linkActive();
    m_linkDisable = instance->linkDisable();
    m_linkInactive = instance->linkInactive();
    m_linkVisitedActive = instance->linkVisitedActive();
    m_linkVisitedDisable = instance->linkVisitedDisable();
    m_linkVisitedInactive = instance->linkVisitedInactive();
    m_midActive = instance->midActive();
    m_midDisable = instance->midDisable();
    m_midInactive = instance->midInactive();
    m_midlightActive = instance->midlightActive();
    m_midlightDisable = instance->midlightDisable();
    m_midlightInactive = instance->midlightInactive();
    m_normalline = instance->normalline();
    m_noRoleActive = instance->noRoleActive();
    m_noRoleDisable = instance->noRoleDisable();
    m_noRoleInactive = instance->noRoleInactive();
    m_placeholderTextActive = instance->placeholderTextActive();
    m_placeholderTextDisable = instance->placeholderTextDisable();
    m_placeholderTextInactive = instance->placeholderTextInactive();
    m_shadow = instance->shadow();
    m_shadowActive = instance->shadowActive();
    m_shadowDisable = instance->shadowDisable();
    m_shadowInactive = instance->shadowInactive();
    m_textActive = instance->textActive();
    m_textDisable = instance->textDisable();
    m_textInactive = instance->textInactive();

    // 提示框/窗口属性
    m_toolTipBaseActive = instance->toolTipBaseActive();
    m_toolTipBaseDisable = instance->toolTipBaseDisable();
    m_toolTipBaseInactive = instance->toolTipBaseInactive();
    m_toolTipTextActive = instance->toolTipTextActive();
    m_toolTipTextDisable = instance->toolTipTextDisable();
    m_toolTipTextInactive = instance->toolTipTextInactive();
    m_windowActive = instance->windowActive();
    m_windowDisable = instance->windowDisable();
    m_windowInactive = instance->windowInactive();
    m_windowTextActive = instance->windowTextActive();
    m_windowTextDisable = instance->windowTextDisable();
    m_windowTextInactive = instance->windowTextInactive();

    // 圆角尺寸属性
    m_kradiusMax = instance->kradiusMax();
    m_kradiusMenu = instance->kradiusMenu();
    m_kradiusMin = instance->kradiusMin();
    m_kradiusNormal = instance->kradiusNormal();
    m_kradiusWindow = instance->kradiusWindow();

    emit tokenChanged();
}

// ========== 基础调色板属性 - GET/SET ==========
const QBrush &UKUIToken::alternateBaseActive() const { return m_alternateBaseActive; }

const QBrush &UKUIToken::alternateBaseDisable() const { return m_alternateBaseDisable; }

const QBrush &UKUIToken::alternateBaseInactive() const { return m_alternateBaseInactive; }

const QBrush &UKUIToken::baseActive() const { return m_baseActive; }

const QBrush &UKUIToken::baseDisable() const { return m_baseDisable; }

const QBrush &UKUIToken::baseInactive() const { return m_baseInactive; }

const QBrush &UKUIToken::brightTextActive() const { return m_brightTextActive; }

const QBrush &UKUIToken::brightTextDisable() const { return m_brightTextDisable; }

const QBrush &UKUIToken::brightTextInactive() const { return m_brightTextInactive; }

const QBrush &UKUIToken::buttonActive() const { return m_buttonActive; }

const QBrush &UKUIToken::buttonDisable() const { return m_buttonDisable; }

const QBrush &UKUIToken::buttonInactive() const { return m_buttonInactive; }

const QBrush &UKUIToken::buttonTextActive() const { return m_buttonTextActive; }

const QBrush &UKUIToken::buttonTextDisable() const { return m_buttonTextDisable; }

const QBrush &UKUIToken::buttonTextInactive() const { return m_buttonTextInactive; }

const QBrush &UKUIToken::darkActive() const { return m_darkActive; }

const QBrush &UKUIToken::darkDisable() const { return m_darkDisable; }

const QBrush &UKUIToken::darkInactive() const { return m_darkInactive; }

int UKUIToken::focusline() const { return m_focusline; }

const QBrush &UKUIToken::highLightActive() const { return m_highLightActive; }

const QBrush &UKUIToken::highLightDisable() const { return m_highLightDisable; }

const QBrush &UKUIToken::highLightInactive() const { return m_highLightInactive; }

const QBrush &UKUIToken::highLightedTextActive() const { return m_highLightedTextActive; }

const QBrush &UKUIToken::highLightedTextDisable() const { return m_highLightedTextDisable; }

const QBrush &UKUIToken::highLightedTextInactive() const { return m_highLightedTextInactive; }

// ========== 品牌色/组件色属性 - GET/SET ==========
const QBrush &UKUIToken::kBlack() const { return m_kBlack; }

const QBrush &UKUIToken::kBrand1() const { return m_kBrand1; }

const QBrush &UKUIToken::kBrand2() const { return m_kBrand2; }

const QBrush &UKUIToken::kBrand3() const { return m_kBrand3; }

const QBrush &UKUIToken::kBrand4() const { return m_kBrand4; }

const QBrush &UKUIToken::kBrand5() const { return m_kBrand5; }

const QBrush &UKUIToken::kBrand6() const { return m_kBrand6; }

const QBrush &UKUIToken::kBrand7() const { return m_kBrand7; }

const QBrush &UKUIToken::kBrand8() const { return m_kBrand8; }

const QBrush &UKUIToken::kBrand9() const { return m_kBrand9; }

const QBrush &UKUIToken::kBrandClick() const { return m_kBrandClick; }

const QBrush &UKUIToken::kBrandDisable() const { return m_kBrandDisable; }

const QBrush &UKUIToken::kBrandFocus() const { return m_kBrandFocus; }

const QBrush &UKUIToken::kBrandHover() const { return m_kBrandHover; }

const QBrush &UKUIToken::kBrandNormal() const { return m_kBrandNormal; }

const QBrush &UKUIToken::kComponentAlphaClick() const { return m_kComponentAlphaClick; }

const QBrush &UKUIToken::kComponentAlphaDisable() const { return m_kComponentAlphaDisable; }

const QBrush &UKUIToken::kComponentAlphaHover() const { return m_kComponentAlphaHover; }

const QBrush &UKUIToken::kComponentAlphaNormal() const { return m_kComponentAlphaNormal; }

const QBrush &UKUIToken::kComponentClick() const { return m_kComponentClick; }

const QBrush &UKUIToken::kComponentDisable() const { return m_kComponentDisable; }

const QBrush &UKUIToken::kComponentHover() const { return m_kComponentHover; }

const QBrush &UKUIToken::kComponentNormal() const { return m_kComponentNormal; }

const QBrush &UKUIToken::kComponentSelectedAlphaDisable() const { return m_kComponentSelectedAlphaDisable; }

const QBrush &UKUIToken::kComponentSelectedAlphaNormal() const { return m_kComponentSelectedAlphaNormal; }

const QBrush &UKUIToken::kComponentSelectedAlphaHover() const { return m_kComponentSelectedAlphaHover; }

const QBrush &UKUIToken::kComponentSelectedAlphaClick() const { return m_kComponentSelectedAlphaClick; }

const QBrush &UKUIToken::kComponentInput() const { return m_kComponentInput; }

const QBrush &UKUIToken::kComponentInputAlpha() const { return m_kComponentInputAlpha; }

const QBrush &UKUIToken::kComponentSelectedDisable() const { return m_kComponentSelectedDisable; }

const QBrush &UKUIToken::kComponentSelectedNormal() const { return m_kComponentSelectedNormal; }

const QBrush &UKUIToken::kComponentSelectedHover() const { return m_kComponentSelectedHover; }

const QBrush &UKUIToken::kComponentSelectedClick() const { return m_kComponentSelectedClick; }

// ========== 容器/分割线属性 - GET/SET ==========
const QBrush &UKUIToken::kContainAlphaClick() const { return m_kContainAlphaClick; }

const QBrush &UKUIToken::kContainAlphaHover() const { return m_kContainAlphaHover; }

const QBrush &UKUIToken::kContainClick() const { return m_kContainClick; }

const QBrush &UKUIToken::kContainGeneralAlphaNormal() const { return m_kContainGeneralAlphaNormal; }

const QBrush &UKUIToken::kContainGeneralInactive() const { return m_kContainGeneralInactive; }

const QBrush &UKUIToken::kContainGeneralNormal() const { return m_kContainGeneralNormal; }

const QBrush &UKUIToken::kContainHover() const { return m_kContainHover; }

const QBrush &UKUIToken::kContainSecondaryAlphaNormal() const { return m_kContainSecondaryAlphaNormal; }

const QBrush &UKUIToken::kContainSecondaryInactive() const { return m_kContainSecondaryInactive; }

const QBrush &UKUIToken::kContainSecondaryNormal() const { return m_kContainSecondaryNormal; }

const QBrush &UKUIToken::kDivider() const { return m_kDivider; }

const QBrush &UKUIToken::kDividerWhite() const { return m_kDividerWhite; }

// ========== 状态色属性 - GET/SET ==========
const QBrush &UKUIToken::kErrorClick() const { return m_kErrorClick; }

const QBrush &UKUIToken::kErrorDisable() const { return m_kErrorDisable; }

const QBrush &UKUIToken::kErrorHover() const { return m_kErrorHover; }

const QBrush &UKUIToken::kErrorNormal() const { return m_kErrorNormal; }

const QBrush &UKUIToken::kSuccessClick() const { return m_kSuccessClick; }

const QBrush &UKUIToken::kSuccessDisable() const { return m_kSuccessDisable; }

const QBrush &UKUIToken::kSuccessHover() const { return m_kSuccessHover; }

const QBrush &UKUIToken::kSuccessNormal() const { return m_kSuccessNormal; }

const QBrush &UKUIToken::kWarningClick() const { return m_kWarningClick; }

const QBrush &UKUIToken::kWarningDisable() const { return m_kWarningDisable; }

const QBrush &UKUIToken::kWarningHover() const { return m_kWarningHover; }

const QBrush &UKUIToken::kWarningNormal() const { return m_kWarningNormal; }

// ========== 字体色属性 - GET/SET ==========
const QBrush &UKUIToken::kFontPlaceholdertextDisable() const { return m_kFontPlaceholdertextDisable; }

const QBrush &UKUIToken::kFontPlaceholdertext() const { return m_kFontPlaceholdertext; }

const QBrush &UKUIToken::kFontPrimaryDisable() const { return m_kFontPrimaryDisable; }

const QBrush &UKUIToken::kFontPrimary() const { return m_kFontPrimary; }

const QBrush &UKUIToken::kFontSecondaryDisable() const { return m_kFontSecondaryDisable; }

const QBrush &UKUIToken::kFontSecondary() const { return m_kFontSecondary; }

const QBrush &UKUIToken::kFontStrong() const { return m_kFontStrong; }

const QBrush &UKUIToken::kFontWhiteDisable() const { return m_kFontWhiteDisable; }

const QBrush &UKUIToken::kFontWhitePlaceholdertextDisable() const { return m_kFontWhitePlaceholdertextDisable; }

const QBrush &UKUIToken::kFontWhiteplaceholdertext() const { return m_kFontWhiteplaceholdertext; }

const QBrush &UKUIToken::kFontWhite() const { return m_kFontWhite; }

const QBrush &UKUIToken::kFontWhiteSecondaryDisable() const { return m_kFontWhiteSecondaryDisable; }

const QBrush &UKUIToken::kFontWhiteSecondary() const { return m_kFontWhiteSecondary; }

// ========== 灰色系属性 - GET/SET ==========
const QBrush &UKUIToken::kGray0() const { return m_kGray0; }

const QBrush &UKUIToken::kGray1() const { return m_kGray1; }

const QBrush &UKUIToken::kGray2() const { return m_kGray2; }

const QBrush &UKUIToken::kGray3() const { return m_kGray3; }

const QBrush &UKUIToken::kGray4() const { return m_kGray4; }

const QBrush &UKUIToken::kGray5() const { return m_kGray5; }

const QBrush &UKUIToken::kGray6() const { return m_kGray6; }

const QBrush &UKUIToken::kGray7() const { return m_kGray7; }

const QBrush &UKUIToken::kGray8() const { return m_kGray8; }

const QBrush &UKUIToken::kGray9() const { return m_kGray9; }

const QBrush &UKUIToken::kGray10() const { return m_kGray10; }

const QBrush &UKUIToken::kGray11() const { return m_kGray11; }

const QBrush &UKUIToken::kGray12() const { return m_kGray12; }

const QBrush &UKUIToken::kGray13() const { return m_kGray13; }

const QBrush &UKUIToken::kGray14() const { return m_kGray14; }

const QBrush &UKUIToken::kGray15() const { return m_kGray15; }

const QBrush &UKUIToken::kGray16() const { return m_kGray16; }

const QBrush &UKUIToken::kGray17() const { return m_kGray17; }

const QBrush &UKUIToken::kGrayAlpha0() const { return m_kGrayAlpha0; }

const QBrush &UKUIToken::kGrayAlpha1() const { return m_kGrayAlpha1; }

const QBrush &UKUIToken::kGrayAlpha2() const { return m_kGrayAlpha2; }

const QBrush &UKUIToken::kGrayAlpha3() const { return m_kGrayAlpha3; }

const QBrush &UKUIToken::kGrayAlpha4() const { return m_kGrayAlpha4; }

const QBrush &UKUIToken::kGrayAlpha5() const { return m_kGrayAlpha5; }

const QBrush &UKUIToken::kGrayAlpha6() const { return m_kGrayAlpha6; }

const QBrush &UKUIToken::kGrayAlpha7() const { return m_kGrayAlpha7; }

const QBrush &UKUIToken::kGrayAlpha8() const { return m_kGrayAlpha8; }

const QBrush &UKUIToken::kGrayAlpha9() const { return m_kGrayAlpha9; }

const QBrush &UKUIToken::kGrayAlpha10() const { return m_kGrayAlpha10; }

const QBrush &UKUIToken::kGrayAlpha11() const { return m_kGrayAlpha11; }

const QBrush &UKUIToken::kGrayAlpha12() const { return m_kGrayAlpha12; }

const QBrush &UKUIToken::kGrayAlpha13() const { return m_kGrayAlpha13; }

// ========== 特殊色/线条属性 - GET/SET ==========
const QBrush &UKUIToken::kGreen1() const { return m_kGreen1; }

const QBrush &UKUIToken::kOrange1() const { return m_kOrange1; }

const QBrush &UKUIToken::kRed1() const { return m_kRed1; }

const QBrush &UKUIToken::kLineBrandClick() const { return m_kLineBrandClick; }

const QBrush &UKUIToken::kLineBrandDisable() const { return m_kLineBrandDisable; }

const QBrush &UKUIToken::kLineBrandHover() const { return m_kLineBrandHover; }

const QBrush &UKUIToken::kLineBrandNormal() const { return m_kLineBrandNormal; }

const QBrush &UKUIToken::kLineComponentClick() const { return m_kLineComponentClick; }

const QBrush &UKUIToken::kLineComponentDisable() const { return m_kLineComponentDisable; }

const QBrush &UKUIToken::kLineComponentHover() const { return m_kLineComponentHover; }

const QBrush &UKUIToken::kLineComponentNormal() const { return m_kLineComponentNormal; }

const QBrush &UKUIToken::kLineDisable() const { return m_kLineDisable; }

const QBrush &UKUIToken::kLineDock() const { return m_kLineDock; }

const QBrush &UKUIToken::kLineErrorClick() const { return m_kLineErrorClick; }

const QBrush &UKUIToken::kLineErrorDisable() const { return m_kLineErrorDisable; }

const QBrush &UKUIToken::kLineErrorHover() const { return m_kLineErrorHover; }

const QBrush &UKUIToken::kLineErrorNormal() const { return m_kLineErrorNormal; }

const QBrush &UKUIToken::kLineInputClick() const { return m_kLineInputClick; }

const QBrush &UKUIToken::kLineInputDisable() const { return m_kLineInputDisable; }

const QBrush &UKUIToken::kLineInputHover() const { return m_kLineInputHover; }

const QBrush &UKUIToken::kLineInputNormal() const { return m_kLineInputNormal; }

const QBrush &UKUIToken::kLineMenu() const { return m_kLineMenu; }

const QBrush &UKUIToken::kLineNormal() const { return m_kLineNormal; }

const QBrush &UKUIToken::kLineSelectboxClick() const { return m_kLineSelectboxClick; }

const QBrush &UKUIToken::kLineSelectboxDisable() const { return m_kLineSelectboxDisable; }

const QBrush &UKUIToken::kLineSelectboxHover() const { return m_kLineSelectboxHover; }

const QBrush &UKUIToken::kLineSelectboxNormal() const { return m_kLineSelectboxNormal; }

const QBrush &UKUIToken::kLineSelectboxSelected() const { return m_kLineSelectboxSelected; }

const QBrush &UKUIToken::kLineSuccessClick() const { return m_kLineSuccessClick; }

const QBrush &UKUIToken::kLineSuccessDisable() const { return m_kLineSuccessDisable; }

const QBrush &UKUIToken::kLineSuccessHover() const { return m_kLineSuccessHover; }

const QBrush &UKUIToken::kLineSuccessNormal() const { return m_kLineSuccessNormal; }

const QBrush &UKUIToken::kLineTable() const { return m_kLineTable; }

const QBrush &UKUIToken::kLineWarningClick() const { return m_kLineWarningClick; }

const QBrush &UKUIToken::kLineWarningDisable() const { return m_kLineWarningDisable; }

const QBrush &UKUIToken::kLineWarningHover() const { return m_kLineWarningHover; }

const QBrush &UKUIToken::kLineWarningNormal() const { return m_kLineWarningNormal; }

const QBrush &UKUIToken::kLineWindowActive() const { return m_kLineWindowActive; }

const QBrush &UKUIToken::kLineWindowInactive() const { return m_kLineWindowInactive; }

// ========== 菜单/阴影属性 - GET/SET（续） ==========
const QBrush &UKUIToken::kMenu() const { return m_kMenu; }

const QBrush &UKUIToken::kModalmask() const { return m_kModalmask; }

const QBrush &UKUIToken::kSelectAlphaWhiteClick() const { return m_kSelectAlphaWhiteClick; }

const QBrush &UKUIToken::kSelectAlphaWhiteDisable() const { return m_kSelectAlphaWhiteDisable; }

const QBrush &UKUIToken::kSelectAlphaWhiteHover() const { return m_kSelectAlphaWhiteHover; }

const QBrush &UKUIToken::kSelectAlphaWhite() const { return m_kSelectAlphaWhite; }

const QBrush &UKUIToken::kshadowComponent() const { return m_kshadowComponent; }

const QBrush &UKUIToken::kshadowMenu() const { return m_kshadowMenu; }

const QBrush &UKUIToken::kshadowMin() const { return m_kshadowMin; }

const QBrush &UKUIToken::kshadowPrimaryActive() const { return m_kshadowPrimaryActive; }

const QBrush &UKUIToken::kshadowPrimaryInactive() const { return m_kshadowPrimaryInactive; }

const QBrush &UKUIToken::kshadowSecondaryActive() const { return m_kshadowSecondaryActive; }

const QBrush &UKUIToken::kshadowSecondaryInactive() const { return m_kshadowSecondaryInactive; }

const QBrush &UKUIToken::kWhite() const { return m_kWhite; }


// ========== 通用状态属性 - GET/SET ==========
const QBrush &UKUIToken::lightActive() const { return m_lightActive; }

const QBrush &UKUIToken::lightDisable() const { return m_lightDisable; }

const QBrush &UKUIToken::lightInactive() const { return m_lightInactive; }

const QBrush &UKUIToken::linkActive() const { return m_linkActive; }

const QBrush &UKUIToken::linkDisable() const { return m_linkDisable; }

const QBrush &UKUIToken::linkInactive() const { return m_linkInactive; }

const QBrush &UKUIToken::linkVisitedActive() const { return m_linkVisitedActive; }

const QBrush &UKUIToken::linkVisitedDisable() const { return m_linkVisitedDisable; }

const QBrush &UKUIToken::linkVisitedInactive() const { return m_linkVisitedInactive; }

const QBrush &UKUIToken::midActive() const { return m_midActive; }

const QBrush &UKUIToken::midDisable() const { return m_midDisable; }

const QBrush &UKUIToken::midInactive() const { return m_midInactive; }

const QBrush &UKUIToken::midlightActive() const { return m_midlightActive; }

const QBrush &UKUIToken::midlightDisable() const { return m_midlightDisable; }

const QBrush &UKUIToken::midlightInactive() const { return m_midlightInactive; }

int UKUIToken::normalline() const { return m_normalline; }

const QBrush &UKUIToken::noRoleActive() const { return m_noRoleActive; }

const QBrush &UKUIToken::noRoleDisable() const { return m_noRoleDisable; }

const QBrush &UKUIToken::noRoleInactive() const { return m_noRoleInactive; }

const QBrush &UKUIToken::placeholderTextActive() const { return m_placeholderTextActive; }

const QBrush &UKUIToken::placeholderTextDisable() const { return m_placeholderTextDisable; }

const QBrush &UKUIToken::placeholderTextInactive() const { return m_placeholderTextInactive; }

const QBrush &UKUIToken::shadow() const { return m_shadow; }

const QBrush &UKUIToken::shadowActive() const { return m_shadowActive; }

const QBrush &UKUIToken::shadowDisable() const { return m_shadowDisable; }

const QBrush &UKUIToken::shadowInactive() const { return m_shadowInactive; }

const QBrush &UKUIToken::textActive() const { return m_textActive; }

const QBrush &UKUIToken::textDisable() const { return m_textDisable; }

const QBrush &UKUIToken::textInactive() const { return m_textInactive; }

// ========== 提示框/窗口属性 - GET/SET ==========
const QBrush &UKUIToken::toolTipBaseActive() const { return m_toolTipBaseActive; }

const QBrush &UKUIToken::toolTipBaseDisable() const { return m_toolTipBaseDisable; }

const QBrush &UKUIToken::toolTipBaseInactive() const { return m_toolTipBaseInactive; }

const QBrush &UKUIToken::toolTipTextActive() const { return m_toolTipTextActive; }

const QBrush &UKUIToken::toolTipTextDisable() const { return m_toolTipTextDisable; }

const QBrush &UKUIToken::toolTipTextInactive() const { return m_toolTipTextInactive; }

const QBrush &UKUIToken::windowActive() const { return m_windowActive; }

const QBrush &UKUIToken::windowDisable() const { return m_windowDisable; }

const QBrush &UKUIToken::windowInactive() const { return m_windowInactive; }

const QBrush &UKUIToken::windowTextActive() const { return m_windowTextActive; }

const QBrush &UKUIToken::windowTextDisable() const { return m_windowTextDisable; }

const QBrush &UKUIToken::windowTextInactive() const { return m_windowTextInactive; }

// ========== 圆角尺寸属性 - GET/SET ==========
int UKUIToken::kradiusMax() const { return m_kradiusMax; }

int UKUIToken::kradiusMenu() const { return m_kradiusMenu; }

int UKUIToken::kradiusMin() const { return m_kradiusMin; }

int UKUIToken::kradiusNormal() const { return m_kradiusNormal; }

int UKUIToken::kradiusWindow() const { return m_kradiusWindow; }
