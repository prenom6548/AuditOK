#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H
#include <QFrame>
#include <QLabel>
#include <QSlider>
#include <QHBoxLayout>

#include "fixlabel.h"
#include <kysdk/applications/kslider.h>
#include "libukcc_global.h"
#include "ukccframe.h"

using namespace kdk;
/**
 * @brief 创建一个带 KSlider 的 Frame
 *
 */
class LIBUKCC_EXPORT SliderWidget : public UkccFrame
{
    Q_OBJECT

public:
    /**
     * @brief 创建一个带 KSlider 的 Frame
     *
     * @param title:    text of m_settingName
     * @param hasNode:  是否带节点
     * @param style:   设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    SliderWidget(QString title, bool hasNode, QWidget *parent = nullptr, UkccFrame::BorderRadiusStyle style = UkccFrame::None, Qt::Orientation orientation = Qt::Horizontal);

    /**
     * @brief
     *
     */
    ~SliderWidget();

    /**
     * @brief 设置是否可点击
     * @param checked
     */
    void setEnabled(const bool checked = true);

    /**
     * @brief
     *
     * @return bool
     */
    bool isEnabled() const;

    /**
     * @brief 设置滑动条值
     * @param value
     */
    void setValue(const int value);

    /**
     * @brief 设置 m_settingName text
     * @param title
     */
    void setTitle(const QString &title);

    /**
     * @brief 设置 m_leftLabel text
     * @param text
     */
    void setLeftText(const QString text);

    /**
     * @brief 设置 m_rightLabel text
     * @param text
     */
    void setRightText(const QString text);

    /**
     * @brief 设置 m_leftLabel pixmap
     *
     * @param pixmap
     */
    void setLeftPixmap(const QPixmap &pixmap);

    /**
     * @brief 设置 m_rightLabel pixmap
     *
     * @param pixmap
     */
    void setRightPixmap(const QPixmap &pixmap);

    /**
     * @brief 获取 m_settingName text
     * @return
     */
    QString title() const;

    /**
     * @brief 设置 m_settingName label 宽度
     *
     * @param w
     */
    void setTitleFixedWidth(int w);

    /**
     * @brief 获取 m_slider 值
     * @return int
     */
    int value() const;

    /**
     * @brief 获取 m_slider
     * @return KSlider: m_slider
     */
    KSlider *slider() const;

    /**
     * @brief 获取 m_leftLabel
     * @return QLabel: m_leftLabel
     */
    QLabel *leftLabel() const;

    /**
     * @brief 获取 m_rightLabel
     * @return QLabel: m_rightLabel
     */
    QLabel *rightLabel() const;

    /**
     * @brief 在 mainLayout 中插入序号为 index 的 widget
     *
     * @param index:    插入序号
     * @param widget:   插入控件
     * @param stretch:  stretch factor
     * @param alignment:alignment
     */
    void insertWidget(int index, QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());

private:
    void init();

    QHBoxLayout *mainLayout;
    FixLabel *m_settingName;
    KSlider *m_slider;
    QLabel *m_leftLabel;
    QLabel *m_rightLabel;

private Q_SLOTS:

Q_SIGNALS:
    /**
     * @brief This signal is emitted when the slider value has changed
     *
     * @param value: the new slider value
     */
    void valueChanged(int value);
};

#endif // SLIDERWIDGET_H
