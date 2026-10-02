#ifndef UKCCFRAME_H
#define UKCCFRAME_H

#include <QFrame>
#include <QPainterPath>

#include "libukcc_global.h"
#include <QtDBus/QDBusInterface>

#define RADIUS_VALUE 6 /**< 圆角数值 */
#define PC_HEIGHT 60 /**< pc模式下高度 */
#define TABLET_HEIGHT 64 /**< 平板模式下高度 */

class LIBUKCC_EXPORT UkccFrame : public QFrame {
    Q_OBJECT
public:
    enum BorderRadiusStyle {
        Top,   /**< 顶部有圆角 */
        None,  /**< 没有圆角 */
        Bottom, /**< 底部有圆角 */
        Around /**< 四周都有圆角 */
    };
    /**
     * @brief 构造函数
     *
     * @param parent 父对象
     * @param style 圆角风格枚举值
     * @param heightAdaptive 是否自动调整高度，默认PC下60，平板模式64
     */
    explicit UkccFrame(QWidget* parent = nullptr, BorderRadiusStyle style = Around, bool heightAdaptive = false);
    ~UkccFrame();
    /**
     * @brief 设置圆角风格
     *
     * @param style 圆角风格枚举值
     */
    void setRadiusType(BorderRadiusStyle style);

    /**
     * @brief 获取平板模式
     *
     * @return bool 是否是平板模式
     */
    inline bool getTabletMode()
    {
        return mIsTabletMode;
    }

    /**
     * @brief 获取是否是一个容器frame，此时settinggroup需要设置的是其子frame的圆角
     *
     * @return bool 是/不是
     */
    inline bool iscontainer()
    {
        return mIscontainer;
    }

    /**
     * @brief 设置该frame是否是一个容器frame,如果是同时setFrameShape(NoFrame)；
     *
     * @param b 是/不是
     */
    inline void setContainer(bool b = false)
    {
        mIscontainer = b;
        if (b) {
            this->setFrameShape(QFrame::NoFrame);
        }
    }


Q_SIGNALS:
    void tabletModeChanged(bool b);

private Q_SLOTS:
    void mode_change_signal_slots(bool b);

protected:
    void paintEvent(QPaintEvent *e);

private:
    BorderRadiusStyle mRadiusType;
    bool mIsTabletMode = false;
    QDBusInterface *mStatusSessionDbus = nullptr;
    bool mHeightAdaptive = false;
    bool mIscontainer = false;
};



#endif // UKCCFRAME_H
