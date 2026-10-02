#ifndef GETINFOTHREAD_H
#define GETINFOTHREAD_H

#include <QThread>

class GetInfoThread : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshScannerInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshGraphicCardInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshPrinterInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshInputDevInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshVideoDevInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshOtherDevInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshDiskvInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshMonitorInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshKeyboardInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshMouseInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshBluetoothInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshNetworkInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

class RefreshCdInfo : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};

#endif //GETINFOTHREAD_H
