#ifndef BINARYKEYBOARY_H
#define BINARYKEYBOARY_H

#include <QWidget>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QList>

#include "basebinary.h"

class BinaryKeyboary : public QWidget
{
    Q_OBJECT
public:
    explicit BinaryKeyboary();

    // 设置需要显示的二进制
    void setData(QString value);

    // 返回当前显示的数据
    QString data();

    void clear();

private:
    // 存放显示四位二进制的基本控件指针
    QList<BaseBinary *> m_baseList;

    QHBoxLayout* m_topLayoout;
    QHBoxLayout* m_bottomLayout;
    QVBoxLayout* m_vlayout;

    void init(void);


signals:

};

#endif // BINARYKEYBOARY_H
