#ifndef LINEEDITWIDGET_H
#define LINEEDITWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QLineEdit>


#include "fixlabel.h"
#include "libukcc_global.h"
#include "ukccframe.h"
#include <QLine>
/**
 * @brief 创建一个带 QLineEdit 的 Frame
 *
 */
class LIBUKCC_EXPORT LineEditWidget : public UkccFrame
{
    Q_OBJECT

public:
    /**
     * @brief 创建一个带 QLineEdit 的 Frame
     *
     * @param title:  text of m_settingName
     * @param style:  设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    explicit LineEditWidget(const QString title, QWidget *parent = nullptr, UkccFrame::BorderRadiusStyle style = UkccFrame::None);
    /**
     * @brief
     *
     */
    ~LineEditWidget();

    /**
     * @brief 设置 m_lineEdit 是否可点击
     *
     * @param checked
     */
    void setEnabled(const bool checked);

    /**
     * @brief 获取 m_lineEdit 是否 Enabled
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
     * @brief 获取 m_lineEdit
     *
     * @return QLineEdit:  m_lineEdit
     */
    QLineEdit *lineEdit();

private:
    void init();

    QHBoxLayout *mainLayout;
    FixLabel *m_settingName;
    QLineEdit *m_lineEdit;

private Q_SLOTS:

Q_SIGNALS:


};

#endif // LINEEDITWIDGET_H
