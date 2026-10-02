#include "kyimgosproberprocess.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/kdir.h"
#include <QDebug>

namespace KInstaller {

using namespace KServer;

KyimgOsproberProcess::KyimgOsproberProcess(QObject *parent) : QThread(parent)
{

}

void KyimgOsproberProcess::run()
{
    QString output;
    QString ScriptPath;
    QString error;
    int exitcode;
    while(1){
	output = "";
        ScriptPath  = "";
        error = "";
        exitcode = 0;
    	KServer::KCommand::getInstance()->RunScripCommand("/usr/share/kylin-os-installer/data/kyimg-prober",{}, ScriptPath, output, error, exitcode);
    	//KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
	msleep(100);
	emit signalKyimgOsprober();
    }
}

}
