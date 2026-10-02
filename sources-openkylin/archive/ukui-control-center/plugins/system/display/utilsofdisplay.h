#ifndef KSCREEN_KCM_UTILS_H
#define KSCREEN_KCM_UTILS_H

#include "ukcccommon.h"
using namespace ukcc;

#include <QString>
#include <QSize>
#include <QStringList>

#include <KF5/KScreen/kscreen/types.h>
#include <KF5/KScreen/kscreen/output.h>

namespace UtilsOfDisplay {
QString outputName(const KScreen::OutputPtr &output);

QString sizeToString(const QSize &size);
}

#endif
