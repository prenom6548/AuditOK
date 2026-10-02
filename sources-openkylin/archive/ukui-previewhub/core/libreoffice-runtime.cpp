#include "libreoffice-runtime.h"

#include <QProcess>

namespace
{

enum LibreOfficeInstallType
{
    LibreOfficeNotFound,
    LibreOfficeDeb,
    LibreOfficeKaiming
};

const char kKaimingLibreOfficeId[] = "org.libreoffice.libreoffice";

LibreOfficeInstallType detectLibreOffice()
{
    if (QProcess::execute(QLatin1String("which"), QStringList() << QLatin1String("libreoffice")) == 0)
        return LibreOfficeDeb;

    QProcess proc;
    proc.start(QLatin1String("kaiming"), QStringList() << QLatin1String("list"));
    if (!proc.waitForStarted(3000) || !proc.waitForFinished(3000))
        return LibreOfficeNotFound;

    const QString output = QString::fromLocal8Bit(proc.readAllStandardOutput())
            + QString::fromLocal8Bit(proc.readAllStandardError());
    if (output.contains(QLatin1String(kKaimingLibreOfficeId), Qt::CaseInsensitive))
        return LibreOfficeKaiming;

    return LibreOfficeNotFound;
}

QStringList convertArguments(const QString &outDir, const QString &filePath)
{
    return QStringList() << QLatin1String("--headless")
                         << QLatin1String("--invisible")
                         << QLatin1String("--convert-to") << QLatin1String("pdf")
                         << QLatin1String("--outdir") << outDir
                         << filePath;
}

} // namespace

namespace LibreOfficeRuntime
{

bool isAvailable()
{
    return detectLibreOffice() != LibreOfficeNotFound;
}

Command convertToPdfCommand(const QString &outDir, const QString &filePath)
{
    Command command;
    const LibreOfficeInstallType installType = detectLibreOffice();
    const QStringList loArgs = convertArguments(outDir, filePath);

    if (installType == LibreOfficeKaiming) {
        command.program = QLatin1String("kaiming");
        command.arguments = QStringList()
                << QLatin1String("run")
                << QLatin1String("--command=libreoffice")
                << QLatin1String(kKaimingLibreOfficeId);
        command.arguments.append(loArgs);
    } else {
        command.program = QLatin1String("libreoffice");
        command.arguments = QStringList() << QLatin1String("--headless") << QLatin1String("--invisible")
                                          << QLatin1String("--convert-to") << QLatin1String("pdf")
                                          << QLatin1String("--outdir") << outDir
                                          << filePath;
    }

    return command;
}

} // namespace LibreOfficeRuntime
