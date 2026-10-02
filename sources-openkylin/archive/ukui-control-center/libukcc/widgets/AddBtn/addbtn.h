#ifndef ADDBTN_H
#define ADDBTN_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QPainterPath>

#include "libukcc_global.h"

#define THEME_QT_SCHEMA  "org.ukui.style"
#define MODE_QT_KEY      "style-name"

class LIBUKCC_EXPORT AddBtn : public QPushButton
{
    Q_OBJECT
public:
    AddBtn(QWidget *parent = nullptr);
    ~AddBtn();

    enum Shape {
        None,
        Top,
        Bottom,
        Box
    };
    Q_ENUM(Shape)
    void setBtnStyle(Shape type);
    
    QPainterPath makeRoundedRect(const QRect& rect,int radius,int corner);

protected:
    void leaveEvent(QEvent *event);
    void enterEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
    
private:
    int mCorner = 0;

Q_SIGNALS:
    void enterWidget();
    void leaveWidget();
};

#endif // ADDBTN_H
