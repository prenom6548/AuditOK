#ifndef SHREDFAILINFODIALOG_H
#define SHREDFAILINFODIALOG_H

#include <QObject>
#include <QWidget>
#include <QTreeWidget>
#include <QFrame>
#include "kdialog.h"
// #include "divider.h"
using namespace kdk;

class ShredFailInfoDialog : public KDialog
{
    Q_OBJECT
public:
    ShredFailInfoDialog(QWidget *parent = nullptr);
    void insertFailContent(QStringList failList);

private Q_SLOTS:
    void setTextToolTips(QTreeWidgetItem *item, int indexCol);

private:
    // Divider *m_divider = nullptr;
    QFrame *m_frame = nullptr;
    QTreeWidget *m_failTable = nullptr;
    QString m_iconName = "";
    void initFailTable();
    bool refreshInfoContents(QString path);
};

#endif // SHREDFAILINFODIALOG_H
