#ifndef BLUETOOTHOFFSENDFILEMESSAGEBOX_H
#define BLUETOOTHOFFSENDFILEMESSAGEBOX_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QMessageBox>


class BluetoothOffSendFileDialog : public QMessageBox
{
    Q_OBJECT
public:
    BluetoothOffSendFileDialog();
    ~BluetoothOffSendFileDialog();

protected slots:
    void bt_accepted();
    void bt_rejected();

protected:
    QPushButton * m_cancel = nullptr;
    QPushButton * m_ok = nullptr;

};

#endif // BLUETOOTHOFFSENDFILEMESSAGEBOX_H
