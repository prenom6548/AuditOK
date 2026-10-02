#ifndef ADDBUTTON_H
#define ADDBUTTON_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QPainterPath>
#include <QDBusReply>

#include "libukcc_global.h"
#include "ukccframe.h"

#define TOP_LEFT_CORNER 1
#define TOP_RIGHT_CORNER 2
#define BOTTOM_RIGHT_CORNER 4
#define BOTTOM_LEFT_CORNER 8
#define THEME_QT_SCHEMA  "org.ukui.style"
#define MODE_QT_KEY      "style-name"

class LIBUKCC_EXPORT AddButton : public QPushButton
{
    Q_OBJECT
public:
    explicit AddButton(QWidget* parent = nullptr, UkccFrame::BorderRadiusStyle style = UkccFrame::BorderRadiusStyle::Bottom, bool heightAdaptive = true);
    ~AddButton();

    void setRadiusType(UkccFrame::BorderRadiusStyle style);
    /**
     * @brief 获取平板模式
     *
     * @return bool 是否是平板模式
     */
    inline bool getTabletMode()
    {
        return m_isTabletMode;
    }
protected:
    void paintEvent(QPaintEvent *event);
    QPainterPath getPaintrPath();

Q_SIGNALS:
    void tabletModeChanged(bool b);

private Q_SLOTS:
    void mode_change_signal_slots(bool b);

private:
    UkccFrame::BorderRadiusStyle m_radiusType;
    bool m_isTabletMode = false;
    QDBusInterface *m_statusSessionDbus = nullptr;
    bool m_heightAdaptive = false;

};


#endif // ADDBUTTON_H
