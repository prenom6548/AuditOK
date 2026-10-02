#ifndef PACKAGEFRAME_H
#define PACKAGEFRAME_H

#include <globalinfo.h>
#include <QFrame>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QTextEdit>
#include <QMap>
#include <QVariant>
#include <QList>
#include <QPair>

class PackageFrame : public QFrame
{
    Q_OBJECT

public:
    explicit PackageFrame(QWidget *parent = nullptr);
    void getData();
    void clear(const QString &id);
    void load(const KMBuilderManifest::Package &package);

    //id
    QString id() const {
        return m_idEdit->text();
    }

    void setIdFocus() {
        m_idEdit->setFocus();
    }

    //name
    QString name() const {
        return m_nameEdit->text();
    }

    void setNameFocus(){
        return m_nameEdit->setFocus();
    }

    //version
    QString version() const {
        return m_versionEdit->text();
    }

    void setVersionFocus()
    {
        return m_versionEdit->setFocus();
    }


public slots:
//    void toggleMoreLanguages();
//    void addLanguageField();

private:
    QLineEdit *m_versionEdit, *m_idEdit, *m_nameEdit;
    QComboBox *m_kindComboBox;
    QTextEdit *m_descriptionEdit;

//    QList<QPair<QString, QPair<QLineEdit*, QTextEdit*>>> m_languageWidgets;
//    QPushButton *m_moreButton;
//    QPushButton *m_addLangButton;
//    QGroupBox *m_multiLangSection;
//    QComboBox *m_languageComboBox;
//    QLineEdit *m_multiLangNameEdit;
//    QTextEdit *m_multiLangDescEdit;
};

#endif // PACKAGEFRAME_H
