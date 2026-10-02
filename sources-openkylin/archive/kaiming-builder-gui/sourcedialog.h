#ifndef SOURCEDIALOG_H
#define SOURCEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QStackedWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMap>
#include <QWidget>
#include <QString>
#include <QList>
#include <QTextEdit>
#include <QGroupBox>

#include <component/chooselineedit.h>
#include "logic/KMBuilderManifest.h"
#include "messageboxutils.h"

class QComboBox;
class QLineEdit;

class SourceDialog : public QDialog {
    Q_OBJECT
public:
    explicit SourceDialog(const QString& componentName, QWidget *parent = nullptr);
    // 直接使用原结构体类型
    KMBuilderManifest::Component::Source getSourceData();
    void setSourceData(const KMBuilderManifest::Component::Source &source);

private slots:
    void onTypeChanged(int index);

private:
    void createGitForm();
    void createArchiveForm();
    void createDirForm();
    void createFileForm();
    QVector<QString> splitAndTrim(const QString& input);

    QGroupBox *m_onlyArchesGroupBox;
    QGroupBox *m_skipArchesGroupBox;
    QString m_componentName;
    QComboBox *m_typeCombo;
    QStackedWidget *m_stack;
    QMap<QString, QLineEdit*> m_fields; // 存储所有输入字段
//    QTextEdit *m_commandEdit;

    // 用于type=dir/file时，选择本地原始源路径
    ChooseLineEdit * m_originalDirEdit = nullptr;
    ChooseLineEdit * m_originalFileEdit = nullptr;
};

#endif // SOURCEDIALOG_H

