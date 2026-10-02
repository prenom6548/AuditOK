//
// Created by sqp on 2022/5/31.
//

#ifndef KYLIN_CONNECTIVITY_INPUTSOCKET_H
#define KYLIN_CONNECTIVITY_INPUTSOCKET_H


#include <QTcpSocket>

class InputSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit InputSocket(QObject *parent = nullptr);

    virtual ~InputSocket();

protected Q_SLOTS:

    void onReadyRead();

Q_SIGNALS:
    void sigGetInputState();
    void sigGetInputPos(int x, int y);
    void sigLostInputState();
};


#endif // KYLIN_CONNECTIVITY_INPUTSOCKET_H
