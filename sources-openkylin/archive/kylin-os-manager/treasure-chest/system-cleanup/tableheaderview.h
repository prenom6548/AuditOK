#ifndef TABLEHEADERVIEW_H
#define TABLEHEADERVIEW_H

#include <QObject>
#include <QHeaderView>
#include <QPainter>
#include <QCheckBox>
#include <QObject>

#include <QDebug>

class TableHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    //构造函数， 第一个参数设定表头方向
    explicit TableHeaderView(QStringList list, Qt::Orientation orientation, QWidget *parent = nullptr);
    void setHeaderCheckBoxCheckEnable(bool checkEnable);

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const;
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool event(QEvent *event);

private:
    //checkbox的开启或关闭状态
    bool m_checkable; // 是否可以被点击
    bool m_bTristate;// 是否表头被点亮（三态）
    bool m_bNoChange;// 是否含有部分没有选中
    bool m_bMoving;// 
    bool m_bChecked;// 是否表头被点亮
    bool m_bPressed;// 

    QStringList m_HeaderList;
    int m_iSelectedCol;  //点击选中的标题列
    
public Q_SLOTS:
    void slot_checkStateChange(int iState);
Q_SIGNALS:
    void signal_headerCheckStateChanged(int state);
};

#endif 