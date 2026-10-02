#ifndef TITLE_BAR
#define TITLE_BAR

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPropertyAnimation>

#include "menumodule.h"
#include "basicbutton.h"

class QLabel;
class QPushButton;

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = 0);
    ~TitleBar();

    // 设置模式或功能名称
    void setFuncLabel(QString label);

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    // 设置最大化按钮状态
    void setMaxBtnMode(bool isShowMax);

    QPushButton *m_pIconBtn;   // 左上角应用图标
    QLabel *m_pFuncLabel;      // 界面标识
    QPushButton *m_pTopButton; // 界面置顶按钮
    menuModule *menuBar;
    QPushButton *m_pMinimizeButton; // 最小化按钮
    QPushButton *m_pMaximizeButton; // 最大化按钮
    QPushButton *m_pCloseButton;    // 关闭按钮

    // 模式或功能名称
    QString STANDARD_LABEL;
    QString SCIENTIFIC_LABEL;
    QString EXCHANGE_RATE_LABEL;
    QString PROGRAMMER_LABEL;

    /* snow revised it in 2021-07-17 10:18 */
public:
    void createInterUi(void);
    void createInterStyle(void);
    void changeModeIcon(void);

    QPushButton *m_Icon;
    QPushButton *m_mode;
    QLabel *m_modeText;
    QLabel *m_modeIcon;
    QHBoxLayout *m_hlayout1;
    IntelModeList *m_modeList;
    QHBoxLayout *hlayout;
    QVBoxLayout *m_vlayout;

    QPushButton *m_min;
    QPushButton *m_max;
    QPushButton *m_close;

public slots:
    void slotModeChange(void);
    void slotChangeStandard(void);
    void slotChangeScientific(void);

signals:
    void sigModeChange(QString);
    void sigFontUpdate(); //更新字号


protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
private slots:

    // 进行置顶、最小化、关闭操作
    void onClicked();

private:
    QPoint m_start; //最大化之前窗口位置
};

#endif // TITLE_BAR
