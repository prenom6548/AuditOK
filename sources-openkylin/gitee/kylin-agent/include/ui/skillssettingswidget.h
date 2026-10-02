#ifndef SKILLSETTINGSWIDGET_H
#define SKILLSETTINGSWIDGET_H

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVector>
#include <QVBoxLayout>
#include <QWidget>

class SkillsSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit SkillsSettingsWidget(QWidget *parent = nullptr);

    void loadSettings();
    void saveSettings();

    struct SkillInfo {
        QString id;
        QString name;
        QString category;
        QString description;
        QString path;
        QString skillFile;
        QString content;
        bool installed = false;
        bool imported = false;
    };

protected:
    void resizeEvent(QResizeEvent *event) override;

    QVBoxLayout *m_mainLayout;
    QLineEdit *m_searchLineEdit;
    QCheckBox *m_selfEvolutionCheckBox;
    QLabel *m_countLabel;
    QPushButton *m_refreshButton;
    QWidget *m_cardsWidget;
    QGridLayout *m_cardsLayout;
    QLabel *m_emptyLabel;

    QVector<SkillInfo> m_installedSkills;
    int m_lastColumnCount = 0;
    int m_cardBuildGeneration = 0;

    void createSkillsGroup();
    void refreshSkills();
    void refreshSkillsWithFeedback();
    void updateCards();
    void appendSkillCardBatch(const QVector<SkillInfo> &skills,
                              int columns,
                              int start,
                              int generation);
    void showSkillPreview(const SkillInfo &skill);
    bool deleteSkill(const SkillInfo &skill);
    SkillInfo installedSkillFor(const SkillInfo &skill, bool *found = nullptr) const;

    QVector<SkillInfo> loadSkillsFromRoot(const QString &rootPath, bool installed) const;
    SkillInfo readSkillInfo(const QString &skillDir,
                            const QString &skillFile,
                            const QString &fallbackName,
                            const QString &fallbackCategory,
                            bool installed) const;
    QString runtimeHome() const;
    QString installedSkillsRoot() const;
    QString importSkillsRoot() const;
    int columnCountForWidth(int width) const;
    bool skillSelfEvolutionEnabled() const;
    bool setSkillSelfEvolutionEnabled(bool enabled) const;

    // ── New: create / import / edit skill ────────────────────────────
    void onCreateSkill();
    void onEditSkill(const SkillInfo &skill);
    void onImportSkill();
    bool ensureImportExternalDir();
    bool copyDirectoryRecursive(const QString &src, const QString &dst);
    bool extractZipArchive(const QString &archivePath, const QString &dstDir, QString *errorMessage);
    bool validateSkillDir(const QString &dirPath, QString *outName = nullptr) const;
    void updateConfigExternalDirs();
};

#endif // SKILLSETTINGSWIDGET_H
