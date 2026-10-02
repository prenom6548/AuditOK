#ifndef SIMPLEDESKAPPLICATION_H
#define SIMPLEDESKAPPLICATION_H

#include <QApplication>
#include "xdgautostart.h"
#include "xdgdesktopfile.h"
#include "xdgdirs.h"
#include "ukuimodule.h"

typedef QMap<QString, UkuiModule *> ModulesMap;
class simpleDeskApplication : public QApplication
{
    Q_OBJECT
public:
    simpleDeskApplication(int &argc, char **argv);
    ~simpleDeskApplication();

private Q_SLOTS:
    void restartModules(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QStringList desktop_paths;
    ModulesMap mNameMap;
    void startProcess(const XdgDesktopFile &file, bool required);

    bool autoRestart(const XdgDesktopFile &file);
};

#endif // SIMPLEDESKAPPLICATION_H
