#ifndef KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_TOOL_BOX_H
#define KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_TOOL_BOX_H

#include <QObject>
#include <kom_application_interface.h>

namespace tool_box {

class ToolBox: public KomApplicationInterface {
public:
    ToolBox();
    virtual ~ToolBox();
    virtual void init(void (*frame_callback)(const char *funcName, ...)) override;
    virtual std::string name() override;
    virtual std::string i18nName() override;
    virtual std::string icon() override;
    virtual int sort() override;
    virtual QWidget *createWidget() override;
};

class ToolBoxProvider : public KomApplicationProvider {
public:
    KomApplicationInterface *create() const;
};

}

#endif
