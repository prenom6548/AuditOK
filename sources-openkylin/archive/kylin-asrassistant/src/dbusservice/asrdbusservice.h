#ifndef ASRDBUSSERVICE_H
#define ASRDBUSSERVICE_H

#define KYLIN_ASR_INTERFACE_PATH "/"
#define KYLIN_ASR_SERVICE "com.kylinos.asr.assistant.service"

#include <QObject>

class AsrDbusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", KYLIN_ASR_SERVICE)
public:
    AsrDbusService(QObject *parent = nullptr);

public Q_SLOTS:
    void showWindow();
};

#endif  // ASRDBUSSERVICE_H
