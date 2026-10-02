#ifndef KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_BLANKPAGE_H
#define KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_BLANKPAGE_H

#include <future>
#include <QWidget>
#include <QEvent>
#include <QVariant>
#include "text_label.h"
#include "icon_label.h"
#include "defines.h"

namespace tool_box {

class BlankPage: public QWidget {
    Q_OBJECT
public:

    BlankPage(QWidget *parent = nullptr);
    void changeThemeIcon();

private:
    K::IconLabel * icon_label_;
    K::TextLabel * text_label_;

};

}

#endif
