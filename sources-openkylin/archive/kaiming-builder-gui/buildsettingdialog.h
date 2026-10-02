#ifndef BUILDSETTINGDIALOG_H
#define BUILDSETTINGDIALOG_H

#include <QDialog>
#include <QScopedPointer>
#include <QCheckBox>
#include <logic/BuildSettingManager.h>

class BuildSettingDialogPrivate;

class BuildSettingDialog : public QDialog
{
    Q_OBJECT
public:
    BuildSettingDialog(QWidget *parent = nullptr, BuildSettingManager *buildSettingManager = nullptr);
    virtual ~BuildSettingDialog();

    QString getCommandString();

private:
    Q_DECLARE_PRIVATE(BuildSettingDialog)
    QScopedPointer<BuildSettingDialogPrivate> d_ptr;

};


#endif // BUILDSETTINGDIALOG_H
