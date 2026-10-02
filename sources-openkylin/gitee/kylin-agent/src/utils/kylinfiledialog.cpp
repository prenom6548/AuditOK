#include "utils/kylinfiledialog.h"

#include <kdialog.h>

#include <QDir>
#include <QFileInfo>
#include <QVBoxLayout>

namespace {

QStringList runFileDialog(QWidget *parent,
                          const QString &caption,
                          const QString &directory,
                          const QString &filter,
                          QFileDialog::FileMode fileMode,
                          QFileDialog::AcceptMode acceptMode,
                          QFileDialog::Options options)
{
    kdk::KDialog window(parent);
    window.setWindowTitle(caption);
    window.resize(820, 560);

    auto *layout = new QVBoxLayout(window.mainWidget());
    layout->setContentsMargins(0, 0, 0, 0);

    auto *picker = new QFileDialog(window.mainWidget(), caption, QString(), filter);
    picker->setWindowFlags(Qt::Widget);
    picker->setOption(QFileDialog::DontUseNativeDialog, true);
    picker->setOptions(picker->options() | options | QFileDialog::DontUseNativeDialog);
    picker->setFileMode(fileMode);
    picker->setAcceptMode(acceptMode);

    if (!directory.trimmed().isEmpty()) {
        const QFileInfo initial(directory);
        if (acceptMode == QFileDialog::AcceptSave && !initial.isDir()) {
            picker->setDirectory(initial.absolutePath());
            picker->selectFile(initial.fileName());
        } else {
            picker->setDirectory(directory);
        }
    }

    layout->addWidget(picker);
    QObject::connect(picker, &QFileDialog::accepted, &window, &kdk::KDialog::accept);
    QObject::connect(picker, &QFileDialog::rejected, &window, &kdk::KDialog::reject);

    if (window.exec() != kdk::KDialog::Accepted) {
        return {};
    }
    return picker->selectedFiles();
}

} // namespace

QString KylinFileDialog::getOpenFileName(QWidget *parent,
                                         const QString &caption,
                                         const QString &directory,
                                         const QString &filter)
{
    return runFileDialog(parent, caption, directory, filter,
                         QFileDialog::ExistingFile, QFileDialog::AcceptOpen, {}).value(0);
}

QStringList KylinFileDialog::getOpenFileNames(QWidget *parent,
                                              const QString &caption,
                                              const QString &directory,
                                              const QString &filter)
{
    return runFileDialog(parent, caption, directory, filter,
                         QFileDialog::ExistingFiles, QFileDialog::AcceptOpen, {});
}

QString KylinFileDialog::getSaveFileName(QWidget *parent,
                                         const QString &caption,
                                         const QString &directory,
                                         const QString &filter)
{
    return runFileDialog(parent, caption, directory, filter,
                         QFileDialog::AnyFile, QFileDialog::AcceptSave, {}).value(0);
}

QString KylinFileDialog::getExistingDirectory(QWidget *parent,
                                              const QString &caption,
                                              const QString &directory,
                                              Options options)
{
    return runFileDialog(parent, caption, directory, QString(),
                         QFileDialog::Directory, QFileDialog::AcceptOpen, options).value(0);
}
