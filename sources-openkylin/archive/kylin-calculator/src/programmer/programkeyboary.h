#ifndef PROGRAMKEYBOARY_H
#define PROGRAMKEYBOARY_H

#include <QWidget>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QGridLayout>
#include <QDebug>
#include <QKeyEvent>

#include "../common/picturetowhite.h"
#include "widgetstyle.h"

class ProgramKeyboary : public QWidget
{
    Q_OBJECT
public:
    explicit ProgramKeyboary(QWidget *parent = nullptr);

    // 根据模式设置按钮是否可用
    void setBtnEnable(int base);

    // 匹配数字按钮
    bool containsNum(QString num);

    // 匹配操作符
    bool containsOp(QString op);

private:
    // 所有按钮标识
    QStringList m_btnList = {"0",   "1",   "2",     "3",    "4",    "5",  "6",     "7",     "8",   "9",
                             "A",   "B",   "C",     "D",    "E",    "F",  "AND",   "OR",    "NOT", "XOR",
                             "NOR", "Lsh", "Rsh",   "RoL",  "RoR",  "XY", "YX",    "ADD",   "SUB", "Multi",
                             "Div", "Rev", "Equal", "LBra", "RBra", "AC", "Clear", "Delete"};
    // 二进制需要禁用的按钮
    QStringList m_btnBinDisList = {"2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};
    // 八进制需要禁用的按钮
    QStringList m_btnOtcDisList = {"8", "9", "A", "B", "C", "D", "E", "F"};
    // 十进制需要禁用的按钮
    QStringList m_btnDecDisList = {"A", "B", "C", "D", "E", "F"};
    // 所有数字按钮
    QStringList m_btnAllNumList = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};
    // 所有操作符以及括号
    QStringList m_btnAllOpList = {"&", "|", "~", "^", "N", "q", "p", "L", "R",
                                  "<", ">", "+", "s", "*", "/", "v", "(", ")"};
    // 运算符按钮对应的运算符号
    QMap<QString, QString> m_symbolMap = {
        {"AND", "&"}, {"OR", "|"},  {"NOT", "~"},   {"XOR", "^"}, {"NOR", "N"},  {"RoL", "L"},
        {"RoR", "R"}, {"Lsh", "q"}, {"Rsh", "p"},   {"Rev", "v"}, {"XY", "<"},   {"YX", ">"},
        {"ADD", "+"}, {"SUB", "s"}, {"Multi", "*"}, {"Div", "/"}, {"LBra", "("}, {"RBra", ")"},
    };
    // 所有按钮
    QMap<QString, QPushButton *> m_btnMap;

    QGridLayout *m_layout;

    PictureToWhite *m_picturetowhite;

    void init();
    void initLayout();

    // 设置列表中所有按钮状态
    void setBtnList(QStringList btnList, bool state);

signals:
    void sigBtnClicked(QString);
public slots:
    void onClicked(void);
    void keyPressEvent(QKeyEvent *event);
};

#endif // PROGRAMKEYBOARY_H
