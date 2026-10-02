#ifndef BUILDOPTIONSDIALOG_H
#define BUILDOPTIONSDIALOG_H

#include "logic/KMBuilderManifest.h"
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QTimer>
#include <QPushButton>
#include <QDebug>

class BuildOptionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BuildOptionsDialog(QWidget *parent = nullptr);
    ~ BuildOptionsDialog() = default;
    KMBuilderManifest::Component::BuildOptions getBuildOptionsData() const;
    void setBuildOptionsData(const KMBuilderManifest::Component::BuildOptions &buildOptions);


private:
    QLineEdit *m_cFlagsEdit;
    QLineEdit *m_cppFlagsEdit;
    QLineEdit *m_cxxFlagsEdit;
    QLineEdit *m_ldFlagsEdit;
    QLineEdit *m_appendPathEdit;
    QLineEdit *m_prependPathEdit;
    QLineEdit *m_appendLdLibraryPathEdit;
    QLineEdit *m_prependLdLibraryPathEdit;
    QLineEdit *m_appendPkgConfigPathEdit;
    QLineEdit *m_prependPkgConfigPathEdit;
    QTextEdit *m_makeArgsEdit;
    QTextEdit *m_makeInstallArgsEdit;
    QWidget *m_envContainer;
    QTableWidget *m_envTableWidget;
    QPushButton *m_addEnvButton;
    void initUI();
    void createEnvRow();

};

#endif // BUILDOPTIONSDIALOG_H
