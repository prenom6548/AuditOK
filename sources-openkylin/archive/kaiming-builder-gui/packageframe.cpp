#include "packageframe.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QMap>
#include <QVariant>
#include <QScrollArea>

#define INIT_FIELD(labelText, widget, WidgetType) \
    { \
        QLabel *label = new QLabel(labelText); \
        label->setFixedWidth(150); \
        widget = new WidgetType(this); \
        widget->setFixedHeight(35); \
        widget->setPlaceholderText(labelText); \
    }

#define INIT_COMBOBOX(labelText, widget) \
    { \
        QLabel *label = new QLabel(labelText); \
        label->setFixedWidth(150); \
        widget = new QComboBox(this); \
        widget->addItems({"app","base", "runtime", "depend"}); \
    }

PackageFrame::PackageFrame(QWidget *parent)
    : QFrame(parent)
//      m_moreButton(new QPushButton("More", this))
//      m_multiLangSection(new QGroupBox("Multi-language Info", this)),
//      m_languageComboBox(new QComboBox(this)),
//      m_addLangButton(new QPushButton("+ Add Language", this))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Package info section
    QFormLayout *formLayout = new QFormLayout;

    INIT_FIELD("id", m_idEdit, QLineEdit);
    formLayout->addRow(tr("id") + "<font color = red>*</font>", m_idEdit);
    // 设置为不可编辑
    m_idEdit->setReadOnly(true);
    // 设置背景颜色为灰色
    m_idEdit->setStyleSheet("background-color: #f0f0f0;");
    m_idEdit->setPlaceholderText(tr("Unique identifier of the software. It consists of a triplet."
                                 "Example: "
                                 "top.openkylin.demo"));
    m_idEdit->setToolTip(tr("Unique identifier of the software. It consists of a triplet.\n"
                            "Example:\n"
                            "top.openkylin.demo"));

    INIT_FIELD("name", m_nameEdit, QLineEdit);
    formLayout->addRow(tr("name") + "<font color = red>*</font>", m_nameEdit);
    m_nameEdit->setPlaceholderText(tr("Package name, which is usually the last value of the id."
                                   "Example:  "
                                   "demo"));
    m_nameEdit->setToolTip(tr("Package name, which is usually the last value of the id.\n"
                              "Example:\n"
                              "demo"));

    INIT_FIELD("version", m_versionEdit, QLineEdit);
    formLayout->addRow(tr("version") + "<font color = red>*</font>", m_versionEdit);
    m_versionEdit->setPlaceholderText(tr("Software version number."
                                      "Example:  "
                                      "1.0.1"));
    m_versionEdit->setToolTip(tr("Software version number.\n"
                                 "Example:\n"
                                 "1.0.1"));

    INIT_COMBOBOX("kind", m_kindComboBox);
    formLayout->addRow(tr("kind") + "<font color = red>*</font>", m_kindComboBox);
    m_kindComboBox->setToolTip(tr("Software package type, including app, base, runtime, and depend."));

    INIT_FIELD("description", m_descriptionEdit, QTextEdit);
    formLayout->addRow(tr("description"), m_descriptionEdit);
    m_descriptionEdit->setPlaceholderText(tr("A brief description of the package. "
                                             "Example:  "
                                             "Demo for test"));
    m_descriptionEdit->setToolTip(tr("A brief description of the package.\n"
                                     "Example:\n"
                                     "Demo for test"));

    mainLayout->addLayout(formLayout);

//    // More Languages section
//    mainLayout->addWidget(m_moreButton);
//    m_languageComboBox->addItems({"zh_CN", "en_US", "fr_FR", "es_ES"});
//    QFormLayout *multiLangLayout = new QFormLayout(m_multiLangSection);
//    multiLangLayout->addRow("Language", m_languageComboBox);
//    multiLangLayout->addWidget(m_addLangButton);  // Add the + button
//    m_multiLangSection->setLayout(multiLangLayout);
//    mainLayout->addWidget(m_multiLangSection);
//    m_multiLangSection->setVisible(false);

//    connect(m_moreButton, &QPushButton::clicked, this, &PackageFrame::toggleMoreLanguages);
//    connect(m_addLangButton, &QPushButton::clicked, this, &PackageFrame::addLanguageField);

    setLayout(mainLayout);
}

void PackageFrame::getData()
{
    KMBuilderManifest * manifest = GlobelInfo::instance().getManifest();
    manifest->m_package.m_id = m_idEdit->text().trimmed();
    manifest->m_package.m_name  = m_nameEdit->text().trimmed();
    manifest->m_package.m_version = m_versionEdit->text().trimmed();
    manifest->m_package.m_kind = m_kindComboBox->currentText();
    manifest->m_package.m_description = m_descriptionEdit->toPlainText();
}

void PackageFrame::clear(const QString &id)
{
    m_idEdit->setText(id);
    m_nameEdit->clear();
    m_versionEdit->clear();
    m_kindComboBox->setCurrentIndex(0);
    m_descriptionEdit->clear();
}

void PackageFrame::load(const KMBuilderManifest::Package &package)
{
    m_idEdit->setText(package.m_id);
    m_nameEdit->setText(package.m_name);
    m_versionEdit->setText(package.m_version);
    m_kindComboBox->setCurrentText(package.m_kind);
    m_descriptionEdit->setText(package.m_description);
}
//void PackageFrame::toggleMoreLanguages()
//{
//    bool isVisible = m_multiLangSection->isVisible();
//    m_multiLangSection->setVisible(!isVisible);
//    m_moreButton->setText(isVisible ? "More" : "Less");
//}

//void PackageFrame::addLanguageField()
//{
//    // Get the selected language
//    QString selectedLang = m_languageComboBox->currentText();

//    // Create new language widgets (Name and Description)
//    QWidget *langWidget = new QWidget;
//    QFormLayout *langLayout = new QFormLayout(langWidget);

//    QLineEdit *nameEdit = new QLineEdit(this);
//    nameEdit->setPlaceholderText("Name[" + selectedLang + "]");
//    QTextEdit *descEdit = new QTextEdit(this);
//    descEdit->setPlaceholderText("Description[" + selectedLang + "]");

//    langLayout->addRow("Name:", nameEdit);
//    langLayout->addRow("Description:", descEdit);

//    // Add the new language widget to the section
//    m_languageWidgets.append(qMakePair(selectedLang, qMakePair(nameEdit, descEdit)));

//    m_multiLangSection->layout()->addWidget(langWidget);  // Add the new widget to the layout
//}
