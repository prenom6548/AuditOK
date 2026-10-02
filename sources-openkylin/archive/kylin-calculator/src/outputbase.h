#ifndef OUTPUTBASE_H
#define OUTPUTBASE_H

#define MIN_FONT_SIZE 16
#define OUTPUTBASE_OBJECT_NAME "OutputBase"

#include <QWidget>
#include <QLabel>
#include <QPushButton>

#include "widgetstyle.h"

class OutputBase : public QWidget
{
    Q_OBJECT
public:
    explicit OutputBase(QWidget *parent = 0);
    void setHistoryText(const QString &text);
    void setNowText(const QString &text);
    // 显示输出组件
    QLabel *staLabLast;
    QLabel *staLabNow;
    QLabel *staLabPre;

protected:
    QHBoxLayout *staLabNowLayout;

private:
    QPushButton *staLabNowLeft;
    QPushButton *staLabNowRight;
    QString m_nowTextString;
    //数字开头位置列表
    QList<int> m_indexList;
    //当前左侧显示第几个数字
    int m_listIndex;
    //当前右侧显示第几个数字
    int m_listIndexR;
    bool m_preIsLeft = true;
private slots:
    void onStaLabNowLeftClick();
    void onStaLabNowRightClick();
};

#endif // OUTPUTBASE_H
