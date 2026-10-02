#include "dialogcommon.h"

DialogCommon::DialogCommon(QWidget *parent) : kdk::KDialog(parent)
{
    setWindowIcon(QIcon::fromTheme(("kylin-win-data-migration")));

    setWindowTitle(Constant::g_toolbarText);
    minimumButton()->show();
}
