/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#ifndef UKUICONFIGSTYLE_H
#define UKUICONFIGSTYLE_H

#include <QProxyStyle>
#include <QFontMetrics>
#include <QStyleOption>
#include <private/qtextengine_p.h>
#include <qmath.h>
#include <QGSettings>
#include "widget-parameters/control-parameters.h"
#include <QHash>
#include <QSet>
#include <QDBusInterface>
#include <QGSettings/qgsettings.h>


class QStyleOptionViewItem;
class QDBusInterface;

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
#include <private/qfusionstyle_p.h>
#define Style QFusionStyle
#else
#define Style QProxyStyle
#endif


class BlurHelper;
class GestureHelper;
class UkuiWindowHelper;
class WindowManager;

//class ApplicationStyleSettings;
//QT_BEGIN_NAMESPACE
//namespace UKUIConfigStyleSpace { class UKUIConfigStyle; }
//QT_END_NAMESPACE
/*!
 * \brief The UKUIConfigStyle class
 * \details
 * This class provide the default ukui style in UKUI desktop enviroment.
 */
namespace UKUIConfigStyleSpace {
class ConfigTabWidgetAnimationHelper;
class ConfigScrollBarAnimationHelper;
class ConfigButtonAnimationHelper;
class ConfigBoxAnimationHelper;
class ConfigProgressBarAnimationHelper;
class ConfigShadowHelper;
class ConfigRadioButtonAnimationHelper;
class ConfigCheckBoxAnimationHelper;
class ConfigSliderAnimationHelper;
class ConfigTreeAnimationHelper;
class UKUIConfigStyleParameters;
}
class UKUIConfigStyle : public Style
{
    Q_OBJECT
public:
    explicit UKUIConfigStyle(QString name);
    ~UKUIConfigStyle();

    bool shouldBeTransparent(const QWidget *w) const;

    //debuger
    bool eventFilter(QObject *obj, QEvent *e);

    int styleHint(StyleHint hint,
                  const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData) const;

    void polish(QWidget *widget);
    void unpolish(QWidget *widget);
    void polish(QPalette& palette) ;
    void polish(QApplication *app);


    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr,
                           const QWidget *widget = nullptr) const override;

    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;

    QPixmap testGeneratedSVGPixmap(QPixmap pixmap) const;

    QPalette standardPalette() const;

    void drawPrimitive(QStyle::PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget = nullptr) const;

    void drawComplexControl(QStyle::ComplexControl control,
                            const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget = nullptr) const;

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const;

    int pixelMetric(QStyle::PixelMetric metric,
                    const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const;

    QRect subControlRect(QStyle::ComplexControl control,
                         const QStyleOptionComplex *option,
                         QStyle::SubControl subControl,
                         const QWidget *widget = nullptr) const;

    QStyle::SubControl hitTestComplexControl(QStyle::ComplexControl control,
                                              const QStyleOptionComplex *option,
                                              const QPoint &pos,
                                              const QWidget *widget = nullptr) const override;

    void drawItemPixmap(QPainter *painter,
                        const QRect &rect,
                        int alignment,
                        const QPixmap &pixmap) const;

    QRect subElementRect(SubElement element,
                                 const QStyleOption *option,
                                 const QWidget *widget = nullptr) const;
    QSize sizeFromContents(ContentsType ct, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const;


    QPixmap drawColorPixmap(QPainter *p, QColor color, QPixmap &pixmap) const;

protected:

    void realSetWindowSurfaceFormatAlpha(const QWidget *widget) const;
    void realSetMenuTypeToMenu(const QWidget *widget) const;
    QRect centerRect(const QRect &rect, int width, int height) const;

private Q_SLOTS:
    void updateTabletModeValue(bool isTabletMode);
    QRect scrollbarSliderRec(QWidget *w);
    void setScrollbarSliderRec(QWidget *w, QRect rect) const;
    QRect sliderHandleRec(QWidget *w);
    void setSliderHandleRec(QWidget *w,QRect rect) const;

private:
    bool isUseDarkPalette() const;

    QColor button_Click(const QStyleOption *option) const;
    QColor button_Hover(const QStyleOption *option) const;
    QColor transparentButton_Click(const QStyleOption *option) const;
    QColor transparentButton_Hover(const QStyleOption *option) const;
    QColor highLight_Click(const QStyleOption *option) const;
    QColor highLight_Hover(const QStyleOption *option) const;

    void setThemeColor(QColor color, QPalette &palette) const;

    // view
    QString calculateElidedText(const QString &text, const QTextOption &textOption,
                                    const QFont &font, const QRect &textRect, const Qt::Alignment valign,
                                    Qt::TextElideMode textElideMode, int flags,
                                    bool lastVisibleLineShouldBeElided, QPointF *paintStartPosition) const;
    void viewItemDrawText(QPainter *painter, const QStyleOptionViewItem *option, const QRect &rect) const;
    void viewItemLayout(const QStyleOptionViewItem *option,  QRect *checkRect, QRect *pixmapRect, QRect *textRect, bool sizehint) const;
    QSize viewItemSize(const QStyleOptionViewItem *option, int role) const;

    void getDefaultControlParameters(QApplication *app);

UkuiWindowHelper *windowHelperForWidget(QWidget *widget);
    void releaseWindowHelper(QWidget *widget);
    static bool isStandaloneLineEdit(const QLineEdit *lineEdit);
    void ensureLineEditClearToolButton(QLineEdit *lineEdit);
    void updateLineEditClearToolButton(QLineEdit *lineEdit);

private:
    UKUIConfigStyleSpace::ConfigTabWidgetAnimationHelper *m_tab_animation_helper = nullptr;
    UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper *m_scrollbar_animation_helper = nullptr;
    UKUIConfigStyleSpace::ConfigButtonAnimationHelper*  m_button_animation_helper = nullptr;
    UKUIConfigStyleSpace::ConfigBoxAnimationHelper* m_combobox_animation_helper = nullptr;
    UKUIConfigStyleSpace::ConfigShadowHelper *m_shadow_helper = nullptr;
    UKUIConfigStyleSpace::ConfigProgressBarAnimationHelper *m_animation_helper = nullptr;
    UKUIConfigStyleSpace::ConfigRadioButtonAnimationHelper *m_radiobutton_animation_helper = nullptr;
    UKUIConfigStyleSpace::ConfigCheckBoxAnimationHelper *m_checkbox_animation_helper = nullptr;
    UKUIConfigStyleSpace::ConfigSliderAnimationHelper *m_slider_animation_helper = nullptr;
    UKUIConfigStyleSpace::ConfigTreeAnimationHelper *m_tree_animation_helper = nullptr;

    bool m_drak_palette = false;
    bool m_default_palette = false;
    QString m_widget_theme = "ukui-default";
    QString m_style_name = "ukui-default";

    bool m_is_tablet_mode = false;
    QDBusInterface *m_statusManagerDBus = nullptr;

    // UKUIConfigStyleParameters
    UKUIConfigStyleSpace::UKUIConfigStyleParameters *sp = nullptr;

    mutable QHash<QWidget*, QRect> m_scrollBarSliderRec;
    mutable QHash<QWidget*, QRect>  m_sliderHandleRec;

    BlurHelper *m_blur_helper = nullptr;
    GestureHelper *m_gesture_helper = nullptr;
    WindowManager *m_window_manager = nullptr;
    QHash<QWidget *, UkuiWindowHelper *> *m_windowHelperHash = nullptr;

//    ApplicationStyleSettings *m_app_style_settings = nullptr;

    bool m_blink_cursor = true;
    int m_blink_cursor_time = 1200;

private:
    mutable UKUIElementStatus::ProgressBarOrientation pbOrientation;//记录当前进度条状态
    const int pbBaseGrooveSpan = 10; // 循环进度条槽区厚度（px）

    QDBusInterface* m_wlcomDBus = nullptr;
    bool m_isHardwareRendering = true;
    QPointer<QObject> focusObj = nullptr;
    QPointer<QWidget> m_itemViewKeyboardFocus = nullptr; // Tab 链进入的 ItemView 键盘焦点（绘制 Table 用），鼠标点击清除
    mutable bool m_tabNavigationPending = false; // 刚按下 Tab/Shift+Tab，补 Qt 返回 OtherFocusReason 的情况

    QGSettings* m_peripheralsGsettings = nullptr;
public:
    void calculateHighColor(int& colorVal) const;
};

#endif // UKUICONFIGSTYLE_H
