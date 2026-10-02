#ifndef GENERATEPROJECTPAGE_H
#define GENERATEPROJECTPAGE_H

#include <QTextEdit>
#include <QWizard>
#include <QTime>

#include "common/subprocess.h"
#include "component/keywordhighlighter.h"
#include "component/outputedit.h"

class GenerateProjectPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit GenerateProjectPage(QWidget *parent = nullptr);
    void initializePage() override;
    void startConvert();

signals:
    void fileSelected(const QString &filePath);

private:
    OutputEdit * m_output;
    KeywordHighlighter * m_highlighter = nullptr;
    SubProcess m_subProcess;
    QString m_projectPath;
    QString m_projectID;
    QString m_debianSourcePath;
    QString m_base;
    QString m_runtime;

    bool isComplete() const;
    bool validatePage();
};
#endif // GENERATEPROJECTPAGE_H
