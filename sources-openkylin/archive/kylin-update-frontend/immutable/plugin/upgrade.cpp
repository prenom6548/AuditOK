#include "upgrade.h"

#include <QWidget>
#include <QMainWindow>

Upgrade::Upgrade() :mFirstLoad(true) {
    QTranslator *translator = new QTranslator (this);
    qInfo()<<"the system.name is "<<QLocale::system().name();
    translator->load("/usr/share/upgrade/translations/" + QLocale::system().name());
    QApplication::installTranslator(translator);
    pluginName = tr("Upgrade");
    pluginType = UPDATE;
}

Upgrade::~Upgrade(){
    QFile File;
    File.remove("/tmp/upgrade/upgrade.list");
}

QString Upgrade::plugini18nName(){
    return pluginName;
}

int Upgrade::pluginTypes(){
    return pluginType;
}


QWidget *Upgrade::pluginUi(){
    if (mFirstLoad) {
        mFirstLoad = false;
        // will delete by takewidget
        pluginWidget = new UpgradeMain("");
    }
    return pluginWidget;
}

bool Upgrade::isEnable() const{
    QString version;
    version =kdk_system_get_projectName();
    if("V10SP1-edu" == version)
        return false;
    else
        return true;
}

const QString Upgrade::name() const{
    return QStringLiteral("Upgrade");
}

bool Upgrade::isShowOnHomePage() const{
    return true;
}

QIcon Upgrade::icon() const{
    return QIcon::fromTheme("software-update-available-symbolic");
}

QString Upgrade::translationPath() const{
    return "/usr/share/upgrade/translations/%1.ts";
}

void Upgrade::initSearchText(){
    //~ contents_path /Upgrade/SystemUpdate
    tr("SystemUpdate");
    //~ contents_path /Upgrade/Automatically updates
    tr("Automatically updates");
    //~ contents_path /Upgrade/View history
    tr("View history");
    //~ contents_path /Upgrade/Update Settings
    tr("Update Settings");
    //~ contents_path /Upgrade/Allowed to renewable notice
    tr("Allowed to renewable notice");
}
