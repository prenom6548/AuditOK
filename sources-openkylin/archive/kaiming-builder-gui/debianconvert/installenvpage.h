#ifndef INSTALLENVPAGE_H
#define INSTALLENVPAGE_H

#include <QProgressBar>
#include <QTextEdit>
#include <QWizard>
#include <QTimer>
#include <QTime>

#include <QScopedPointer>
class InstallEnvPagePrivate;

class InstallEnvPage : public QWizardPage
{
    Q_OBJECT

public:
    InstallEnvPage(QWidget *parent = nullptr);
    virtual ~InstallEnvPage();
    void initializePage() override;
    void startInstall(QString baseStr, QString runtimeStr);

private:
    Q_DECLARE_PRIVATE(InstallEnvPage)
    QScopedPointer<InstallEnvPagePrivate> d_ptr;
    bool isComplete() const;
    bool validatePage();

};
#endif // INSTALLENVPAGE_H
