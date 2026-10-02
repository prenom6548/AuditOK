#ifndef KYLIN_OS_MANAGER_PLUGINS_HOME_PAGE_HOME_PAGE_H
#define KYLIN_OS_MANAGER_PLUGINS_HOME_PAGE_HOME_PAGE_H

#include "kom_application_interface.h"

namespace home_page {

class HomePage: public QObject, public KomApplicationInterface {
    Q_OBJECT
public:
    HomePage();
    ~HomePage();
    void init(void (*frame_callback)(const char *function_name, ...)) override;
    std::string name() override;
    std::string i18nName() override;
    std::string icon() override;
    int sort() override;
    QWidget *createWidget() override;
};

class HomePageProvider: public KomApplicationProvider {
public:
    KomApplicationInterface *create() const override;
};

}

#endif
