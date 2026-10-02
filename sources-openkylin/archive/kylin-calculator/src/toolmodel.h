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

#ifndef TOOLMODEL_H
#define TOOLMODEL_H

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QFont>
#include <QLabel>
#include <QWidget>
#include <QListWidget>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTime>
#include <QDateTime>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QEventLoop>
#include <QRegExp>
#include <QEvent>
#include <QLocale>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QThread>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>

#include "widgetstyle.h"
#include "basicbutton.h"
#include "InputSymbols.h"
#include "../common/picturetowhite.h"
//#include "cal.h"

class UpdateRateThread;
class UnitListWidget;

// 换算器输入输出界面
class ToolModelOutput : public QWidget, public InputSymbols
{
    Q_OBJECT

public:
    explicit ToolModelOutput(QWidget *parent = 0);
    ~ToolModelOutput() {}

    // 当前功能标识
    QString funName;

    // 汇率更新
    QLabel *toolLabUpdate;
    QToolButton *toolIconUpdate;
    QLabel *toolLabTime;
    QLabel *toolLabRate;

    // 单位换算比率
    double toolDouRate;
    // 获取汇率信息字符串键值对
    QStringList strRateList;
    // 货币及其符号对应
    QMap<QString, QString> rateMap;
    // 符号及其货币的对应关系
    QMap<QString, QString> currencyInfoUS;
    QMap<QString, QString> currencyInfoCN;

    QVBoxLayout *toolUpdateLayout;
    QVBoxLayout *timeRateLayout;
    QWidget *toolUpdateWid;

    // 单位标识
    QString toolRateNameBef;
    QString toolRateSymbBef;
    QLabel *toolLabUnitBef;
    QLabel *toolLabIconBef;

    QString toolRateNameAft;
    QString toolRateSymbAft;
    QLabel *toolLabUnitAft;
    QLabel *toolLabIconAft;

    QHBoxLayout *toolUnitBefLayout;
    QHBoxLayout *toolUnitAftLayout;

    QPushButton *toolUnitBefWid;
    QPushButton *toolUnitAftWid;

    // 记录计算历史记录,输入'='时触发
    QVector<QString> toolDisHistory;

    // 换算历史
    QLabel *toolLabHis;
    // 换算前
    QLabel *toolLabBef;
    // 换算后
    QLabel *toolLabAft;

    // 换算前的单位列表
    UnitListWidget *unitListBef;
    // 换算后的单位列表
    UnitListWidget *unitListAft;

    // 第一次初始化默认汇率
    bool m_firstLoad;

    // 更新汇率线程
    UpdateRateThread *updateRateThread;

    PictureToWhite *m_picturetowhite;

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle(bool resetFontSize = true);

    // 初始化单位列表
    void initUnitList(QString listStr);

    // 单位换算函数
    void unitConversion();

    // 单位换算历史记录转换
    QString unitConvHistory(QString hisText);

    // 汇率更新函数
    void updateRate();
    // 默认更新汇率
    void defUpdateRate();


public slots:
    // 控制换算前的单位列表
    void unitListBefShow();

    // 控制换算后的单位列表
    void unitListAftShow();

    void listItemClicked(QListWidgetItem *item);

    // 响应线程汇率更新完成
    void dealDone(QStringList list);

private:
    // 根据符号更新货币名称并显示
    void updateRateName();
};

// 换算器按钮界面
class ToolModelButton : public QWidget
{
    Q_OBJECT

public:
    explicit ToolModelButton(QWidget *parent = 0);

    BasicButton *btnNum[10];

    BasicButton *btnClear;
    BasicButton *btnPer;
    BasicButton *btnDelete;

    BasicButton *btnDiv;
    BasicButton *btnMulti;
    BasicButton *btnSub;
    BasicButton *btnAdd;
    BasicButton *btnEqual;

    // BasicButton *btnDZero;
    BasicButton *btnPoint;

    PictureToWhite *m_picturetowhite;

    // 初始化组件
    void setWidgetUi();

public slots:
    // 键盘响应事件
    void keyPressEvent(QKeyEvent *event);
};

class UnitListWidget : public QWidget
{
    Q_OBJECT

public:
    UnitListWidget(QWidget *parent = nullptr);
    ~UnitListWidget();

    void insertItem(int row, QListWidgetItem *item);
    void clear(void);
    void setSpacing(int space);
    void setCurrentRow(int row);
    QListWidget *getListWidget(void);

protected:
    void paintEvent(QPaintEvent *event);
    // void focusOutEvent(QFocusEvent *event);

private:
    QListWidget *m_listwidget;

signals:
    void itemClicked(QListWidgetItem *item);
};


class UpdateRateThread : public QThread
{
    Q_OBJECT
public:
    explicit UpdateRateThread(QObject *parent = 0) {}
    ~UpdateRateThread() {}
    QStringList getConfigData();

protected:
    // QThread的虚函数
    //线程处理函数
    //不能直接调用，通过start()间接调用
    void run();

private:
    QStringList getHtmlData(const QString &str);
    bool isTableText(const QString &str);
    bool isDateTimeText(const QString &str);
    bool isNumberText(const QString &str);
    QStringList specificationFormat(const QStringList &head, const QStringList &value);
    QStringList CreatIndirectPricing();
    QMap<QString, QString> creatMoneyTypeMap();
    bool getConfigDataSuccess();
    void saveToConfig(const QString &dateTime, const QStringList &result);
    QString m_configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + qAppName();
    QString m_configFileName = "rate.conf";
    int m_moneyType = 25;
    QTimer *m_timer;

signals:
    void isDone(QStringList); //处理完成信号
};

#endif // TOOLMODEL_H
