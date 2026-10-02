/*
 * KWin Style UKUI
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <kwineffects.h>

#include "button.h"
#include "ukui-decoration.h"

#include "effects.h"
#include "shadow-helper.h"
#include "xatom-helper.h"
#include <QtDBus> //必须放xatom-helper.h前面

#include <QPainter>
#include <QToolTip>

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationSettings>
#include <KPluginFactory>

#include <KConfigGroup>
#include <KSharedConfig>

#define FONT_SIZE 11 // 字体大小
#define CUSOR_BORDER 13 // 边框伸展光标范围
#define RADIUS 12

// UKUI 4.0 阴影更新
#define SHADOW_BORDER_ACTIVE 38 // 阴影大小激活状态
#define SHADOW_BORDER_INACTIVE 25 // 阴影大小非激活状态
#define SHADOW_SHIFT_Y_ACTIVE 22 // 阴影偏移激活状态
#define SHADOW_SHIFT_Y_INACTIVE 15 // 阴影偏移非激活状态
#define SHADOWCOLOR_ALPHAF_ACTIVE 0.60 // 阴影透明度激活状态
#define SHADOWCOLOR_ALPHAF_INACTIVE 0.40 // 阴影透明度激活状态
#define SHADOW_BORDER_ALPHA 0.15 // 描边透明度

K_PLUGIN_FACTORY_WITH_JSON(
    UKUIDecotionFactory,
    "kwin-style-ukui.json",
    registerPlugin<UKUI::Decoration>();)

using namespace UKUI;

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
    , m_shadowRadius(RADIUS)
    , m_shadowBorder_active(SHADOW_BORDER_ACTIVE)
    , m_shadowBorder_inactive(SHADOW_BORDER_INACTIVE)
    , m_shadowShift_active(0, SHADOW_SHIFT_Y_ACTIVE)
    , m_shadowShift_inactive(0, SHADOW_SHIFT_Y_INACTIVE)
    , m_shadowAlphaF_active(SHADOWCOLOR_ALPHAF_ACTIVE)
    , m_shadowAlphaF_inactive(SHADOWCOLOR_ALPHAF_INACTIVE)
    , m_borderAlphaF(SHADOW_BORDER_ALPHA)
    , m_borderActiveColor(QColor(0, 0, 0, 255 * SHADOW_BORDER_ALPHA))
    , m_borderInactiveColor(QColor(0, 0, 0, 255 * SHADOW_BORDER_ALPHA))
    , m_shadowActiveColor(QColor(0, 0, 0, 255 * SHADOWCOLOR_ALPHAF_ACTIVE))
    , m_shadowInactiveColor(QColor(0, 0, 0, 255 * SHADOWCOLOR_ALPHAF_INACTIVE))
    , m_frameActiveColor(QColor(255, 255, 255))
    , m_frameInactiveColor(QColor(245, 245, 245))
    , m_fontColor(QColor(38, 38, 38))
    , m_themeId(0)
    , m_themeProject(THEME_PROJECT_UKUI_BASE)
    , m_iconTheme(IconThemeDefault)
    , m_bTabletMode(false)
{
    int nDpi = 96;
    m_Font.setPointSize(FONT_SIZE); // setPointSize可以根据dpi自动调整，所以m_nFont不需要乘缩放系数，而setPixelSize会写死
    if (false == args.isEmpty()) {
        const auto map = args.first().toMap();
        auto it = map.constFind(QStringLiteral("dpi"));
        if (it != map.constEnd()) {
            nDpi = it.value().toInt();
        }

        it = map.constFind(QStringLiteral("themeId"));
        if (it != map.constEnd()) {
            themeUpdate(it.value().toBool());
        }

        it = map.constFind(QStringLiteral("themeProject"));
        if (it != map.constEnd()) {
            m_themeProject = static_cast<ThemeProject>(it.value().toInt());
        }

        it = map.constFind(QStringLiteral("systemFontSize"));
        if (it != map.constEnd()) {
            m_Font.setPointSize(it.value().toInt());
        }

        it = map.constFind(QStringLiteral("systemFont"));
        if (it != map.constEnd()) {
            m_Font.setFamily(it.value().toString());
        }

        it = map.constFind(QStringLiteral("tabletMode"));
        if (it != map.constEnd()) {
            m_bTabletMode = it.value().toBool();
        }

        it = map.constFind(QStringLiteral("iconTheme"));
        if (it != map.constEnd()) {
            iconThemeUpdate(it.value().toInt());
        }
    }
    m_rightToLeft = (QGuiApplication::layoutDirection() == Qt::RightToLeft);

    m_scaler = nDpi / 96.0f;
    m_borderLeft = m_scaler * 0;
    m_borderTop = m_scaler * 38;
    // 平板模式下，主界面标题栏高度 64，模态窗口标题栏高度 48;
    m_tabletBorderTop = m_scaler * 64;
    m_tabletBorderTopForModal = m_scaler * 48;
    m_borderRight = m_scaler * 0;
    m_borderBottom = m_scaler * 0;

    m_buttonWidth = m_scaler * (m_rightToLeft ? 24 : 30);
    m_buttonHeight = m_scaler * (m_rightToLeft ? 24 : 30);

    // 平板模式下，三联按钮显示区域 40x40，响应区域 48x48，
    // 因此需要在按钮内部增加 4 像素的内边距;
    m_innerButtonBorder = 0;
    m_tabletInnerButtonBorder = 4;

    m_tabletButtonWidth = m_scaler * (m_rightToLeft ? 32 : 48);
    m_tabletButtonHeight = m_scaler * (m_rightToLeft ? 32 : 48);

    m_leftButtonWidth = m_scaler * (m_rightToLeft ? 30 : 24);
    m_leftButtonHeight = m_scaler * (m_rightToLeft ? 30 : 24);

    // 平板模式下，左侧图标按钮大小 32x32;
    m_tabletLeftButtonWidth = m_scaler * (m_rightToLeft ? 48 : 32);
    m_tabletLeftButtonHeight = m_scaler * (m_rightToLeft ? 48 : 32);

    m_leftButtonMarginTop = m_ButtonMarginTop = m_scaler * 4;

    // 平板模式下，左侧按钮 32x32，标题栏高 64，因此需要上下边距 16 像素;
    m_tabletLeftButtonMarginTop = m_scaler * (m_rightToLeft ? 8 : 16);
    // 平板模式下，左侧按钮 32x32，标题栏高 64，因此需要上下边距 16 像素;
    m_tabletLeftButtonMarginTopForModal = m_scaler * (m_rightToLeft ? 0 : 8);
    // 平板模式下，主界面三联按钮 48x48，标题栏高 64，因此需要上下边距 8 像素;
    m_tabletButtonMarginTop = m_scaler * (m_rightToLeft ? 16 : 8);
    // 平板模式下，模态窗口三联按钮 48x48，标题栏高 48，因此需要上下边距 0 像素;
    m_tabletButtonMarginTopForModal = m_scaler * (m_rightToLeft ? 8 : 0);

    m_tabletLeftButtonSpacing = m_leftButtonSpacing = m_buttonSpacing = m_scaler * 4;
    // 平板模式下，内边距充当按钮 spacing，故 spacing 设置为 0;
    m_tabletButtonSpacing = m_scaler * 0;

    m_leftButtons = nullptr;
    m_rightButtons = nullptr;
}

void Decoration::init()
{
    m_shadowRadius = RADIUS;
    if (client().toStrongRef().data()->shadowRadius() >= 0) {
        m_shadowRadius = client().toStrongRef().data()->shadowRadius();
    }

    // 从读取配置文件的方式，判断kwin是否能开启毛玻璃效果，如果不能，则阴影圆角为0.
    auto config = KSharedConfig::openConfig("kwinrc");
    auto group = KConfigGroup(config, "Compositing");
    if (group.readEntry("Backend") == "XRender" || group.readEntry("OpenGLIsUnsafe") == "true") {
        m_shadowRadius = 0;
    }
    if (XAtomHelper::getInstance()->isFrameLessWindow(this->client().toStrongRef().data()->windowId())) {
        m_shadowRadius = 0;
    }

    XAtomHelper::getInstance()->setUKUIDecoraiontHint(client().toStrongRef().data()->windowId(), true);

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        m_styleSettings.reset(new QGSettings("org.ukui.style"));
        if (m_styleSettings) {
            auto readStyleName = [this] {
                QString themeName = m_styleSettings->get("style-name").toString();
                if (themeName == "ukui-light")
                    themeUpdate(0);
                else if (themeName == "ukui-dark")
                    themeUpdate(1);
                else
                    themeUpdate(0);
            };

            auto readIconThemeName = [this] {
                QString iconThemeName = m_styleSettings->get("icon-theme-name").toString();
                if (iconThemeName == "ukui-icon-theme-fashion")
                    iconThemeUpdate(1);
                else if (iconThemeName == "ukui-icon-theme-classical")
                    iconThemeUpdate(2);
                else
                    iconThemeUpdate(0);
            };

            if (m_styleSettings->keys().contains("styleName")) {
                readStyleName();
            }
            if (m_styleSettings->keys().contains("iconThemeName")) {
                readIconThemeName();
            }
            if (m_styleSettings->keys().contains("windowRadius")) {
                m_shadowRadius = m_styleSettings->get("window-radius").toInt();
                update();
                updateShadow();
            }

            connect(m_styleSettings.get(), &QGSettings::changed, this, [this, readStyleName, readIconThemeName](const QString &key) {
                if (key == "styleName") {
                    readStyleName();
                }
                if (key == "iconThemeName") {
                    readIconThemeName();
                }
                if (key == "windowRadius") {
                    m_shadowRadius = m_styleSettings->get("window-radius").toInt();
                    update();
                    updateShadow();
                }
            });
        }
    }

    bool isDecoBorderOnly = XAtomHelper::getInstance()->isWindowDecorateBorderOnly(client().toStrongRef().data()->windowId()); // 是否是仅修饰边框
    bool isUKUIDeco = property("ukui-decoration").toBool();
    isDecoBorderOnly |= isUKUIDeco;
    if (!isDecoBorderOnly) {
        // 标题栏dbus监听PC-平板模式切换
        QDBusConnection::sessionBus().connect("com.kylin.statusmanager.interface",
                                              "/",
                                              "com.kylin.statusmanager.interface",
                                              "mode_change_signal",
                                              this,
                                              SLOT(slotSwitchTablet(bool)));

        themeUpdate(m_themeId);
        calculateBorders();
        // button
        m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &UKUI::Button::create);
        m_leftButtons->setSpacing(leftButtonSpacing());

        calculateLeftButtonCount();

        m_rightButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &UKUI::Button::create);
        m_rightButtons->setSpacing(buttonSpacing());

        calculateRightButtonCout();

        connect(settings().data(), &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, &UKUI::Decoration::updateButtonsGeometry);
        connect(settings().data(), &KDecoration2::DecorationSettings::fontChanged, this, &UKUI::Decoration::updatefont);
        connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::sizeChanged, this, &UKUI::Decoration::updateButtonsGeometry);
        connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::paletteChanged, this, static_cast<void (Decoration::*)()>(&Decoration::update));
        connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::activeChanged, this, static_cast<void (Decoration::*)()>(&Decoration::update));
        connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::maximizeableChanged, this, &Decoration::calculateRightButtonCout); // 安装兼容应用全屏后还原，可最大化按钮有改变
        connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::calculateBorders);
        connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::captionChanged, this,
                [this]() {
            update(); // 更新标题栏标题内容, update(titleBar())这有时不能即时更新标题栏内容
        });

        connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::widthChanged, this, &Decoration::updateTitleBar);
        connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateTitleBar);
    } else {
        setBorders(QMargins(0, 0, 0, 0));
        setResizeOnlyBorders(QMargins(CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER));
        // connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Decoration::updateShadow);
    }

    connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::activeChanged, this, &UKUI::Decoration::updateShadow);
    connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::adjacentScreenEdgesChanged, this, &UKUI::Decoration::updateShadow);

    auto effectsHandler = dynamic_cast<KWin::EffectsHandlerImpl *>(KWin::effects);
    if (effectsHandler) {
        connect(effectsHandler, &KWin::EffectsHandlerImpl::activeFullScreenEffectChanged, this, [this, effectsHandler] {
            if (!effectsHandler->hasActiveFullScreenEffect())
                updateShadow();
        });
    }

    connect(client().toStrongRef().data(), &KDecoration2::DecoratedClient::sizeChanged, this, [=]() {
        auto effectManager = KWin::effects;
        if (!effectManager)
            return;
        for (auto window : effectManager->stackingOrder()) {
            if (!window)
                continue;
            bool sameCaption = false;
            bool sameSize = false;
            if (window->caption() == this->client().toStrongRef().data()->caption()) {
                sameCaption = true;
            }
            if (window->frameGeometry().size() == client().toStrongRef().data()->decoration().data()->size()) {
                sameSize = true;
            }
            if (sameCaption && sameSize) {
                bool isEdge = client().toStrongRef().data()->adjacentScreenEdges() != Qt::Edge();
                if (isEdge) {
                    window->setData(1000, true);
                } else {
                    window->setData(1000, QVariant());
                }
            }
        }
    });

    // get theme project
    if (m_themeProject == THEME_PROJECT_UKUI_BASE) {
        QSettings osinfo("/etc/os-release", QSettings::IniFormat);
        if (osinfo.contains(QString("PROJECT_CODENAME"))) {
            QString project_codename = osinfo.value(QString("PROJECT_CODENAME")).toString();
            if (0 == project_codename.compare("V10SP1-edu", Qt::CaseInsensitive))
                m_themeProject = THEME_PROJECT_LAIKA;
            else if (0 == project_codename.compare("V10SPX-mavis-edu", Qt::CaseInsensitive))
                m_themeProject = THEME_PROJECT_MAVIS;
        }
    }
}

void Decoration::updatefont(QFont font)
{
    m_Font = font;
    QToolTip::setFont(font);
    update();
}

void Decoration::updateShadow()
{
    auto ubr = XAtomHelper::getInstance()->getWindowBorderRadius(client().toStrongRef().data()->windowId());

    // 控制左上、右上的阴影，ubr不生效时，阴影也应该是0
    if (ubr.topLeft <= 0) {
        ubr.topLeft = m_shadowRadius * m_scaler;
    }
    if (ubr.topRight <= 0) {
        ubr.topRight = m_shadowRadius * m_scaler;
    }
    if (ubr.bottomLeft <= 0) {
        ubr.bottomLeft = m_shadowRadius * m_scaler;
    }
    if (ubr.bottomRight <= 0) {
        ubr.bottomRight = m_shadowRadius * m_scaler;
    }

    bool ubrEnabled = false;
    auto effectsHandler = dynamic_cast<KWin::EffectsHandlerImpl *>(KWin::effects);
    if ((effectsHandler && effectsHandler->isEffectLoaded("ubr") && !client().toStrongRef().data()->adjacentScreenEdges()))
        ubrEnabled = true;
    int isActive = client().toStrongRef().data()->isActive();
    int shadowBorder = isActive ? m_shadowBorder_active : m_shadowBorder_inactive;
    QPoint shadowShift = isActive ? m_shadowShift_active : m_shadowShift_inactive;
    QColor shadowColor = isActive ? m_shadowActiveColor : m_shadowInactiveColor;
    QColor borderColor = isActive ? m_borderActiveColor : m_borderInactiveColor;
    auto old_shadow = shadow();
    ShadowInfo shadowInfo(shadowColor, borderColor, ubr.topLeft, ubr.topRight, ubr.bottomLeft, ubr.bottomRight, shadowBorder, shadowShift, ubrEnabled);
    auto shadow = ShadowHelper::globalInstance()->getCreatedShadow(shadowInfo);
    if (old_shadow == shadow) // 如果返回的阴影相同，则不更新阴影。
        return;
    setResizeOnlyBorders(QMargins(CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER));
    if (effectsHandler && !effectsHandler->hasActiveFullScreenEffect())
        setShadow(shadow);
}

void Decoration::updateTitleBar()
{
    bool isUKUIDeco = property("ukui-decoration").toBool();
    bool isDecoBorderOnly = XAtomHelper::getInstance()->isWindowDecorateBorderOnly(client().toStrongRef().data()->windowId());
    if (isDecoBorderOnly || isUKUIDeco)
        return;
    auto c = client().toStrongRef().data();
    if (m_rightToLeft) {
        // x 代表左侧按钮范围：最左侧有一个 buttonMarginTop() 的空白，然后一个 spacing，然后一个左侧按钮，再一个 spacing，再一个 buttonMarginTop()；
        const int x = 2 * buttonMarginTop() + buttonSpacing() * 2 + leftButtonWidth() * 3;
        const int width = c->width() - x - buttonWidth() - buttonMarginTop() * 2 - buttonSpacing() * 2;
        setTitleBar(QRect(x, 0, width, borderTop()));
    } else {
        // x 代表左侧按钮范围：最左侧有一个 buttonMarginTop() 的空白，然后一个 spacing，然后一个左侧按钮，再一个 spacing，再一个 buttonMarginTop()；
        const int x = (leftButtonMarginTop() + buttonSpacing()) * 2 + m_nleftButtonCout * leftButtonWidth();
        const int width = c->width() + (c->isMaximized() ? 0 : (m_borderLeft + m_borderRight))
            - x // 减去左侧按钮空间
            - m_nrightButtonCout * (buttonWidth() + buttonSpacing()); // 减去右侧按钮空间
        setTitleBar(QRect(x, 0, width, borderTop()));
    }
}

void Decoration::calculateBorders()
{
    bool maximized = client().toStrongRef().data()->isMaximized();
    if (maximized) {
        // 在最大化时需要对setBorders中其中一个参数调整一下大小，否则，兆芯笔记本使用VSCode开源软件最大化时，会出现页面刷新不完全的情况。此处选择将m_borderTop - 1进行调整。
        // 后来由于使用新版VSCode不会出现该问题，故又将m_borderTop - 1调整回m_borderTop
        setBorders(QMargins(0, borderTop(), 0, 0)); // 真正的边框尺寸
        setResizeOnlyBorders(QMargins(0, 0, 0, 0)); // 边框伸展光标范围
    } else {
        setBorders(QMargins(m_borderLeft, borderTop(), m_borderRight, m_borderBottom)); // 真正的边框尺寸
        setResizeOnlyBorders(QMargins(CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER)); // 边框伸展光标范围
    }
}

void Decoration::slotSwitchTablet(bool bTabletMode)
{
    if (bTabletMode == m_bTabletMode)
        return;

    m_bTabletMode = bTabletMode;

    calculateLeftButtonCount();
    calculateRightButtonCout();
    calculateBorders();

    return;
}

void Decoration::calculateLeftButtonCount()
{
    m_nleftButtonCout = 0;
    auto buttons = m_leftButtons->buttons();
    for (const KDecoration2::DecorationButton *button : buttons) {
        auto ukuiButton = (UKUI::Button *)button;
        ukuiButton->setGeometry(QRectF(QPointF(0, 0), QSizeF(leftButtonWidth(), leftButtonHeight())));
        if (KDecoration2::DecorationButtonType::Menu == ukuiButton->type())
            m_nleftButtonCout++;
    }
}

void Decoration::calculateRightButtonCout()
{
    m_nrightButtonCout = 0;
    auto buttons = m_rightButtons->buttons();

    for (const KDecoration2::DecorationButton *button : buttons) {
        auto ukuiButton = (UKUI::Button *)button;

        if (KDecoration2::DecorationButtonType::Minimize == ukuiButton->type()) {
            // 平板模式下，模态窗口去掉最小化按钮;
            if (!client().toStrongRef().data()->isMinimizeable() || (client().toStrongRef().data()->isModal() && m_bTabletMode)) {
                ukuiButton->setVisible(false);
                continue;
            } else {
                ukuiButton->setVisible(true);
                m_nrightButtonCout++;
                ukuiButton->setInnerBorder(innerButtonBorder());
            }
        }

        // 安卓兼容应用全屏后还原，从可最大化转变为不可最大化，此处增加隐藏按钮设定
        if (KDecoration2::DecorationButtonType::Maximize == ukuiButton->type()) {
            if (!client().toStrongRef().data()->isMaximizeable()) {
                ukuiButton->setVisible(false);
                continue;
            } else {
                if (m_bTabletMode)
                    ukuiButton->setVisible(false);
                else {
                    ukuiButton->setVisible(true);
                    m_nrightButtonCout++;
                }
            }
        }

        if (KDecoration2::DecorationButtonType::Close == ukuiButton->type()) {
            if (!client().toStrongRef().data()->isCloseable()) {
                ukuiButton->setVisible(false);
                continue;
            } else {
                ukuiButton->setVisible(true);
                m_nrightButtonCout++;
                ukuiButton->setInnerBorder(innerButtonBorder());
            }
        }

        ukuiButton->setGeometry(QRectF(QPointF(0, 0), QSizeF(buttonWidth(), buttonHeight())));
    }

    updateButtonsGeometry();
    updateTitleBar();
}

void Decoration::themeUpdate(int themeId)
{
    if (m_themeId == themeId)
        return;
    m_themeId = themeId;
    if (1 == m_themeId) {
        m_frameActiveColor = QColor(18, 18, 18);
        m_frameInactiveColor = QColor(28, 28, 28);
        m_fontColor = QColor(207, 207, 207);
        m_borderActiveColor = QColor(255, 255, 255, 255 * SHADOW_BORDER_ALPHA);
        m_borderInactiveColor = QColor(255, 255, 255, 255 * SHADOW_BORDER_ALPHA);
    } else {
        m_frameActiveColor = QColor(255, 255, 255);
        m_frameInactiveColor = QColor(245, 245, 245);
        m_fontColor = QColor(38, 38, 38);
        m_borderActiveColor = QColor(0, 0, 0, 255 * SHADOW_BORDER_ALPHA);
        m_borderInactiveColor = QColor(0, 0, 0, 255 * SHADOW_BORDER_ALPHA);
    }

    update();
    updateShadow();
}

void Decoration::iconThemeUpdate(int iconTheme)
{
    m_iconTheme = static_cast<KylinIconTheme>(iconTheme);
}

void Decoration::paint(QPainter *painter, const QRect &repaintRegion)
{
    if (!repaintRegion.isEmpty()) {
        painter->setClipRect(repaintRegion);
    }

    auto c = client().toStrongRef().data();

    // 总体框架刷颜色
    painter->save();

    auto effectsHandler = dynamic_cast<KWin::EffectsHandlerImpl *>(KWin::effects);
    bool ubr = effectsHandler && effectsHandler->isEffectLoaded("ubr");
    bool noRoundCorner = c->isMaximized() || !ubr;
    if (noRoundCorner) {
        auto rect = QRect(QPoint(0, 0), this->client().toStrongRef().data()->size());
        painter->fillRect(rect, this->client().toStrongRef().data()->isActive() ? this->m_frameActiveColor : this->m_frameInactiveColor);
    } else if (c->adjacentScreenEdges() != Qt::Edge()) {
        auto rect = QRect(0, 0, (c->size().width() + m_borderLeft + m_borderRight), (c->size().height() + borderTop() + m_borderBottom));
        painter->setPen(Qt::NoPen);
        painter->setBrush(this->client().toStrongRef().data()->isActive() ? this->m_frameActiveColor : this->m_frameInactiveColor);
        painter->drawRoundedRect(rect, 0, 0);
    } else {
        auto rect = QRect(0, 0, (c->size().width() + m_borderLeft + m_borderRight), (c->size().height() + borderTop() + m_borderBottom));
        painter->setPen(Qt::NoPen);
        painter->setBrush(this->client().toStrongRef().data()->isActive() ? this->m_frameActiveColor : this->m_frameInactiveColor);
        // 控制左上、右上的阴影
        painter->drawRoundedRect(rect, m_shadowRadius, m_shadowRadius);

        auto rectLeftBottom = QRect(0, rect.height() - m_shadowRadius * 2, m_shadowRadius * 2, m_shadowRadius * 2);
        painter->drawRoundedRect(rectLeftBottom, 0, 0); // 左下角补角
        auto rectRightBottom = QRect(rect.width() - m_shadowRadius * 2, rect.height() - m_shadowRadius * 2, m_shadowRadius * 2, m_shadowRadius * 2);
        painter->drawRoundedRect(rectRightBottom, 0, 0);
    }
    painter->restore();

    // 写标题
    painter->setFont(m_Font);
    painter->setPen(fontColor());

    if (!m_rightToLeft) {
        const auto cR = qMakePair(titleBar(), Qt::AlignVCenter | Qt::AlignLeft);
        const QString caption = painter->fontMetrics().elidedText(c->caption(), Qt::ElideMiddle, cR.first.width());
        painter->drawText(cR.first, cR.second | Qt::TextSingleLine, caption);
    } else {
        const auto cR = qMakePair(titleBar(), Qt::AlignVCenter | Qt::AlignRight);
        const QString caption = painter->fontMetrics().elidedText(c->caption(), Qt::ElideMiddle, cR.first.width());
        painter->drawText(cR.first, cR.second | Qt::TextSingleLine, caption);
    }

    // 按钮组刷颜色
    if (m_leftButtons)
        m_leftButtons->paint(painter, repaintRegion);
    if (m_rightButtons)
        m_rightButtons->paint(painter, repaintRegion);
}

void Decoration::updateButtonsGeometry()
{
    auto c = client().toStrongRef().data();

    if (m_rightToLeft) {
        m_leftButtons->setPos(QPoint(leftButtonMarginTop(), leftButtonMarginTop()));
        // 由于上边檐是 buttonMarginTop()，右侧有m_nrightButtonCout个按钮和(m_nrightButtonCout - 1)个按钮间隙，以此确定右侧按钮的起始坐标
        auto posX = c->width() - buttonSpacing() - buttonWidth() - buttonMarginTop();
        m_rightButtons->setPos(QPoint(posX, buttonMarginTop() + buttonSpacing()));
    } else {
        m_leftButtons->setPos(QPoint(leftButtonMarginTop() + buttonSpacing(), leftButtonMarginTop() + buttonSpacing()));
        // 由于上边檐是 buttonMarginTop()，右侧有m_nrightButtonCout个按钮和(m_nrightButtonCout - 1)个按钮间隙，以此确定右侧按钮的起始坐标
        auto posX = c->width() + (c->isMaximized() ? 0 : (m_borderLeft + m_borderRight))
            // minus buttons' width and minus buttons' spacing, also minus one buttonMarginTop to make left spacing equals to right spacing.
            - m_nrightButtonCout * buttonWidth() - (m_nrightButtonCout - 1) * buttonSpacing() - buttonMarginTop();
        m_rightButtons->setPos(QPoint(posX, buttonMarginTop()));
    }
    update();
}

QColor Decoration::fontColor() const
{
    auto c = client().toStrongRef().data();
    if (c->isActive()) {
        return m_fontColor;
    } else {
        QColor color = m_fontColor;
        color.setAlphaF(0.3);
        return color;
    }
}

int Decoration::borderTop() const
{
    if (m_bTabletMode)
        return client().toStrongRef().data()->isModal() ? m_tabletBorderTopForModal : m_tabletBorderTop;
    else
        return m_borderTop;
}

int Decoration::buttonWidth() const
{
    return m_bTabletMode ? m_tabletButtonWidth : m_buttonWidth;
}

int Decoration::buttonHeight() const
{
    return m_bTabletMode ? m_tabletButtonHeight : m_buttonHeight;
}

int Decoration::leftButtonWidth() const
{
    return m_bTabletMode ? m_tabletLeftButtonWidth : m_leftButtonWidth;
}

int Decoration::leftButtonHeight() const
{
    return m_bTabletMode ? m_tabletLeftButtonHeight : m_leftButtonHeight;
}

int Decoration::innerButtonBorder() const
{
    return m_bTabletMode ? m_tabletInnerButtonBorder : m_innerButtonBorder;
}

int Decoration::buttonMarginTop() const
{
    if (m_bTabletMode)
        return client().toStrongRef().data()->isModal() ? m_tabletButtonMarginTopForModal : m_tabletButtonMarginTop;
    else
        return m_ButtonMarginTop;
}

int Decoration::leftButtonMarginTop() const
{
    if (m_bTabletMode)
        return client().toStrongRef().data()->isModal() ? m_tabletLeftButtonMarginTopForModal : m_tabletLeftButtonMarginTop;
    else
        return m_leftButtonMarginTop;
}

int Decoration::buttonSpacing() const
{
    return m_bTabletMode ? m_tabletButtonSpacing : m_buttonSpacing;
}

int Decoration::leftButtonSpacing() const
{
    return m_bTabletMode ? m_tabletLeftButtonSpacing : m_leftButtonSpacing;
}

#include <ukui-decoration.moc>
