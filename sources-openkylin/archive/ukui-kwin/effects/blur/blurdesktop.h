#ifndef BLUR_INACTIVE_H
#define BLUR_INACTIVE_H

#include <kwineffects.h>
#include <kwinglplatform.h>
#include <kwinglutils.h>
#include <xcbutils.h>

#include <QVector>
#include <QVector2D>
#include <QStack>
#include <QWidget>
#include <QPainter>

#define KAWASE_DOWNSAMPLEITERATIONS 4  //! 下采样的迭代次数
#define KAWASE_KERNAL_OFFSET        4  //! kawase算法渲染核的偏移量

/*!
 * \brief 平板模式下，对激活窗口和任务栏之外的区域进行虚化
 * \author Yunpeng Zhu.
 */
namespace KWin {

class BlurShader;

class BlurDesktopEffect : public Effect
{
    Q_OBJECT
public:
    BlurDesktopEffect();
    ~BlurDesktopEffect() override;

    bool isActive() const override;
    void prePaintScreen(ScreenPrePaintData &data, int time) override;
    void paintScreen(int mask, const QRegion &region, ScreenPaintData &data) override;

private Q_SLOTS:
    void slotTabletModeChanged(bool isTabletMode);
    void slotUpdateBlurWindow();
    void printEffectWindowInfo(EffectWindow *w);
    void slotWindowAdded(KWin::EffectWindow *w);
    void slotWindowClosed(KWin::EffectWindow *w);
    void slotWindowActivated(KWin::EffectWindow *w);
    void slotWindowMinimized(KWin::EffectWindow *w);
    void slotWindowUnminimized(KWin::EffectWindow *w);
    void slotWindowMaximizedStateChanged(KWin::EffectWindow *w, bool horizontal, bool vertical);


private:
    bool isTabletMode() const ;
    void setTabletMode(bool isTabletMode);

    void init();
    void initBlurStrengthValues();
    void initSlotConnection();
    // 判断窗口是否在虚化列表中
    bool isInBlurAllowList(EffectWindow *w);
    KWin::EffectWindow* findBlurHelperWindow();
    bool skipWindow(KWin::EffectWindow *w);
    bool ignoreWindow(KWin::EffectWindow *w);
    void setActive(bool active);

private:
    bool m_bTabletMode;     //! 标记是否是平板模式

    // 记录特殊的需要虚化的桌面组件
    QVector<QString> m_blurAllowList;

    EffectWindow *m_blurHelperWindow;
    bool m_activated;
};

}


#endif // BLUR_INACTIVE_H
