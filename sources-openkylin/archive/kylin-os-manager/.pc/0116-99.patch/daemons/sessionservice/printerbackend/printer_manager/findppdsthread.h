#ifndef FINDPPDSTHREAD_H
#define FINDPPDSTHREAD_H

#include <QObject>
#include <QApplication>
#include <QMetaType>
#include <QMutexLocker>
#include <QTimer>
#include "cupsconnection4ppds.h"


class PPDsAndAttr
{
public:
    QString ppdname;
    QString ppd_make;
    QString ppd_product;
    QString ppd_make_and_model;
    QString ppd_device_id;
    QString match_str;
};
Q_DECLARE_METATYPE(PPDsAndAttr)

enum class RequestError:int
{
    PPDS_GOT_SUCCESS = 0,
    CUPS_REQUEST_ERROR,
    IPP_REQUEST_ERROR,
    GOT_NO_PPDS,
};

using ppdPrinterMap = QMap<QString,QMap<QString,PPDsAndAttr> > ;

class FindPPDsThread : public QObject
{
    Q_OBJECT
public:
    static FindPPDsThread* getInstance(http_t* httpConnection);

protected:


signals:
    void gotAllHandledPPDs(const ppdPrinterMap &origin,const ppdPrinterMap &extra);
    void ippRequestError(int);
    void getPPDsFromCUPSErr();
    void ppdRequestSuccessed();

    void start();

public slots:
    void initPPDMapConstruct();

    void detectStart();
private:
    ~FindPPDsThread();
    explicit FindPPDsThread(http_t* httpConnection, QObject *parent = nullptr);
    FindPPDsThread(FindPPDsThread const&);
    FindPPDsThread& operator =(FindPPDsThread const&);

    static QMutex m_mutex;
    http_t* m_newHttp = nullptr;
    ipp_t* m_ppdRequest = nullptr;
    ipp_t* m_cupsAnswer = nullptr;
    ipp_attribute_t *m_attr = nullptr;

    QMap<QString,QMap<QString,PPDsAndAttr> > m_originPPDs;
    QMap<QString,QMap<QString,PPDsAndAttr> > m_deviceIDPPDs;
    int getPPDsFromCUPS();
};

#endif // FINDPPDSTHREAD_H
