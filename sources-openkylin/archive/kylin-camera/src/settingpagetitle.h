#ifndef SETTINGPAGETITLE_H
#define SETTINGPAGETITLE_H


#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPoint>

class SettingPageTitle : public QWidget
{
    Q_OBJECT
public:
    explicit SettingPageTitle(QWidget *parent = nullptr);
    QLabel *s_pFuncLabel = nullptr;           // 界面标识
//    QLabel *sIcon;
    QPushButton *sIcon = nullptr;
    QPushButton *s_pCloseButton = nullptr;


public Q_SLOTS:
    void closeButton();
};

#endif // SETTINGPAGETITLE_H
