/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QLabel>
#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QRegExp>
#include <QTime>
#include <QVector>
#include <QPoint>
#include <QMenu>
#include <QClipboard>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QEvent>
#include <ukuiwindowhelper/ukuiwindowhelper.h>
#include <libkydatacollect.h>

#include "titlebar.h"
#include "widgetstyle.h"
#include "funclist.h"
#include "standardmodel.h"
#include "scientificmodel.h"
#include "toolmodel.h"
#include "InputProcess.h"
#include "InputSymbols.h"
#include "programmer/programmodel.h"
#include "HorizontalOrVerticalMode.h"

class MainWindow : public QMainWindow, public InputSymbols
{
    Q_OBJECT

public:
    static MainWindow *getInstance();
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    TitleBar *pTitleBar;
    QString currentModel; /* 保存当前计算器模式 */

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    // 公有组件，即标题栏和功能列表
    void setCommonUi();

    // 计算器输出窗口组件
    void setOutputUi();

    // 标准计算界面布局
    void setStandardUi();

    // 科学计算界面布局
    void setScientificUi();

    // 换算器界面布局
    void setToolUi();

    // 程序员模式界面布局
    void setProgrammerUi();

    // 计算器界面切换布局
    void changeCalculatorUi();

    // 换算器界面切换布局
    void changeToolUi();

    // 重置输入界面字号
    void resetFontSize(QString calModel, QString fontSizeStr);

    // 显示输出
    void lab_output(QString s, QLabel *l);
    void updateOutput(QVector<QString> outVector);

    // 添加千分位
    QString addComma(QString s);

    // 换算器输入响应事件
    void unitCalc();

    // 将用于显示的表达式格式化为用于运算的表达式
    QString formatDisToCal(QString text);

    // 切换深色主题
    void changeDarkTheme();

    // 切换浅色主题
    void changeLightTheme();
    QPoint mMovePosition;
    bool mMoveing;

    // 判断字符串是否为纯数字
    bool isDigitStr(QString str);

    // 最小化状态下拉起主界面
    void pullUpWindow();

    /* 初始化gsetting */
    void initGsetting(void);
    QDBusInterface *m_intelModeDbus = nullptr;

    /* 设置是否平板模式 */
    void setIsTabletMode(bool isTabletMode);

    /* 是否平板模式 */
    bool isTabletMode();

    /* 设置当前界面的大小*/
    void setWindowSize();

public slots:
    // 键盘响应事件
    void keyPressEvent(QKeyEvent *event);

    void btn_merge(const QString &);
    // 处理按钮点击事件
    void btn_handler(bool);

    // 科学模式下专属按钮点击事件
    void sciBtnHandler(bool);

    // del按钮点击事件
    void delete_btn_handle(bool);

    // 功能列表按钮点击事件
    void funcListHandle(bool);

    // 功能列表项点击事件
    void funcListItemClicked(QListWidgetItem *);

    // 窗口置顶按钮点击事件
    void stayTop();

    // 汇率换算事件
    void unitConversion();

    void fontUpdate();

    void changeModel(QString);

    // 响应右键菜单
    void myCustomContextMenuRequested(const QPoint &pos);

    // 复制计算结果到剪切板
    void copyCalResult();

    // 将剪切板内容粘贴到运算式中
    void pasteToLabNow();

    /* 平板中的自动屏幕旋转on/off */
    void slotIntelModeChanged(bool);

    /* 平板模式切换 */
    void tabletModeChange(bool isTabletMode);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void changeEvent(QEvent *event);

private:
    void setIntelModeChange(deviceMode mode);
    OutputBase *getOutputParent(QObject *obj);
	bool isVaildExpression(QString expression);
	void pasteError(void);

    // 监控是否平板的dbus接口
    QDBusInterface * m_statusSessionDbus = nullptr;

    // 是否平板模式
    bool m_isTabletMode = false;

    //平板中的横竖屏切换部分
    deviceMode hLayoutFlag = PCMode;              //默认横屏
    HorizontalOrVerticalMode *hOrVMode = nullptr; //平板横竖屏Dbus的信号监听及接口调用
    bool m_isPcModeStarted = true;
    bool m_isSwitchMode = false;

    int m_hisNumMax = PC_HIS_NUM;

    Qt::WindowFlags winFlags;

    QLabel *lab_last;
    QLabel *lab_now;
    QLabel *lab_prepare;

    QMenu *labelMenu;     // 右键菜单
    QAction *copyAction;  // 复制
    QAction *pasteAction; // 粘贴

    QStackedWidget *stackedModelWid;
    QStackedWidget *stackedToolWod;

    QPushButton *btnNum[10];
    QPushButton *btnClear;
    QPushButton *btnDiv;
    QPushButton *btnMulti;
    QPushButton *btnDelete;
    QPushButton *btnSub;
    QPushButton *btnAdd;
    QPushButton *btnEqual;
    QPushButton *btnPer;
    QPushButton *btnPoint;

    // 整体界面布局
    QVBoxLayout *mainLayout;

    QWidget *mainWid;
    QWidget *titleBarWid = nullptr;
    QWidget *outputWid = nullptr;
    QWidget *buttonWid = nullptr;
    QWidget *funcListWid;

    QVBoxLayout *mainTitleLayout;
    QVBoxLayout *mainOutputLayout;
    QVBoxLayout *mainButtonLayout;

    FuncList *funcList;

    StandardOutput *standardOutput = nullptr;
    StandardModel *standardModel = nullptr;
    TitleBar *standardTitle = nullptr;

    ScientificOutput *scientificOutput = nullptr;
    ScientificModel *scientificModel = nullptr;
    TitleBar *scientificTitle = nullptr;

    ToolModelOutput *toolModelOutput = nullptr;
    ToolModelButton *toolModelButton = nullptr;

    ProgramModel *m_prograModel = nullptr;


    QString dis_data;
    QString result;
    QString num_now;

    QString disData; /* 用与显示的表达式 */
    QString calData; /* 用于运算的表达式 */

    // 标识输入到最长
    bool inputLongSym;

    /* 记录计算历史记录,输入'='时触发 */
    QVector<QString> disHistory;

    // 剪切板
    QClipboard *clipboard = QApplication::clipboard();

    UkuiWindowHelper *m_windowHelper = nullptr;
signals:
    void sigTranparencyChange(void);
};

#endif // MAINWINDOW_H
