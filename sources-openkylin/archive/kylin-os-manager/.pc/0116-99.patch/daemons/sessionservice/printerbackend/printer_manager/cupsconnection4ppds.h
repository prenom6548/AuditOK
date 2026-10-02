#ifndef CUPSCONNECTION4PPDS_H
#define CUPSCONNECTION4PPDS_H

#include <sys/utsname.h>
#include <cups/cups.h>
#include <cups/http.h>
#include <QDebug>
#include <map>
#include <string>
#include <qstring.h>
#include <stdexcept>
#include <QMutex>
#include <QMutexLocker>

class CupsConnection4PPDs
{
public:
//    static CupsConnection4PPDs* getInstance();
//    static QMutex m_mutex;
    http_t* getCupsConnection() const;
    ~CupsConnection4PPDs();
    explicit CupsConnection4PPDs();
protected:

private:
    http_t *m_ppdRequestConnection;
    const char *m_server;
    int m_ippPortNum;
    http_encryption_t m_encryptionType;

//    void CupsPpdsRequest();
};

#endif // CUPSCONNECTION4PPDS_H
