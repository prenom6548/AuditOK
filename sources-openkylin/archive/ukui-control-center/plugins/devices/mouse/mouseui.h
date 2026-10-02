#ifndef MOUSEUI_H
#define MOUSEUI_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QRadioButton>
#include <QSlider>
#include <QByteArray>
#include <QVariant>
#include <QApplication>
#include <QButtonGroup>
#include <QDebug>
#include <QToolButton>
#include <QFocusEvent>
#include <QLineEdit>

#include "ukcccommon.h"
using namespace ukcc;

#include "titlelabel.h"
#include "switchwidget.h"
#include "sliderwidget.h"
#include "radiobuttonwidget.h"
#include "comboxwidget.h"
#include "settinggroup.h"

#include "kslider.h"
#include "kswitchbutton.h"
#include "klabel.h"
using namespace kdk;

#include "../../personalized/theme/themebutton.h"

#define THEME_QT_SCHEMA  "org.ukui.style"
#define MODE_QT_KEY      "style-name"

#define SMALLPOINTER 24
#define MEDIUMPOINTER 36
#define LARGEPOINTER 48

#define POINTER_SIZE_WIDTH 64
#define POINTER_SIZE_HEIGHT 96
#define SMALL_ICON_SIZE 24,24
#define MEDIUE_ICON_SIZE 32,32
#define LARGE_ICON_SIZE 48,48

enum PositionType {
    NONE = 0,
    PRESSCTRL,
    SHARKMOUSE,
};

class DoubleClickTestLabel : public QLabel
{
    Q_OBJECT

public:
    DoubleClickTestLabel();
    ~DoubleClickTestLabel();

public:
    QDBusInterface * mDoubleClickTestDbus;
    QString picOn = ":/img/plugins/mouse/double-click-on-white.png";
    QString picOff = ":/img/plugins/mouse/double-click-off-white.png";

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
};

class MyTextEdit : public QLineEdit
{
    Q_OBJECT

public:
    void startCursor(void);
};

class MouseUI : public QWidget
{
    Q_OBJECT

public:
    explicit MouseUI(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~MouseUI();

    void settingForIntel();

public:
    inline RadioButtonWidget *dominantHandWidget()
    {
        return mDominantHandWidget;
    }

    inline QRadioButton *dominantHandLeftRadio()
    {
        return mDominantHandLeftRadio;
    }

    inline QRadioButton *dominantHandRightRadio()
    {
        return mDominantHandRightRadio;
    }

    inline RadioButtonWidget *scrollDirectionWdiget()
    {
        return mScrollDirectionWidget;
    }

    inline QRadioButton *scrollDirectionForwardRadio()
    {
        return mScrollDirectionForwardRadio;
    }

    inline QRadioButton *scrollDirectionReverseRadio()
    {
        return mScrollDirectionReverseRadio;
    }

    inline SliderWidget *wheelSpeedWidget()
    {
        return mWheelSpeedWidget;
    }

    inline SliderWidget *doubleClickIntervalWidget()
    {
        return mDoubleClickIntervalWidget;
    }

    inline SliderWidget *pointerSpeedWidget()
    {
        return mPointerSpeedWidget;
    }

    inline SwitchWidget *tailLightEffectWidget()
    {
        return mMouseTailLightEffectWidget;
    }

    inline SwitchWidget *mouseAccelerationWidget()
    {
        return mMouseAccelerationWidget;
    }

    inline ComboxWidget *pointerPositionWidget()
    {
        return mPointerPositionWidget;
    }

    inline QButtonGroup *pointerSizeButtonGroup()
    {
        return m_pPointerSizeGroup;
    }

    inline ThemeButton *pointerSizeSmallRadio()
    {
        return m_pPointerSizeSmallBtn;
    }

    inline ThemeButton *pointerSizeMediumRadio()
    {
        return m_pPointerSizeMediumBtn;
    }

    inline ThemeButton *pointerSizeLargeRadio()
    {
        return m_pPointerSizeLargeBtn;
    }

    inline SwitchWidget *blinkingCursorOnTextWidget()
    {
        return mBlinkingCursorOnTextWidget;
    }

    inline SliderWidget *cursorSpeedWidget()
    {
        return mCursorSpeedWidget;
    }

private:
    RadioButtonWidget *mDominantHandWidget;    // 惯用手
    RadioButtonWidget *mScrollDirectionWidget; // 滚动方向
    SliderWidget *mWheelSpeedWidget;           // 滚轮速度
    SliderWidget *mDoubleClickIntervalWidget;  // 双击间隔时长
    SliderWidget *mPointerSpeedWidget;         // 指针速度
    SwitchWidget *mMouseAccelerationWidget;    // 鼠标加速
    SwitchWidget *mMouseTailLightEffectWidget; // 尾灯效果
    ComboxWidget *mPointerPositionWidget;      // 按 ctrl 键时显示指针位置
    UkccFrame *mPointerSizeWidget;      // 指针大小
    SwitchWidget *mBlinkingCursorOnTextWidget; // 文本区域闪烁光标
    SliderWidget *mCursorSpeedWidget;          // 光标速度

    TitleLabel *mMouseTitleLabel;
    TitleLabel *mPointerTitleLabel;
    TitleLabel *mCursorTitleLabel;

    QRadioButton *mDominantHandLeftRadio;
    QRadioButton *mDominantHandRightRadio;
    QRadioButton *mScrollDirectionForwardRadio;
    QRadioButton *mScrollDirectionReverseRadio;

    KLabel *m_pPointerSizeLabel = nullptr;
    ThemeButton *m_pPointerSizeSmallBtn;
    ThemeButton *m_pPointerSizeMediumBtn;
    ThemeButton *m_pPointerSizeLargeBtn;
    QButtonGroup *m_pPointerSizeGroup;

    QVBoxLayout *mVlayout;

private:
    void initUI();
    void setDominantHandFrame();
    void setScrollDirectionFrame();
    void setWheelSpeedFrame();
    void setDoubleClickFrame();
    void setPointerSpeedFrame();
    void setMouseAccelerationFrame();
    void setTailLightEffectFrame();
    void setPointerPositionFrame();
    void setPointerSizeFrame();
    void setBlinkCursorFrame();
    void setCursorSpeedFrame();
};
#endif // MOUSEUI_H
