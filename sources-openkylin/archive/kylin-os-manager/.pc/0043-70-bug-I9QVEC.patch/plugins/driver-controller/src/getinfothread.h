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
#endif //GETINFOTHREAD_H
