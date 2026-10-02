#ifndef SETTINGGROUP_H
#define SETTINGGROUP_H

#include <QWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QDebug>
#include "libukcc_global.h"
#include "ukccframe.h"
#include <QEvent>

/**
 * @brief 创建一个 Spacing 为 1 的垂直布局 Frame
 *
 */
class LIBUKCC_EXPORT SettingGroup : public UkccFrame
{
    Q_OBJECT
public:
    /**
     * @brief 创建一个 Spacing 为 1 的垂直布局 Frame
     *
     * @param Shape: 设置frameShap
     * @param style: 设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    explicit SettingGroup(QWidget *parent = nullptr, Shape shape = QFrame::NoFrame, UkccFrame::BorderRadiusStyle style = UkccFrame::None);

    /**
     * @brief
     *
     */
    ~SettingGroup();

    /**
     * @brief 为 mainLayout 添加控件并更新圆角
     *
     * @param item:  需要添加的控件
     * @param autoUpdateShape:  是否自动更新item圆角样式
     * @param visible 实际未show控件时isVisible总是false，导致圆角无法正常，因此重新设置，应为Qt的BUG
     */
    void addWidget(QFrame *item, bool autoUpdateShape = true, bool visible = true);

    /**
     * @brief 为 mainLayout 添加控件并更新圆角
     *
     * @param index: 序号
     * @param item:  需要添加的控件
     * @param autoUpdateShape:  是否自动更新item圆角样式
     * @param visible 实际未show控件时isVisible总是false，导致圆角无法正常，因此重新设置，应为Qt的BUG
     */
    void insertWidget(int index, QFrame *item, bool autoUpdateShape = true, bool visible = true);

    /**
     * @brief 为 mainLayout 移除控件并更新圆角
     *
     * @param w:  需要移除的控件
     * @param autoUpdateShape:  是否自动更新item圆角样式
     */
    void removeWidget(QWidget *w, bool autoUpdateShape = true);

    /**
     * @brief 更新圆角
     *
     */
    void updateShape();

    /**
     * @brief 获取 mainLayout
     *
     * @return QVBoxLayout mainLayout
     */
    QVBoxLayout *layout();


    /**
     * @brief 设置最后一个frame的圆角风格
     *
     * @param style 圆角风格
     */
    void setLastRadiusStyle(UkccFrame::BorderRadiusStyle style);


    /**
     * @brief 设置仅一个frame时的圆角风格
     *
     * @param style
     */
    void setOneRadiusStyle(UkccFrame::BorderRadiusStyle style);


    /**
     * @brief 获取显示的子控件个数
     *
     * @return int 显示的子控件个数
     */
    int showItemsCount();

    /**
     * @brief 移除并删除所有子控件
     *
     */
    void removeAndDeleteAllWidget();


private:
    /**
     * @brief 递归添加显示出来的ukccFrame
     *
     * @param frame
     */
    void updateShowItemList(UkccFrame *frame);
    /**
     * @brief 递归获取所有ukccFrame
     *
     * @param frame
     */
    void updateAllItemList(UkccFrame *frame);

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    QDBusInterface *m_statusSessionDbus = nullptr;
    QVBoxLayout *mainLayout;
    UkccFrame::BorderRadiusStyle m_lastRadiusStyle = UkccFrame::BorderRadiusStyle::Bottom;
    UkccFrame::BorderRadiusStyle m_OneRadiusStyle = UkccFrame::BorderRadiusStyle::Around;
    QList<UkccFrame *>showItemList;
    QList<UkccFrame *>allItemList;

private Q_SLOTS:

Q_SIGNALS:

};

#endif // SETTINGGROUP_H
