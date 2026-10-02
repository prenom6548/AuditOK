#ifndef SELECTENVPAGE_H
#define SELECTENVPAGE_H

#include <QWizardPage>
#include <QLineEdit>
#include <QPushButton>

class SelectEnvPage : public QWizardPage
{
    Q_OBJECT
    Q_PROPERTY(bool isBaseInstalled READ isBaseInstalled)
    Q_PROPERTY(bool isRuntimeInstalled READ isRuntimeInstalled)
public:
    explicit SelectEnvPage(QWidget *parent = nullptr);
    bool validatePage();
    int nextId() const override;
    bool isBaseInstalled() const { return m_isBaseInstalled; }
    bool isRuntimeInstalled() const { return m_isRuntimeInstalled; }

signals:
    void propertyChanged(const QString &propertyName);


private:
    QLineEdit *m_baseInput;
    bool m_isRuntimeInstalled = false;
    QPushButton *m_baseSearchButton;
    QLineEdit *m_runtimeInput;
    bool m_isBaseInstalled = false;
    QPushButton *m_runtimeSearchButton;

    void initializePage();
    void openSearchDialog(const QString &kind, const QString &base, const QString &runtime);
};

#endif // SELECTENVPAGE_H
