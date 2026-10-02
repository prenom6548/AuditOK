#ifndef JUNKCLEANMINI_H
#define JUNKCLEANMINI_H

#include <QObject>
#include "cleaner.h"

class JunkCleanMini : public QObject
{
    Q_OBJECT
public:
    JunkCleanMini(QString cleaner);
    void start();
public Q_SLOTS:
    void on_ScanForJunk(QString mark, junk_clean::JunkItem item);
    void on_ScanFinish(QString mark, quint64 size);
    void on_CleanFinish(QString mark);
private:
    QList<junk_clean::JunkItem> m_itemList;
    junk_clean::Cleaner *m_cleaner = nullptr;
};

#endif // JUNKCLEANMINI_H
