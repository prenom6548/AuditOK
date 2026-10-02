#include "theme_util.h"
#include <kylin_system/window_management.hpp>

themeHelper::themeHelper(){}
themeHelper::~themeHelper(){}
void themeHelper::setTheme(QWidget *widget)
{
    if(widget!=nullptr){
        kdk::kabase::WindowManagement::setWindowMotifHint(widget->winId());
    }
}
