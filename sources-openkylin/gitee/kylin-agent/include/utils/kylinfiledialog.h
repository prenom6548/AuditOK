#ifndef KYLINFILEDIALOG_H
#define KYLINFILEDIALOG_H

#include <QFileDialog>
#include <QString>
#include <QStringList>

class QWidget;

class KylinFileDialog
{
public:
    using Option = QFileDialog::Option;
    using Options = QFileDialog::Options;

    static constexpr Option ShowDirsOnly = QFileDialog::ShowDirsOnly;
    static constexpr Option DontResolveSymlinks = QFileDialog::DontResolveSymlinks;

    static QString getOpenFileName(QWidget *parent,
                                   const QString &caption,
                                   const QString &directory = QString(),
                                   const QString &filter = QString());
    static QStringList getOpenFileNames(QWidget *parent,
                                        const QString &caption,
                                        const QString &directory = QString(),
                                        const QString &filter = QString());
    static QString getSaveFileName(QWidget *parent,
                                   const QString &caption,
                                   const QString &directory = QString(),
                                   const QString &filter = QString());
    static QString getExistingDirectory(QWidget *parent,
                                        const QString &caption,
                                        const QString &directory = QString(),
                                        Options options = ShowDirsOnly);
};

#endif // KYLINFILEDIALOG_H
