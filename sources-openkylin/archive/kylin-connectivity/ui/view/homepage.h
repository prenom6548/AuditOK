#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "publicattributes.hpp"
class HomePage : public QWidget
{
    Q_OBJECT
public:
    HomePage(QWidget *parent = nullptr);

    void setTheme(PublicAttributes::Theme theme);
    void setDeviceCode(QString code = "");
    void changeFontSize(double fontSize);

Q_SIGNALS:
    void sigConnectBtnClicked();

private:
    QLabel *m_homePageLab = nullptr;
    QLabel *m_titleLab = nullptr;
    QLabel *m_describeLab = nullptr;
    QLabel *m_deviceCodeLab = nullptr;
    QLabel *m_codeLab = nullptr;

    QLabel *m_folderIconLab = nullptr;  //文件浏览Icon
    QLabel *m_shareIconLab = nullptr;   //共享屏幕Icon
    QLabel *m_crossIconLab = nullptr;   //跨屏操控Icon
    QLabel *m_folderLab = nullptr;      //文件浏览
    QLabel *m_shareLab = nullptr;       //共享屏幕
    QLabel *m_crossLab = nullptr;       //跨屏操控

    QPushButton *m_connectBtn = nullptr;
};

#endif  // HOMEPAGE_H
