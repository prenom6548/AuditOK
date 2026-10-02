#ifndef TITLEBAR_H
#define TITLEBAR_H


#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include "button.h"
#include <QGSettings>
#include "defaultgetset.h"
#include "setting.h"

class TitleBar : public QWidget
{
    Q_OBJECT
public:
  explicit TitleBar(QWidget *parent = 0);
  void titleBarButtonShowOrHide(bool sig);
  ~TitleBar();
  Setting *setWid = nullptr;

//  FuncList *funcList;

  QLabel *m_pFuncLabel = nullptr;           // 界面标识
  QPushButton *m_pTopButton = nullptr;      // 界面置顶按钮
  QToolButton *funcListButton = nullptr;
  // 模式和功能列表按钮
  bool m_leftButtonPressed;       // 鼠标左键按下标记
  bool m_maxButtonPressed;        // 最大化按钮按下
  bool m_setButtonPressed;        //设置按钮按下
  bool need_close_picturepage = false;        // 需要关掉相册
  //QLabel *icon;
  QPushButton *m_pIconBtn = nullptr;
  QPushButton *m_pMinimizeButton = nullptr; // 最小化按钮
  QPushButton *m_pMaximizeButton = nullptr; // 最大化按钮
  QPushButton *m_pCloseButton = nullptr;    // 关闭按钮
  QPoint m_start;                 // 起始点
  QPoint m_end;                   // 结束点
//  static QGSettings *m_pGsettingThemeData; //lhl debug
  QString nowThemeStyle;  //lhl debug
  DefaultGetSet *gsettingItem = nullptr;
  // 设置模式或功能名称
//  void setFuncLabel(QString label);

protected:

    // 进行鼠标界面的拖动
//    virtual void mousePressEvent(QMouseEvent *event);
//    virtual void mouseMoveEvent(QMouseEvent *event);
//    virtual void mouseReleaseEvent(QMouseEvent *event);

Q_SIGNALS:
    void iconButtonSignal();
    void MaxBtn_change();
    void mainWindowClose();
    void close();
    void close_picturePage(bool flag = false);
private Q_SLOTS:

    // 进行置顶、最小化、关闭操作
    void onClicked();
private:

    // 最大化/还原
//    void updateMaximize();
    void dealSystemGsettingChange(const QString);


};

#endif // TITLEBAR_H
