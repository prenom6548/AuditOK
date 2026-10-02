#ifndef COMBOXWIDGET_H
#define COMBOXWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QComboBox>

#include "fixlabel.h"
#include "libukcc_global.h"
#include "ukccframe.h"

/**
 * @brief 创建一个带 QComboBox 的 Frame
 *
 */
class LIBUKCC_EXPORT ComboxWidget : public UkccFrame
{
    Q_OBJECT

public:
    /**
     * @brief 创建一个带 QComboBox 的 Frame
     *
     * @param title:  text of m_settingName
     * @param style:  设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    explicit ComboxWidget(const QString title, QWidget *parent = nullptr, UkccFrame::BorderRadiusStyle style = UkccFrame::None);
    /**
     * @brief
     *
     */
    ~ComboxWidget();

    /**
     * @brief 设置 m_combox 是否可选
     * @param checked
     */
    void setEnabled(const bool checked = true);

    /**
     * @brief m_combox 是否 enabled
     *
     * @return bool 是：true，否：false
     */
    bool isEnabled() const;

    /**
     * @brief 设置 m_settingName text
     * @param title
     */
    void setTitle(const QString &title);

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
     * @brief 设置 QComboBox 当前 text
     *
     * @param text
     */
    void setCurrentText(const QString &text);

    /**
     * @brief 设置 QComboBox 当前 index
     *
     * @param index
     */
    void setCurrentIndex(int index);

    /**
     * @brief 获取 m_combox
     *
     * @return QComboBox:  m_combox
     */
    QComboBox *comboBox() const;

private:
    void init();

    QHBoxLayout *mainLayout;
    FixLabel *m_settingName;
    QComboBox *m_combox;


private Q_SLOTS:

Q_SIGNALS:
    /**
     * @brief QComboBox 当前 index 改变信号
     *
     * @param index
     */
    void currentIndexChanged(int index);

    /**
     * @brief QComboBox 当前 text 改变信号
     *
     * @param index
     */
    void currentTextChanged(const QString &text);

};

#endif // COMBOXWIDGET_H
