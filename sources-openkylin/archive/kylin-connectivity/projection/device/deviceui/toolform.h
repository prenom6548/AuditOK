#ifndef TOOLFORM_H
#define TOOLFORM_H

#include <QPointer>
#include <QWidget>
#include <QPushButton>

#include "basedevice.h"

class BaseDevice;
class ToolForm : public QWidget
{
    Q_OBJECT

public:
    explicit ToolForm(QWidget *parent = nullptr);
    ~ToolForm();

    void setDevice(BaseDevice *device);

public Q_SLOTS:
    void onClicked(void);

private:
    // 绑定槽函数
    void connectInit(void);

private:
    QPointer<BaseDevice> m_device;
    QPushButton *m_menuBtn = nullptr;   // 菜单按钮
    QPushButton *m_homeBtn;             // home按钮
    QPushButton *m_returnBtn = nullptr; // 返回按钮
};

#endif // TOOLFORM_H
