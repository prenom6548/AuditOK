#ifndef PROJECTPATHPAGE_H
#define PROJECTPATHPAGE_H
#include <QWizardPage>

class QLineEdit;
class QPushButton;

class ProjectPathPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ProjectPathPage(QWidget *parent = nullptr);
    bool validatePage() override;

private:
    QLineEdit *m_idEdit;
    QLineEdit *m_projectPathEdit;
    QLineEdit *m_debianSourceEdit;
//    QPushButton *m_browseBtn;
    QPushButton *m_debianBrowseBtn;

    bool isValidAppName(const QString &appName, QString &errMsg);
};
#endif // PROJECTPATHPAGE_H
