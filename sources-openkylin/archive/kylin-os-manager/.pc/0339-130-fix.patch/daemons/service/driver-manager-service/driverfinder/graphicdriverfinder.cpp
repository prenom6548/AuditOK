#include "graphicdriverfinder.h"
#include "../hardwarefinder/graphiccardfinder.h"

#include <QProcess>
#include <QDebug>
#include <QString>
#include <QRegularExpression>

graphicDriverFinder::graphicDriverFinder(QObject *parent) : QObject(parent)
{
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &graphicDriverFinder::dowork);

    connect(this, &graphicDriverFinder::finished, m_thread, &QThread::quit);
    connect(this, &graphicDriverFinder::finished, this, &graphicDriverFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

graphicDriverFinder::~graphicDriverFinder()
{

}

void graphicDriverFinder::dowork()
{
    // 获取硬件信息
    graphicCardFinder graphiccardfinder;
    graphiccardfinder.dowork();
    auto list = graphiccardfinder.getList();

    // 获取驱动包信息
    QString commond = "apt-cache show nvidia-driver*" ;
    QProcess proc;
    proc.start(commond);
    proc.waitForFinished();
    QStringList outPut = QString(proc.readAllStandardOutput()).split("\n\n");
    QStringList hasModaDriver;
    for(int i = 0; i < outPut.length(); i++){
        if(QString(outPut[i]).contains("Modaliases", Qt::CaseInsensitive)){
            hasModaDriver.append(outPut[i]);
        }
    }
    convertStrToDrinfo(hasModaDriver);

    // 匹配硬件和驱动包信息
    driverMatch(list, m_drivers);
    m_hardWareInfo = list;
    emit succeed();
}

void graphicDriverFinder::convertStrToDrinfo(QStringList drivers)
{
    for(int i = 0; i < drivers.length(); i++){
        DriverInfo info;
        QStringList lines = QString(drivers[i]).split("\n");
        for(int j = 0; j < lines.length(); j++){
            // 获取驱动名称
            if(QString(lines[j]).contains("Package: ", Qt::CaseInsensitive)){
                info.driverName = QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取驱动包架构
            if(QString(lines[j]).contains("Architecture:", Qt::CaseInsensitive)){
                info.arch =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取驱动包版本
            if(QString(lines[j]).contains("Version:", Qt::CaseInsensitive)){
                info.version =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取驱动包优先级
            if(QString(lines[j]).contains("Priority:", Qt::CaseInsensitive)){
                info.priority =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取驱动包所属组别
            if(QString(lines[j]).contains("Section:", Qt::CaseInsensitive)){
                info.section =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取驱动包源码包
            if(QString(lines[j]).contains("Source:", Qt::CaseInsensitive)){
                info.source =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取驱动包来源
            if(QString(lines[j]).contains("Origin:", Qt::CaseInsensitive)){
                info.origin =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取驱动包维护者信息
            if(QString(lines[j]).contains("Maintainer:", Qt::CaseInsensitive)){
                info.maintainer =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取驱动包bug报告信息
            if(QString(lines[j]).contains("Bugs:", Qt::CaseInsensitive)){
                info.bugs =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取驱动安装后占用空间信息
            if(QString(lines[j]).contains("Installed-Size:", Qt::CaseInsensitive)){
                info.installedSize =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取驱动包运行时依赖
            if(QString(lines[j]).contains("Depends:", Qt::CaseInsensitive)){
                info.depends =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取建议安装的相关软件包
            if(QString(lines[j]).contains("Recommends:", Qt::CaseInsensitive)){
                info.recommends =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取软件包的二进制文件存储在软件仓库中的路径
            if(QString(lines[j]).contains("Filename:", Qt::CaseInsensitive)){
                info.filename =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取软件包二进制文件在压缩状态下的大小
            if(QString(lines[j]).contains("Size:", Qt::CaseInsensitive)){
                info.size =  QString(lines[j]).split(": ")[1];
                continue;
            }

            // 获取软件包modalias
            if(QString(lines[j]).contains("Modaliases:", Qt::CaseInsensitive)){
                QString modaliases =  QString(lines[j]).split(": ")[1];
                if(modaliases.contains("nvidia(", Qt::CaseInsensitive)){
                    modaliases.replace("nvidia(", "");
                }
                if(modaliases.contains(")", Qt::CaseInsensitive)){
                    modaliases.replace(")", "");
                }
                if(modaliases.contains("\"", Qt::CaseInsensitive)){
                    modaliases.replace("\"", "");
                }
                info.modaliases = modaliases.split(", ");
                continue;
            }

        }
        m_drivers.append(info);
    }
}

void graphicDriverFinder::driverMatch(QList<GraphicCardInfo> &cardsInfo, const QList<DriverInfo> &driversInfo)
{
    for(int i = 0; i < cardsInfo.length(); i++){
        qDebug() << "cardVendor: " << cardsInfo[i].vendor;
        qDebug() << "cardModel: " << cardsInfo[i].model;
        for (int j = 0; j < driversInfo.length(); ++j) {
            for(int k = 0; k < driversInfo[j].modaliases.length(); k++){
                if(fnmatch(driversInfo[j].modaliases[k], cardsInfo[i].modalias)){
//                if(fnmatch(QString(driversInfo[j].modaliases[k]).toLower(), QString("pci:v000010DEd00001DBAsv*sd*bc03sc02i00").toLower())){     // 测试代码
                    cardsInfo[i].driverNameList.append(driversInfo[j]);
                    qDebug() << "Driver: " << driversInfo[j].driverName << "  Version: " << driversInfo[j].version;
                }
            }
        }
    }
}

bool fnmatch(const QString &pattern, const QString &string){
    int pidx = 0;
    int sidx = 0;
    int pmatch = -1;
    int smatch = -1;

    while(sidx< string.length()){
        if(pidx < pattern.length() && pattern[pidx] == "*"){
            pmatch = pidx;
            smatch = sidx;
            pidx++;
        }else if(pidx < pattern.length() && (pattern[pidx] == "?" || pattern[pidx] == string[sidx])){
            pidx++;
            sidx++;
        }else if(pmatch != -1){
            pidx = pmatch + 1;
            sidx = ++smatch;
        }else{
            return false;
        }
    }

    while(pidx < pattern.length() && pattern[pidx] == '*'){
        pidx++;
    }
    return pidx == pattern.length() && sidx == string.length();
}

QList<GraphicCardInfo> graphicDriverFinder::getHardWareInfo()
{
    return m_hardWareInfo;
}


