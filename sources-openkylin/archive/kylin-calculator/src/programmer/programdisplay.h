#ifndef PROGRAMDISPLAY_H
#define PROGRAMDISPLAY_H

#define HIS_NUM_MAX 3

#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QString>
#include <QVector>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "widgetstyle.h"

enum{
    LAB_CURRENT_EXPRESSION = 0,     // 0:当前表达式显示
    LAB_BUDGET_RESULTS = 1,         // 1:预算显示
    LAB_HISTOR_RECORDS = 2,         // 2:历史记录显示
    LAB_CHAR_CODE = 3,              // 3:字符编码显示
};

class ProgramDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit ProgramDisplay(QWidget *parent = nullptr);

    // 设置历史记录显示
    void setHisLab(QString value);

    // 设置预算显示
    void setBudLab(QString value);

    // 设置表达式显示
    void setCurLab(QString value);

    // 设置字符显示
    void setCodeLab(QString value);

    // 返回当前显示数据
    QVector<QString> data();

    // 清空显示
    void clearLab();

    // 浅色模式样式
    void setLightUI();

    // 深色模式样式
    void setDarkUI();

    // 设置显示屏是否可以继续输入
    void setIsInput(bool state);

    // 获取当前显示屏状态
    bool isInput();

private:

    QLabel* m_labHis;
    QLabel* m_labBud;
    QLabel* m_labCur;
    QLabel* m_labCode;

    QVBoxLayout* m_vlayout;
    QHBoxLayout* m_hlayout;

    // 字体颜色
    QString m_fontColor;

    // 是否可以继续输入，超出最大显示字符长度，无法输入
    bool m_isInput = true;

    void init();
    void initLayout();
    // 获取字体合适字号
    int fontSize(QLabel *lab);

signals:

};

#endif // PROGRAMDISPLAY_H
