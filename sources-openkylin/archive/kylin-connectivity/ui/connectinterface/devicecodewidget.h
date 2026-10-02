#ifndef DEVICECODEWIDGET_H
#define DEVICECODEWIDGET_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDebug>
#include <QKeyEvent>
#include <QDBusInterface>
#include "devicecodeitem.h"

class DeviceCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceCodeWidget(QWidget *parent = nullptr);

    void setInput();

Q_SIGNALS:
    void sigDeviceCode(QString);

private:
    void initUI();
    void showKeyboard();
    void hideKeyboard();

private Q_SLOTS:
    void slotTextInput(QString);
    void slotSendDeviceCode();
    void slotBackspace();
    void slotFocus();

private:
    enum MaxCode { Num = 8 };
    DeviceCodeItem *m_codeLines[MaxCode::Num];
    int m_currentCodeIndex = 0;
    QPushButton *m_startConnectBtn = nullptr;
    QDBusInterface *m_keyboardInterface = nullptr;
};

#endif  // DEVICECODEWIDGET_H
