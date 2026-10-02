#ifndef BASEBINARY_H
#define BASEBINARY_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QList>

class BaseBinary : public QWidget
{
    Q_OBJECT
public:
    explicit BaseBinary(QString value);

    // 设置显示
    // value:一个四位的2进制字符串1，将这个value设置到四个按钮上
    void setData(QString value);

    // 返回当前显示的数据
    QString data();

    // 全部设置为0
    void clear();

private:

    QString m_labValue;
    // 设置二进制位
    QString m_flag0 = QString("0");
    QString m_flag1 = QString("1");

    QList<QLabel *> m_btnList;
    QLabel *m_lab;
    QHBoxLayout *m_hlayout;
    QHBoxLayout* m_spaceLayout;
    QVBoxLayout *m_vlayout;

    void init(void);
    void initLayout(void);

signals:

public slots:
    void onClicked(void);

};

#endif // BASEBINARY_H
