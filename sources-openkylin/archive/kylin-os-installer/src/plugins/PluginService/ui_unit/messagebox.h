#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QTextEdit>
#include <QWidget>
#include <QAction>
#include <QScrollBar>
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "clineeditlabel.h"
#include "../../KUserRegister/checkedlineedit.h"
using namespace KInstaller;
namespace KServer {
class MessageBox : public QDialog
{
    Q_OBJECT
public:
    explicit MessageBox( QWidget *parent = nullptr);

    void initUI();                              //加载界面ui
    void initAllConnect();                      //信号和槽链接
    void addStyleSheet();                       //设置样式
    void translateStr();
    void setMessageInfo(QString info);
    void setCancleHidden();
    int execid;
    void setOKInfo(QString strInfo);             //设置m_pOkButton按钮显示的文字
    void clearOKInfo(void);                      //清除对m_pOkButton按钮文字的设置，使其显示默认值文字
    void setBelowLabelColor(QString strInfo);   //设置m_pBelowLabelInfo显示的字体颜色
    void clearBelowLabelColor(void);            //清除对m_pBelowLabelInfo字体颜色的设置，使其显示默认值颜色
signals:
    void signalCancel();
    void signalOK();


private:
    void changeEvent(QEvent *event) override;       //改变语言事件
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *event);

public:
    QWidget *m_pCenterWidget = nullptr;
    QWidget *m_pLabelInfoWidget = nullptr;
    QWidget *m_pIconLabelWidget = nullptr;
    QWidget *m_pCloseButtonWidget = nullptr;
    QWidget *m_pButtonGrupWidget = nullptr;

    QVBoxLayout* m_pmainVlayout = nullptr;

    QLabel *m_pIconlabel = nullptr;
    QLabel *m_pMainLableInfoOne = nullptr;          //显示内容的第一行，白色字体。
    QLabel *m_pBelowLabelInfo = nullptr;            //显示内容的其他行，注意字体默认显示为橘红色

    QPushButton *m_pCloseButton = nullptr;          //右上角的关闭按钮
    bool m_isOnCloseButton = false;                 //控制权是否在此按钮上
    QPushButton *m_pCancelButton = nullptr;         //中间的“取消”按钮
    bool m_isOnCancelButton = false;                //控制权是否在此按钮上
    QPushButton *m_pOkButton = nullptr;             //中间的“确定”按钮
    bool m_isOnOkButton = true;                     //控制权是否在此按钮上
    ArrowWidget *m_tTip;                            //位于m_pCloseButton按钮下方的悬浮框，当鼠标放到此按钮上时显示

    QString m_strOKInfo;                            //m_pOkButton按钮显示信息
    QString m_strBelowLabelColor;                   //m_pBelowLabelInfo显示信息的颜色

signals:
     void  signalClickBtn(int execid);


};

}
#endif // MESSAGEBOX_H
