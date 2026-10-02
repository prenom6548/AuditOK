#include <QDebug>

#include <libkydatacollect.h>

#include "common.h"
#include "information_collector.h"

InformationCollector::InformationCollector(/* args */)
{
    connect(this, &InformationCollector::addMessage, [=](const QString &str){
        if (str.isEmpty()) {
            return ;
        }
        sendMessage("message", str);
    });

    connect(this, &InformationCollector::addMessageMap, this, &InformationCollector::sendMessageMap, Qt::QueuedConnection);
}

InformationCollector::~InformationCollector(/* args */)
{

}
void InformationCollector::addPoint(P point) const
{
    QString str;
    switch (point)
    {
    case InformationCollector::P::ManuallyInstallPrinter : str = "ManuallyInstallPrinter"; break;
    case InformationCollector::P::SetResolution          : str = "SetResolution";          break;
    case InformationCollector::P::SetPaperSize           : str = "SetPaperSize";           break;
    case InformationCollector::P::SetPaperType           : str = "SetPaperType";           break;
    case InformationCollector::P::SetCationSource        : str = "SetCationSource";        break;
    case InformationCollector::P::SetDuplexPrint         : str = "SetDuplexPrint";         break;
    case InformationCollector::P::SetInkType             : str = "SetInkType";             break;
    case InformationCollector::P::ADDRemovePrinter       : str = "ADDRemovePrinter";       break;
    case InformationCollector::P::SetShareStartup        : str = "SetShareStartup";        break;
    case InformationCollector::P::PrintTestPage          : str = "PrintTestPage";          break;
    case InformationCollector::P::CancelPrintJob         : str = "CancelPrintJob";         break;
    case InformationCollector::P::DeletePrintJob         : str = "DeletePrintJob";         break;
    case InformationCollector::P::RePrint                : str = "RePrint";                break;
    case InformationCollector::P::ManualyModifyDrive     : str = "ManualyModifyDrive";     break;
    case InformationCollector::P::Rename                 : str = "Rename";                 break;
    default:
        return;
    }
    sendMessage("point", str);
}

void InformationCollector::sendMessageMap(const QString &messageType, const QMap<QString, QString> &messageMap) const
{
#ifndef TEST_DEBUG
    auto *p = new KCustomProperty[messageMap.size()];

    QByteArrayList buff;
    for (int i = 0; i < messageMap.keys().size(); i++) {
        QString key = messageMap.keys().at(i);
        QString value = messageMap[key];

        if (value.size() > 5000) {
            qDebug() << "InformationCollector::sendMessage value too long !" << value;
        }

        buff.append(key.toLocal8Bit());
        p[i].key = buff.back().data();

        buff.append(value.toLocal8Bit());
        p[i].value = buff.back().data();
    }

    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, p, messageMap.size());
    kdk_dia_upload_default(node, QTC(messageType), const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);


    delete []p;
#endif
}

void InformationCollector::sendMessage(const QString &key, const QString &value) const
{
    QMap<QString, QString> messages;
    messages.insert(key, value);
    InformationCollector::getInstance().addMessageMap("function_type", messages);
}

#ifdef TEST_DEBUG
const char *test_qtc(const QString &qstr)
{
    if (qstr.isEmpty())
        return nullptr;
    QByteArray ba = qstr.toLatin1();
    return ba.data();
}

void InformationCollector::test() const
{
    KCustomProperty p[1];
    p[0].key = "test";
    QString value = QString(100000, 'a');
    p[0].value = const_cast<char *>(test_qtc(value));

    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node,p,1);
    kdk_dia_upload_default(node, const_cast<char *>("function_type"), const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);
}
#endif
