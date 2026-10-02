#ifndef SPACESAVEDIALOG_H
#define SPACESAVEDIALOG_H

#include <QObject>
#include <QStyle>
#include <kysdk/applications/kdialog.h>
#include "peony-core_global.h"

class PEONYCORESHARED_EXPORT SpaceSaveDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SpaceSaveDialog(const QStringList &paths, const QString &conditions, const QString &keyWord = "", QWidget *parent = nullptr);
    ~SpaceSaveDialog();

private:
    QString m_keyWord;
    QString m_conditions;
    QStringList m_paths;
};

#endif // SPACESAVEDIALOG_H
