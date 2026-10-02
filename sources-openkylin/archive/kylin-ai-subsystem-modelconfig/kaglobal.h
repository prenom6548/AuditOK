#ifndef KAGLOBAL_H
#define KAGLOBAL_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QIcon>
#include <QComboBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QMessageBox>
#include <QDesktopServices>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>

#include "libukcc/widgets/SettingWidget/addbutton.h"
#include "libukcc/widgets/SettingWidget/settinggroup.h"


#include <kysdk/applications/kswitchbutton.h>
#include <kysdk/applications/kpasswordedit.h>
#include <kysdk/applications/kborderlessbutton.h>
#include <aiconfig/modelconfig.h>
#include <polkit-qt5-1/polkitqt1-authority.h>

using namespace kyai::config::model;
typedef struct {
    AiCapability capability;
    QString vendor;
    QString modelName;
    std::vector<MultiAuthentication> multiAuths;

} PublicCloudModel;

typedef struct {
    AiCapability capability;
    QString vendor;
    QString modelName;
    std::vector<MultiAuthentication> multiAuths;
    QString debName;
    bool available{true};
} PublicLocalModel;
typedef struct {
    AiCapability capability;
    QString vendor;
    QString modelName;
    QString modelVersion;
    QString apiKey;
    QString apiUrl;
    std::vector<MultiAuthentication> multiAuths;
    bool is3rdModel{false};
} PrivateModel;

enum LocalModelInstallStatus {
    InstallSuccess,
    RebootLater,
    Uninstall,
    Downloding,
};
enum PkgInstallStatus {
    PkgNotFound,
    PkgFoundNotInstall,
    PkgFoundNewVersion,
    PkgLastestVersion
};

class kaglobal
{
public:
    static QUrl openLink;       // 帮助文档链接
    static QString ondeviceModelInstallPath;
    static QString aiAssistantPkgName;
    static QString aiAssistantMinVersion;
    static QString aiNotePkgName;
    static QString aiNoteMinVersion;
    static QString launchpadUrl;
    static QString archiveUrl;
    static QString baiduUrl;
};

#endif // KAGLOBAL_H
