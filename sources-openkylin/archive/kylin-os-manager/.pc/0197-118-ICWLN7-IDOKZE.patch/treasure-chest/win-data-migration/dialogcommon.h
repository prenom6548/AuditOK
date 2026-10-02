#ifndef DIALOGCOMMON_H
#define DIALOGCOMMON_H

#include "constant.h"
#include <kdialog.h>

class DialogCommon : public kdk::KDialog
{
    Q_OBJECT
public:
    DialogCommon(QWidget *parent = nullptr);
};

#endif // DIALOGCOMMON_H
