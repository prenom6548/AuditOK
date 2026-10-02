#include "widgetcommon.h"

WidgetCommon::WidgetCommon(QWidget *parent) : kdk::KWidget(parent)
{
    setWindowTitle(Constant::g_toolbarText);
    setLayoutType(kdk::VerticalType);
    setIcon(QIcon::fromTheme(("kylin-win-data-migration")));
    setWidgetName(Constant::g_toolbarText);
    windowButtonBar()->maximumButton()->hide();
    windowButtonBar()->menuButton()->menu()->clear();

    QAction *act1 = new QAction;
    act1->setText(Constant::g_migrationLogText);
    connect(act1, &QAction::triggered, this, &WidgetCommon::migrationLogs);
    windowButtonBar()->menuButton()->menu()->addAction(act1);
    QAction *act2 = new QAction;
    act2->setText(Constant::g_howToSetText);
    connect(act2, &QAction::triggered, this, &WidgetCommon::howToUse);
    windowButtonBar()->menuButton()->menu()->addAction(act2);
}

void WidgetCommon::migrationLogs()
{
    DialogMigrationLogs::getInstance()->updateAndActive();
}

void WidgetCommon::howToUse()
{
    DialogHowToUse::getInstance()->updateAndActive();
}
