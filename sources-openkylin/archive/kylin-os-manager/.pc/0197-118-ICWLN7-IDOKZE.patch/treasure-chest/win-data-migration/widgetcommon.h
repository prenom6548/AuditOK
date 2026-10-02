#ifndef WIDGETCOMMON_H
#define WIDGETCOMMON_H

#include "dialogmigrationlogs.h"
#include "dialogcommon.h"
#include <kwidget.h>

class WidgetCommon : public kdk::KWidget
{
    Q_OBJECT
public:
    WidgetCommon(QWidget *parent = nullptr);

private slots:
    void migrationLogs();
    void howToUse();

private:
};

#endif // WIDGETCOMMON_H
