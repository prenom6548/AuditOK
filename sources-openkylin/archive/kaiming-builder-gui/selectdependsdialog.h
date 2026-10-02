#ifndef SELECTDEPENDSDIALOG_H
#define SELECTDEPENDSDIALOG_H

#include <QDialog>
#include <QScopedPointer>
#include <QVector>

#include "messageboxutils.h"

class SelectDependsDialogPrivate;

struct SelectedDepend
{
    QString id;
    QString version;
};

class SelectDependsDialog : public QDialog
{
    Q_OBJECT
public:
    SelectDependsDialog(QWidget *parent = nullptr, const QString &base = "", const QString &runtime = "", const QStringList selectedDepends=QStringList());
    virtual ~SelectDependsDialog();

    const QVector<SelectedDepend>& getSelected();

private:
    Q_DECLARE_PRIVATE(SelectDependsDialog)
    QScopedPointer<SelectDependsDialogPrivate> d_ptr;

    QStringList m_selected_depends;
};

#endif // SELECTDEPENDSDIALOG_H
