#ifndef RADIOBUTTONWIDGET_H
#define RADIOBUTTONWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QEvent>

#include "fixlabel.h"
#include "libukcc_global.h"
#include "ukccframe.h"

/**
 * @brief 创建一个带 QButtonGroup 的 Frame
 *
 */
class LIBUKCC_EXPORT RadioButtonWidget : public UkccFrame
{
    Q_OBJECT

public:
    /**
     * @brief 创建一个带 QButtonGroup 的 Frame
     *
     * @param title:  text of m_settingName
     * @param style:  设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    explicit RadioButtonWidget(const QString title, QWidget *parent = nullptr, UkccFrame::BorderRadiusStyle style = UkccFrame::None);

    /**
     * @brief
     *
     */
    ~RadioButtonWidget();

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
     * @brief 添加一个 id = id 的 QRadioButton 到 m_buttonGroup，并放到 radioLayout 中
     *
     * @param button:     需要添加的 QRadioButton
     * @param id:         用于 buttonClicked 信号传递的参数
     * @param addStretch: 是否 addStretch
     */
    void addButton(QRadioButton *button, int id = -1, bool addStretch = false);

    /**
     * @brief setSpacing of radioLayout
     *
     * @param spacing
     */
    void setButtonSpacing(int spacing);

    /**
     * @brief 获取 m_buttonGroup
     *
     * @return QButtonGroup: m_buttonGroup
     */
    QButtonGroup *buttonGroup() const;

    QHBoxLayout *mainHLayout() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void init();
    // Tab 只落在当前选中项；方向键在组内只移焦
    void refreshGroupKeyboardNav();
    void moveFocusInGroup(QRadioButton *from, int key);

    QHBoxLayout *mainLayout;
    FixLabel *m_settingName;
    QButtonGroup *m_buttonGroup;
    QHBoxLayout *radioLayout;

private Q_SLOTS:

Q_SIGNALS:
    /**
     * @brief 点击 QButtonGroup 中的 QRadioButton 信号
     *
     * @param int: addButton 时的 id
     */
    void buttonClicked(int);
};

#endif // RADIOBUTTONWIDGET_H
