#ifndef KSYSTEMSETTING_UNIT_H
#define KSYSTEMSETTING_UNIT_H
#include <QString>
class QString;
class QSettings;
namespace KServer {

struct DiskPartitionSetting
{
    DiskPartitionSetting(){}
    QString rootDisk;
    QString rootPartition;
    QString bootPartition;
    QString mountPoints;
    bool swapFileRequire = false;
    bool uefiRequired = false;
};

struct LanguageSetInINI
{
    LanguageSetInINI(){}
    QString Language;//此语言对应的QLocale::system().name()值。例如中文简体语言对应值：zh_CN
    QString display;//此语言在 安装器选择语言界面中对应的按钮上 显示的文本。例如中文简体语言对应值：中文(简体)
    bool lsInCurrentList = false;//此语言是否在安装器中展示
};

//从systemsetting.ini文件中，读取当前语言设置
//QString ReadSettingIniSet(QString key);

//将语言设置写入systemsetting.ini文件
//bool WriteSettingIniSet(QString key, QString value);
//将自定义分区设置写入systemsetting.ini文件
//bool WriteSettingIniCustomSet(QString key, QString value);


//不同的硬件架构，支持不同的文件系统
//QString getSettingValue(QString key);

//通用写入配置文件设置函数


void updataIni();

bool WriteSettingToIni(QString group, QString key, QString value);
bool WritePwdToIni(QString group, QString key, QString value);
QString ReadSettingIni(QString group, QString key);
void setModel(QString strModeG);
QString AutoReadSettingIni(QString group, QString key);
QList<LanguageSetInINI> ReadLanguageIni(void);

bool getSettingBool(QString key);

void writePartitionInfo(QString& rootdisk, QString& rootpart, QString& bootpath, QString& mountpoints);
void writeRequiringSwapFile(bool isRequired);
void writeUEFI(bool isEfi);
void writeDiskPartitionSetting(DiskPartitionSetting& set);
}

#endif // KSYSTEMSETTING_UNIT_H
