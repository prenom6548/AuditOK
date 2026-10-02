#ifndef DEBIANCONVERTDIALOG_H
#define DEBIANCONVERTDIALOG_H

#include <QWizard>
#include <QStackedWidget>
#include <QList>

class DebianConvertDialog : public QWizard
{
    Q_OBJECT
public:
    enum { Page_ProjectPath, Page_SelectEnv, Page_InstallEnv, Page_GenerateProject };

    explicit DebianConvertDialog(QWidget *parent = nullptr);
    void updateSidebarHighlight(int currentPageId);

    QStackedWidget *m_pageContainer = nullptr;
    QList<QWizardPage*> m_pages;
signals:
    void fileSelected(const QString &filePath);
};
#endif // DEBIANCONVERTDIALOG_H
