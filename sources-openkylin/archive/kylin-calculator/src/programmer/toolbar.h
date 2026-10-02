#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QAction>
#include <QPalette>
#include <QList>
#include <QLabel>

#include "widgetstyle.h"
#include <ktabbar.h>

class ToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBar(QWidget *parent = nullptr);

    // 是否启用进制转换按钮
    void setBaseEnabled(bool state);
    void onBaseTabClicked(int);

private:
    QPushButton *m_btnASCII;
    QPushButton *m_btnUnicode;
    QLabel *m_labCode;
    QPushButton *m_btnBin;
    QPushButton *m_btnMS;
    kdk::KTabBar *m_tabBase;
    QLabel *m_labBase;
    QComboBox *m_boxDigit;
    QAction *m_actionQword;
    QAction *m_actionDword;
    QAction *m_actionWord;
    QAction *m_actionByte;

    QHBoxLayout *m_layoutCode;
    QHBoxLayout *m_layoutBase;
    QHBoxLayout *m_layout;

    // 按钮选中标志位
    bool m_isASCII = false;
    bool m_isUnicode = false;

    void init();
    void initLayout();

signals:
    void sigBtnClicked(QString);
    void sigBoxValueChanged(int);
public slots:
    void onClicked(void);
    void valueChanged(int index); /* 0:64位，1:32位，2:16位，3:8位 */
};

#endif // TOOLBAR_H
