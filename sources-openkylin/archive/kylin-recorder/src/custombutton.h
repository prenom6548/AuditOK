#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QPainter>
#include <QDebug>
#include <QString>

class CustomButton : public QPushButton
{
    Q_OBJECT

    friend class TestMainWindow;
public:
    explicit CustomButton(QString normalPicPath, QString hoverPicPath, QString selectedPicPath,
                          QWidget *parent = nullptr);
    ~CustomButton();

    enum BUTTONSTATE {
        NORMAL = 0X01,   //正常状态
        HOVER = 0X02,    //悬浮状态
        SELECTED = 0X04, //选中状态
        GRAY = 0x06
    };

public:
    /**/
    void setImagePath(QString normalImgPath, QString hoverImgPath, QString selectedImgPath);
    void setButtonEnabled(bool isAvailable, bool isMini);

protected:
    void paintEvent(QPaintEvent *);

    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    QPixmap normalPix;              //正常状态的pix
    QPixmap hoverPix;               //悬浮状态的pix
    QPixmap selectedPix;            //选中状态的pix
    QPixmap grayPix;                //不可点击置灰的pix
    QString m_NormalPicPath = "";   //正常状态的图片资源路径
    QString m_HoverPicPath = "";    //悬浮状态的图片资源路径
    QString m_SelectedPicPath = ""; //选中状态的图片资源路径
    QString m_grayPicPath = "";     //按钮置灰的图片资源路径
    QString m_miniGrayPicPath = ""; // mini按钮置灰的图片资源路径
    BUTTONSTATE m_curState;         //当前状态

    void setNormalPixmap(QString strImagePath);
    void setEnterPixmap(QString strImagePath);
    void setLeavePixmap(QString strImagePath);
    void setPressPixmap(QString strImagePath);
    void setReleasePixmap(QString strImagePath);

    void SetBtnState(BUTTONSTATE state); //设置按钮状态


signals:
};

#endif // CUSTOMBUTTON_H
