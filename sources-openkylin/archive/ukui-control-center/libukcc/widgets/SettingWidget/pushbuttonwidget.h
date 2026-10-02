#ifndef PUSHBUTTONWIDGET_H
#define PUSHBUTTONWIDGET_H

#include <QWidget>
#include <QFrame>

#include <QHBoxLayout>
#include <kpushbutton.h>
using namespace kdk;

#include "fixlabel.h"
#include "libukcc_global.h"
#include "ukccframe.h"

/**
 * @brief 创建一个带 QPushButton 的 Frame
 *
 */
class LIBUKCC_EXPORT PushButtonWidget : public UkccFrame
{
    Q_OBJECT
public:
    /**
     * @brief 创建一个带 QPushButton 的 Frame
     *
     * @param title:  text of m_settingName
     * @param style:  设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    explicit PushButtonWidget(const QString title, QWidget *parent = nullptr, UkccFrame::BorderRadiusStyle style = UkccFrame::None);
    /**
     * @brief
     *
     */
    ~PushButtonWidget();

    /**
     * @brief 设置 m_pushButton 是否可点击
     *
     * @param checked
     */
    void setEnabled(const bool checked);

    /**
     * @brief 获取 m_pushButton 是否可点击
     *
     * @return bool 是：true，否：false
     */
    bool isEnabled() const;

    /**
     * @brief 设置 m_settingName text
     *
     * @param title
     */
    void setTitle(const QString &title);

    /**
     * @brief 获取 m_settingName text
     *
     * @return QString
     */
    QString title() const;

    /**
     * @brief 设置 m_settingName label 宽度
     *
     * @param w
     */
    void setTitleFixedWidth(int w);

    /**
     * @brief 设置 m_pushButton text
     *
     * @param text
     */
    void setButtonText(const QString & text);

    /**
     * @brief 设置 m_pushButton 宽度
     *
     * @param w
     */
    void setButtonFixedWidth(int w);

    /**
     * @brief 设置 m_iconLabel 图标
     *
     * @param icon
     */
    void setIconLabel(QIcon icon);

    /**
     * @brief 获取 m_pushButton
     *
     * @return QPushButton:  m_pushButton
     */
    KPushButton *pushButton();

private:
    void init();

    QHBoxLayout *mainLayout;
    FixLabel *m_settingName;
    KPushButton *m_pushButton;
    QLabel *m_iconLabel;

private Q_SLOTS:

Q_SIGNALS:
    /**
     * @brief m_pushButton 点击信号
     *
     * @param checked
     */
    void clicked(bool checked = false);
};

#endif // PUSHBUTTONWIDGET_H
